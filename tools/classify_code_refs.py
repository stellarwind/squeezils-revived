"""Classify current unresolved code pointers; never generate aliases or offsets.

Ranges are original disassembly ranges, not compiled replacements. Interior
membership is diagnostic evidence only and never a valid relocation recipe.
"""
import bisect
import collections
import hashlib
import json
import re

import integration as build


def main():
    root = build.REPO.parent
    inputs = dict(unresolved=build.OUT / "containers/unresolved-grouped.json",
                  ownership=build.OUT / "ownership.json",
                  original_ranges=root / "tools/fns_all.txt",
                  eh_inventory=root / "META/STATUS_NOTSTARTED_STUBS.md",
                  extracted_targets=build.REPO / "data/function_targets.tsv")
    unresolved = json.loads(inputs["unresolved"].read_text())
    ownership = json.loads(inputs["ownership"].read_text())
    ranges = {}
    for line in inputs["original_ranges"].read_text().splitlines():
        match = re.match(r"([0-9a-fA-F]+) size=(\d+)", line)
        if match:
            ranges[int(match[1], 16)] = int(match[2])
    starts = sorted(ranges)
    eh = {int(m[1], 16) for m in re.finditer(r"(?m)^([0-9a-fA-F]{6})\s+\d+ B", inputs["eh_inventory"].read_text())}
    extracted = {int(line.split()[0], 16): line.split()[1]
                 for line in inputs["extracted_targets"].read_text().splitlines()[1:] if line.strip()}
    rows, categories, affected = [], collections.Counter(), collections.Counter()
    families = collections.defaultdict(lambda: dict(symbols=0, references=0, addresses=[]))
    for name, count in sorted(unresolved.items()):
        match = re.fullmatch(r"_fn_([0-9a-fA-F]{6,8})", name)
        if not match:
            continue
        address = int(match[1], 16)
        key = "%08x" % address
        entries = ownership["addresses"].get(key, [])
        ordinary = [e for e in entries if not e["source"].startswith("AutoGPT/")]
        symbols = sorted({e["symbol"] for e in ordinary})
        row = dict(symbol=name, address=key, references=count,
                   extracted_name=extracted.get(address), owners=entries)
        if address in eh:
            category = "eh_funclet_inventory"
        elif len(symbols) > 1:
            category = "ambiguous_entry_symbols"
        elif symbols and ownership["symbol_addresses"].get(symbols[0]) != [key]:
            category = "entry_implementation_shared_across_addresses"
        elif symbols:
            category = "unique_entry_requires_surviving_owner_review"
        elif entries:
            category = "adapter_only_entry"
        elif address in ranges:
            category = "original_entry_without_target_mapping"
        else:
            index = bisect.bisect_right(starts, address) - 1
            parent = starts[index] if index >= 0 else None
            if parent is not None and address < parent + ranges[parent]:
                category = "interior_original_range"
                row["original_parent"] = "%08x" % parent
                row["original_offset_not_relocatable"] = address - parent
            else:
                category = "absent_original_range_mapping"
        row["category"] = category
        categories[category] += 1
        affected[category] += count
        for source in sorted({e["source"] for e in entries}):
            family = families[source]
            family["symbols"] += 1
            family["references"] += count
            family["addresses"].append(key)
        rows.append(row)
    report = dict(warning="Classification only; cached inventories and heuristic EH list are not ABI proof. No aliases emitted.",
                  reference_count_unit="LNK2001 diagnostic messages, not relocation sites",
                  input_sha256={key: hashlib.sha256(path.read_bytes()).hexdigest() for key, path in inputs.items()},
                  unique_code_pointers=len(rows), categories=dict(categories), affected_references=dict(affected),
                  ranked_owner_families=sorted((dict(source=s, **v) for s, v in families.items()),
                                               key=lambda v: (-v["references"], v["source"])), entries=rows)
    (build.OUT / "code-reference-classification.json").write_text(json.dumps(report, indent=2))
    print(json.dumps({k: v for k, v in report.items() if k not in ("entries", "ranked_owner_families", "input_sha256")}, indent=2))
    print(json.dumps(report["ranked_owner_families"][:8], indent=2))


if __name__ == "__main__":
    main()
