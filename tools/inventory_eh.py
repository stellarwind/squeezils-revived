"""Trace unresolved EH actions through original VC4 FuncInfo/unwind maps.

Evidence only: original stack offsets and local labels cannot be relocated to
recompiled parents. Never emits aliases, replacement bodies or link inputs.
"""
import collections
import bisect
import hashlib
import json
import re
import struct

import integration as build


def main():
    inputs = {
        "classification": build.OUT / "code-reference-classification.json",
        "ownership": build.OUT / "ownership.json",
        "original": build.REPO.parent / "SqueezilsGameFiles/SQZ.EXE",
        "disassembly": build.REPO.parent / "nm/full.asm",
        "ranges": build.REPO.parent / "tools/fns_all.txt",
    }
    classification = json.loads(inputs["classification"].read_text())
    ownership = json.loads(inputs["ownership"].read_text())
    raw = inputs["original"].read_bytes()
    u32 = lambda p: struct.unpack_from("<I", raw, p)[0]
    pe = u32(60)
    opt = pe + 24
    base = u32(opt + 28)
    sections = []
    for i in range(struct.unpack_from("<H", raw, pe + 6)[0]):
        p = opt + struct.unpack_from("<H", raw, pe + 20)[0] + i * 40
        size, va, disk, offset = struct.unpack_from("<IIII", raw, p + 8)
        sections.append((raw[p:p + 8].rstrip(b"\0").decode(), base + va,
                         size, disk, offset))

    def read(address, size):
        for _, start, _, disk, offset in sections:
            if start <= address and address + size <= start + disk:
                p = offset + address - start
                return raw[p:p + size]
        raise ValueError("Address outside original raw sections: %08x" % address)

    # Handler thunks load a FuncInfo pointer then jump to __CxxFrameHandler.
    handlers = collections.defaultdict(list)
    for name, start, _, disk, offset in sections:
        if name != ".text":
            continue
        code = raw[offset:offset + disk]
        for i in range(len(code) - 9):
            if code[i] == 0xb8 and code[i + 5] == 0xe9:
                info = struct.unpack_from("<I", code, i + 1)[0]
                displacement = struct.unpack_from("<i", code, i + 6)[0]
                # This address is the observed original runtime entry, not a
                # linked replacement address or an alias destination.
                if start + i + 10 + displacement == 0x48e5f0:
                    handlers[info].append(start + i)

    actions = collections.defaultdict(list)
    maps = []
    for info, thunks in sorted(handlers.items()):
        fields = struct.unpack("<8I", read(info, 32))
        if fields[0] != 0x19930520:
            raise ValueError("Unexpected VC4 FuncInfo magic at %08x" % info)
        count, unwind = fields[1:3]
        if count > 4096 or (count and not unwind):
            raise ValueError("Invalid unwind count/pointer")
        entries = []
        for state in range(count):
            previous, action = struct.unpack("<iI", read(unwind + state * 8, 8))
            if previous < -1 or previous >= state:
                raise ValueError("Invalid unwind state transition")
            entry = dict(state=state, previous_state=previous,
                         slot="%08x" % (unwind + state * 8 + 4), action="%08x" % action)
            entries.append(entry)
            if action:
                actions[action].append(dict(func_info="%08x" % info,
                                            unwind_map="%08x" % unwind, **entry))
        maps.append(dict(func_info="%08x" % info,
                         handlers=["%08x" % h for h in thunks], entries=entries))

    # Identify parents only from their actual target ranges, never nearest-start
    # guesses. Multiple overlapping scoring owners are retained for review.
    parents = collections.defaultdict(list)
    handler_addresses = {h for hs in handlers.values() for h in hs}
    for line in inputs["disassembly"].open():
        match = re.search(r"^\s*([0-9a-f]+):\s+68 ((?:[0-9a-f]{2} ){3}[0-9a-f]{2})\s+push", line)
        if match:
            address = int(match[1], 16)
            target = int.from_bytes(bytes.fromhex(match[2]), "little")
            if target in handler_addresses:
                parents[target].append(address)
    ranges = {}
    for line in inputs["ranges"].read_text().splitlines():
        match = re.match(r"([0-9a-fA-F]+) size=(\d+)", line)
        if match:
            ranges[int(match[1], 16)] = int(match[2])
    starts = sorted(ranges)
    rows = []
    for row in classification["entries"]:
        if row["category"] != "eh_funclet_inventory":
            continue
        address = int(row["address"], 16)
        evidence = actions.get(address, [])
        rows.append(dict(symbol=row["symbol"], address=row["address"],
                         unwind_entries=evidence,
                         status="validated_original_unwind_action" if evidence else "inventory_only_requires_review"))
    for item in maps:
        item["parent_handler_push_sites"] = ["%08x" % p for h in item["handlers"]
                                             for p in parents[int(h, 16)]]
        item["parent_candidates"] = []
        for site in item["parent_handler_push_sites"]:
            address = int(site, 16)
            index = bisect.bisect_right(starts, address) - 1
            if index >= 0 and address < starts[index] + ranges[starts[index]]:
                key = "%08x" % starts[index]
                item["parent_candidates"].append(dict(
                    address=key, handler_push_site=site,
                    owners=ownership["addresses"].get(key, [])))
    report = dict(warning="Original EH evidence only. No compiled-frame compatibility or metadata liveness proof.",
                  input_sha256={k: hashlib.sha256(p.read_bytes()).hexdigest() for k, p in inputs.items()},
                  counts=dict(collections.Counter(row["status"] for row in rows)),
                  func_info_count=len(maps), entries=rows, func_info_maps=maps)
    (build.OUT / "eh-inventory.json").write_text(json.dumps(report, indent=2))
    print(json.dumps(dict(counts=report["counts"], func_info_count=len(maps)), indent=2))


if __name__ == "__main__":
    main()
