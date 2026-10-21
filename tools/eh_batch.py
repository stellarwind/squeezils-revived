"""Find whole extracted EH objects unreachable from retained link inputs.

Candidates must consist entirely of validated original FuncInfo/unwind bytes.
All noncandidate data is a root. References from any other input (including
libraries and alias objects) are roots, even if that input is itself unused.
Graph reachability retains dependencies of roots. No per-funclet aliases.
"""
import bisect
import collections
import hashlib
import json
import os
import pathlib
import re
import struct

import integration as build


def reachable(roots, edges):
    live = set(roots)
    pending = collections.deque(roots)
    while pending:
        for target in edges.get(pending.popleft(), ()):
            if target not in live:
                live.add(target)
                pending.append(target)
    return live


def discover(response, raw, symbols, sections, inventory):
    pe_raw = (build.REPO.parent / "SqueezilsGameFiles/SQZ.EXE").read_bytes()
    extent_path = build.REPO / "data/ambiguous.tsv"
    original_extents = {int(parts[0], 16): int(parts[1]) for line in extent_path.read_text().splitlines()[1:]
                        if len(parts := line.split("\t")) >= 2}
    pe = struct.unpack_from("<I", pe_raw, 60)[0]
    opt = pe + 24
    base = struct.unpack_from("<I", pe_raw, opt + 28)[0]
    original_sections = [struct.unpack_from("<IIII", pe_raw, opt + struct.unpack_from("<H", pe_raw, pe + 20)[0] + i * 40 + 8)
                         for i in range(struct.unpack_from("<H", pe_raw, pe + 6)[0])]

    def read(address, size):
        for _, va, disk, offset in original_sections:
            if base + va <= address and address + size <= base + va + disk:
                p = offset + address - base - va
                return pe_raw[p:p + size]
        raise ValueError("EH record outside original image")

    spans = []
    runtime_tables = []
    runtime_report_path = build.OUT / "runtime_alias.json"
    runtime_report = json.loads(runtime_report_path.read_text())
    replacements = runtime_report.get("replaced_startup_records", [])
    if replacements:
        if (runtime_report.get("original_sha256") != hashlib.sha256(pe_raw).hexdigest()
                or runtime_report["sha256"] != hashlib.sha256(pathlib.Path(runtime_report["library"]).read_bytes()).hexdigest()):
            raise ValueError("Stale CRT startup replacement evidence")
        for record in replacements:
            address = int(record["address"], 16)
            if record["size"] != 4 or struct.unpack("<I", read(address, 4))[0] != int(record["initializer"], 16):
                raise ValueError("Original CRT startup slot disagrees")
            spans.append((address, address + 4))
            runtime_tables.append(dict(address=record["address"], count=1, size=4,
                                       kind="replaced_crt_initializer", evidence=record))
    # Original CRT mask-and-dispatch tables belong to LIBC's replacement code.
    # Recognize the actual x86 instruction sequence, not a guessed address band.
    for _, va, disk, offset in original_sections:
        if base + va != 0x401000:
            continue
        code = pe_raw[offset:offset + disk]
        for match in re.finditer(rb"\x83\xe0(.)\xff\x24\x85(.{4})", code, re.DOTALL):
            site = base + va + match.start()
            mask = match[1][0]
            count = mask + 1
            if site < 0x48e000 or mask >= 128 or count < 2 or count & (count - 1):
                continue
            address = struct.unpack("<I", match[2])[0]
            targets = struct.unpack("<%dI" % count, read(address, count * 4))
            if not all(0x48e000 <= target < 0x49d000 for target in targets):
                continue
            spans.append((address, address + count * 4))
            runtime_tables.append(dict(address="%08x" % address, count=count,
                                       dispatch="%08x" % site, size=count * 4,
                                       kind="masked_jump_table"))
        # CRT transcendental wrappers load a length-prefixed math descriptor in
        # edx before jumping to the runtime dispatcher. Unary descriptors have
        # four cases; binary descriptors have sixteen (two operand classes).
        disassembly = (build.REPO.parent / "nm/full.asm").read_text()
        seen = set()
        for match in re.finditer(rb"\xba(.{4})(?:\xe9|\xeb)", code, re.DOTALL):
            site = base + va + match.start()
            if site < 0x48e000:
                continue
            if not re.search(r"(?m)^\s*%x:\s+ba .*\bmov\s+edx," % site, disassembly):
                continue
            address = struct.unpack("<I", match[1])[0]
            try:
                header = read(address, 16)
            except ValueError:
                continue
            name = header[1:1 + header[0]]
            arity = header[13]
            if name not in (b"asin", b"acos", b"atan", b"atan2") or arity not in (1, 2):
                continue
            if (name == b"atan2") != (arity == 2) or address in seen:
                continue
            count = 4 ** arity
            size = 16 + count * 4
            targets = struct.unpack("<%dI" % count, read(address + 16, count * 4))
            if not all(0x48e000 <= target < 0x49d000 for target in targets):
                raise ValueError("Math descriptor has a non-CRT destination")
            # Do not classify a descriptor as CRT-owned if the original game
            # references any field directly. Data references remain graph roots.
            game_reference = False
            for line in disassembly.splitlines():
                instruction = re.match(r"\s*([0-9a-f]+):", line)
                if instruction and int(instruction[1], 16) < 0x48e000:
                    if any(address <= int(value, 16) < address + size
                           for value in re.findall(r"0x([0-9a-f]+)", line)):
                        game_reference = True
                        break
            if game_reference:
                continue
            seen.add(address)
            spans.append((address, address + size))
            runtime_tables.append(dict(address="%08x" % address, count=count,
                                       dispatch="%08x" % site, size=size,
                                       kind="math_descriptor", name=name.decode(), arity=arity))
    for info in inventory["func_info_maps"]:
        address = int(info["func_info"], 16)
        fields = struct.unpack("<8I", read(address, 32))
        if fields[0] != 0x19930520 or fields[7] != 0 or fields[2] != address + 32:
            raise ValueError("Unsupported original FuncInfo layout")
        count = fields[1]
        if count != len(info["entries"]):
            raise ValueError("Unwind entry count disagrees")
        spans.append((address, address + 32 + count * 8))
    merged = []
    for lo, hi in sorted(spans):
        if merged and lo <= merged[-1][1]:
            merged[-1] = (merged[-1][0], max(hi, merged[-1][1]))
        else:
            merged.append((lo, hi))
    starts = [lo for lo, _ in merged]
    definitions = collections.defaultdict(list)
    for index, symbol in symbols.items():
        if symbol["section"] > 0 and symbol["storage"] == 2:
            definitions[symbol["section"]].append((symbol["value"], index))
    objects = {}
    owners = {}
    candidates = {}
    for si, values in definitions.items():
        ordered = sorted(values)
        owners[si] = ordered
        for j, (lo, index) in enumerate(ordered):
            hi = ordered[j + 1][0] if j + 1 < len(ordered) else sections[si]["size"]
            symbol = symbols[index]
            name = symbol["name"]
            objects[name] = (si, lo, hi)
            match = re.match(r"\?g_([0-9A-F]{6,8})@@", name)
            if not match or hi <= lo or not sections[si]["offset"]:
                continue
            address = int(match[1], 16)
            k = bisect.bisect_right(starts, address) - 1
            table = next((t for t in runtime_tables if int(t["address"], 16) <= address
                          < int(t["address"], 16) + t["size"]), None)
            payload_size = hi - lo
            if table:
                payload_size = min(payload_size, merged[k][1] - address,
                                   original_extents.get(address, payload_size))
                # COFF may add tail padding to align the next extracted object.
                # It is not an additional byte range from the original image.
                if hi - lo - payload_size >= 8:
                    continue
            if k < 0 or address + payload_size > merged[k][1]:
                continue
            expected = bytearray(read(address, payload_size) + bytes(hi - lo - payload_size))
            actual = raw[sections[si]["offset"] + lo:sections[si]["offset"] + hi]
            for offset, target, typ in sections[si]["relocations"]:
                if lo <= offset < hi:
                    if offset + 4 > hi or typ != 6:
                        raise ValueError("Unsupported EH relocation")
                    destination = symbols[target]
                    target_match = re.match(r"(?:\?g_|_fn_)([0-9A-F]{6,8})(?:@@|$)", destination["name"])
                    if not target_match:
                        raise ValueError("Unexpected target in original EH record")
                    addend = struct.unpack_from("<I", actual, offset - lo)[0]
                    if struct.unpack_from("<I", expected, offset - lo)[0] != int(target_match[1], 16) + addend:
                        raise ValueError("Original EH pointer identity disagrees with COFF")
                    expected[offset - lo:offset - lo + 4] = actual[offset - lo:offset - lo + 4]
            if bytes(expected) != actual:
                raise ValueError("Original EH bytes disagree with extracted data: " + name)
            candidates[name] = (si, lo, hi)

    external = set()
    hashes = {}
    hashes[str(extent_path)] = hashlib.sha256(extent_path.read_bytes()).hexdigest()
    hashes[str(runtime_report_path)] = hashlib.sha256(runtime_report_path.read_bytes()).hexdigest()
    data = build.REPO / "data/data.obj"
    for line in response.splitlines():
        item = line.strip().strip('"')
        if not item or item.startswith(("-", "/")):
            continue
        path = pathlib.Path(item)
        if not path.is_file():
            path = next((pathlib.Path(folder) / item for folder in os.environ.get("LIB", "").split(";")
                         if (pathlib.Path(folder) / item).is_file()), None)
        if path is None:
            raise ValueError("Cannot inspect link input: " + item)
        if path.resolve() == data.resolve():
            continue
        content = path.read_bytes()
        hashes[str(path)] = hashlib.sha256(content).hexdigest()
        external.update(m.decode("ascii") for m in re.findall(rb"\?g_[0-9A-F]{6,8}@@[^\x00]*", content))
    edges = collections.defaultdict(set)
    roots = set(objects) - set(candidates)
    roots.update(external.intersection(candidates))
    for si, section in sections.items():
        ordered = owners.get(si, [])
        offsets = [x[0] for x in ordered]
        for offset, index, typ in section["relocations"]:
            target = symbols[index]["name"]
            j = bisect.bisect_right(offsets, offset) - 1
            if j < 0:
                roots.add(target)
            else:
                source = symbols[ordered[j][1]]["name"]
                edges[source].add(target)
    # Extraction may split a jump table into one symbol per slot. A reference
    # to any slot keeps the whole table: indexed loads need all of its entries.
    for table in runtime_tables:
        lo = int(table["address"], 16)
        hi = lo + table["size"]
        members = [name for name, (_, first, last) in objects.items()
                   if (m := re.match(r"\?[A-Za-z]_([0-9A-F]+)@@", name))
                   and int(m[1], 16) < hi and int(m[1], 16) + last - first > lo]
        if members:
            anchor = members[0]
            edges[anchor].update(members)
            for member in members:
                edges[member].add(anchor)
    live = reachable(roots, edges)
    removals = {name: span for name, span in sorted(candidates.items()) if name not in live}
    report = dict(candidate_objects=len(candidates), external_roots=sorted(external.intersection(candidates)),
                  runtime_jump_tables=[t for t in runtime_tables if t["kind"] == "masked_jump_table"],
                  runtime_math_descriptors=[t for t in runtime_tables if t["kind"] == "math_descriptor"],
                  replaced_crt_initializers=[t for t in runtime_tables if t["kind"] == "replaced_crt_initializer"],
                  retained_candidates=sorted(set(candidates).intersection(live)),
                  removed_objects=len(removals), removed_bytes=sum(hi-lo for _,lo,hi in removals.values()),
                  removed_names=sorted(removals), inspected_inputs_sha256=hashes,
                  rule="Validated original EH records and atomic CRT dispatch tables, unreachable from all noncandidate data and other link inputs")
    return removals, report
