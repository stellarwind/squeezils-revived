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
            if k < 0 or address + hi - lo > merged[k][1]:
                continue
            expected = bytearray(read(address, hi - lo))
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
                raise ValueError("Original EH bytes disagree with extracted data")
            candidates[name] = (si, lo, hi)

    external = set()
    hashes = {}
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
    live = reachable(roots, edges)
    removals = {name: span for name, span in sorted(candidates.items()) if name not in live}
    report = dict(candidate_objects=len(candidates), external_roots=sorted(external.intersection(candidates)),
                  retained_candidates=sorted(set(candidates).intersection(live)),
                  removed_objects=len(removals), removed_bytes=sum(hi-lo for _,lo,hi in removals.values()),
                  removed_names=sorted(removals), inspected_inputs_sha256=hashes,
                  rule="Whole validated original EH records, unreachable from all noncandidate data and all other link inputs")
    return removals, report
