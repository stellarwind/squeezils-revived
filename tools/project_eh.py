"""Remove validated, unreachable original EH records from extracted data.

Compacts actual section bytes, symbols and relocation positions. Never replaces
cleanup code or exports aliases for frame-dependent compiler labels.
"""
import hashlib
import json
import struct

import integration as build


def parse(raw):
    machine, ns, _, sp, count, opt, _ = struct.unpack_from("<HHIIIHH", raw)
    if machine != 0x14c or opt:
        raise ValueError("Expected ordinary i386 COFF")
    st = sp + count * 18
    symbols = {}
    i = 0
    while i < count:
        p = sp + i * 18
        word = raw[p:p + 8]
        if word[:4] == bytes(4):
            start = st + struct.unpack_from("<I", word, 4)[0]
            name = raw[start:raw.index(0, start)].decode("ascii")
        else:
            name = word.rstrip(b"\0").decode("ascii")
        value, section, typ, storage, aux = struct.unpack_from("<IhHBB", raw, p + 8)
        symbols[i] = dict(position=p, name=name, value=value, section=section,
                          storage=storage, aux=aux)
        i += 1 + aux
    sections = {}
    for si in range(1, ns + 1):
        p = 20 + (si - 1) * 40
        size, offset, reloc, lines = struct.unpack_from("<IIII", raw, p + 16)
        nr, nl = struct.unpack_from("<HH", raw, p + 32)
        sections[si] = dict(position=p, size=size, offset=offset, reloc=reloc,
                            lines=lines, line_count=nl,
                            relocations=[struct.unpack_from("<IIH", raw, reloc + j * 10) for j in range(nr)])
    return symbols, sections


def project(response):
    data = build.REPO / "data/data.obj"
    inventory_path = build.OUT / "eh-inventory.json"
    import inventory_eh
    inventory_eh.main()
    inventory = json.loads(inventory_path.read_text())
    raw = data.read_bytes()
    symbols, sections = parse(raw)
    from eh_batch import discover
    removals, batch = discover(response, raw, symbols, sections, inventory)
    input_hashes = dict(batch["inspected_inputs_sha256"])
    for path in (data, inventory_path, build.REPO / "tools/eh_batch.py", build.REPO / "tools/project_eh.py"):
        input_hashes[str(path)] = hashlib.sha256(path.read_bytes()).hexdigest()
    output = bytearray(raw)
    removed_relocs = []
    # Validate that retained pointer addends remain relative to the same symbol
    # after compaction; crossing a deleted interval is never silently rebased.
    for si, sec in sections.items():
        for offset, index, typ in sec["relocations"]:
            source_removed = any(section == si and lo <= offset < hi
                                 for section, lo, hi in removals.values())
            if symbols[index]["name"] in removals and not source_removed:
                raise ValueError("Retained data references removed EH object")
            target = symbols[index]
            spans = [(lo, hi) for section, lo, hi in removals.values() if section == target["section"]]
            if spans and not source_removed:
                if typ != 6:
                    raise ValueError("Unsupported relocation to compacted section")
                addend = struct.unpack_from("<I", raw, sec["offset"] + offset)[0]
                start, end = target["value"], target["value"] + addend
                if any(start < hi and end >= lo for lo, hi in spans):
                    raise ValueError("Retained pointer addend crosses discarded EH bytes")
    for si, sec in sections.items():
        spans = sorted((lo, hi) for section, lo, hi in removals.values() if section == si)
        if not spans:
            continue
        if sec["line_count"]:
            raise ValueError("Line records require explicit projection support")
        def dropped(value):
            return any(lo <= value < hi for lo, hi in spans)
        def shifted(value):
            return value - sum(hi - lo for lo, hi in spans if hi <= value)
        for s in symbols.values():
            if s["section"] != si:
                continue
            p = s["position"]
            if s["storage"] == 3 and s["aux"] and s["name"].startswith("."):
                if output[p + 18 + 14]:
                    raise ValueError("Unexpected COMDAT data section")
                continue
            if dropped(s["value"]):
                if s["name"] not in removals or s["aux"]:
                    raise ValueError("Unreviewed symbol inside discarded bytes")
                struct.pack_into("<Ih", output, p + 8, 0, 0)
            else:
                struct.pack_into("<I", output, p + 8, shifted(s["value"]))
        kept = []
        for offset, index, typ in sec["relocations"]:
            target = symbols[index]
            if dropped(offset):
                if not dropped(offset + 3):
                    raise ValueError("Relocation crosses discarded boundary")
                removed_relocs.append(dict(offset=offset, target=target["name"]))
                continue
            if target["name"] in removals:
                raise ValueError("Retained data references removed EH object")
            kept.append((shifted(offset), index, typ))
        # A section-symbol relocation could encode an interior addend needing
        # rebasing. Reject it across all sections rather than guessing.
        for other in sections.values():
            for _, index, _ in other["relocations"]:
                target = symbols[index]
                if target["section"] == si and target["storage"] != 2:
                    raise ValueError("Section/local-symbol relocation requires review")
        payload = raw[sec["offset"]:sec["offset"] + sec["size"]]
        for lo, hi in reversed(spans):
            payload = payload[:lo] + payload[hi:]
        output[sec["offset"]:sec["offset"] + len(payload)] = payload
        struct.pack_into("<I", output, sec["position"] + 16, len(payload))
        struct.pack_into("<H", output, sec["position"] + 32, len(kept))
        for j, relocation in enumerate(kept):
            struct.pack_into("<IIH", output, sec["reloc"] + j * 10, *relocation)
        for s in symbols.values():
            if s["section"] == si and s["storage"] == 3 and s["aux"] and s["name"].startswith("."):
                struct.pack_into("<IH", output, s["position"] + 18, len(payload), len(kept))
                struct.pack_into("<I", output, s["position"] + 18 + 8, 0)
    # VC4 reports undefined externals even with no remaining relocation. Remove
    # dead symbol-table rows as well, remapping every retained relocation index.
    updated_symbols, updated_sections = parse(output)
    used = {index for sec in updated_sections.values() for _, index, _ in sec["relocations"]}
    orphan_targets = {r["target"] for r in removed_relocs}
    drop_indices = {i for i, s in updated_symbols.items()
                    if s["name"] in removals or (s["name"] in orphan_targets
                       and s["section"] == 0 and s["storage"] == 2 and i not in used)}
    if drop_indices & used:
        raise ValueError("Dead symbol still has a retained relocation")
    _, _, _, sp, count, _, _ = struct.unpack_from("<HHIIIHH", output)
    rows = bytearray()
    mapping = {}
    new_count = 0
    for i, s in updated_symbols.items():
        if i in drop_indices:
            if s["aux"]:
                raise ValueError("Cannot drop auxiliary-bearing symbol")
            continue
        if s["aux"] and not (s["storage"] == 103 or
                              (s["storage"] == 3 and s["section"] > 0 and s["name"].startswith("."))):
            raise ValueError("Auxiliary symbol indices require explicit projection support")
        mapping[i] = new_count
        rows.extend(output[s["position"]:s["position"] + 18 * (1 + s["aux"])])
        new_count += 1 + s["aux"]
    for sec in updated_sections.values():
        for j, (offset, index, typ) in enumerate(sec["relocations"]):
            struct.pack_into("<IIH", output, sec["reloc"] + j * 10, offset, mapping[index], typ)
    output = output[:sp] + rows + output[sp + count * 18:]
    struct.pack_into("<I", output, 12, new_count)
    destination = build.OUT / "containers/data_eh.obj"
    destination.write_bytes(output)
    report = dict(batch=batch, removed_objects=removals,
                  removed_relocations=removed_relocs, input_sha256=input_hashes,
                  removed_symbols=[updated_symbols[i]["name"] for i in sorted(drop_indices)],
                  output_sha256=hashlib.sha256(output).hexdigest(),
                  cached_inputs_verified=False, runtime_validated=False)
    (build.OUT / "containers/eh-projection.json").write_text(json.dumps(report, indent=2))
    original = '"%s"' % data
    if response.count(original) != 1:
        raise ValueError("Expected one extracted data link input")
    return response.replace(original, '"%s"' % destination)
