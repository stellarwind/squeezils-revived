"""Bind extracted entries and explicitly reviewed ordinary ABI aliases.

Run integration.py --inventory-ownership first. Interior/EH labels, ambiguous
targets, multi-address symbols and AutoGPT funclet adapters remain unresolved.
The ordinary registry records reviewed full decorated names and owner evidence.
No callable ABI wrappers are supplied.
"""
import hashlib
import json
import pathlib
import re

import integration as build
from data_aliases import write_coff


def main():
    report = json.loads((build.OUT / "ownership.json").read_text())
    units = json.loads((build.OUT / "units.json").read_text())
    definitions = {}
    references = set()
    for unit in units:
        if unit["returncode"]:
            continue
        obj = pathlib.Path(unit["object"])
        definitions[unit["source"]] = {
            n for n, k in build.coff_symbols(obj)
            if k in ("strong", "comdat")}
        references.update(n for n, k in build.coff_symbols(obj) if k == "undefined")
    pairs, evidence, skipped = [], [], {}
    data = build.REPO / "data" / "data.obj"
    for name, kind in build.coff_symbols(data):
        match = re.fullmatch(r"_fn_([0-9A-F]{6})", name)
        if kind != "undefined" or not match:
            continue
        address = "00" + match[1].lower()
        entries = report["addresses"].get(address, [])
        entries = [e for e in entries if not e["source"].startswith("AutoGPT/")]
        symbols = {e["symbol"] for e in entries}
        if len(symbols) != 1:
            skipped[name] = "no unique ordinary function entry"
            continue
        symbol = next(iter(symbols))
        if report["symbol_addresses"].get(symbol) != [address]:
            skipped[name] = "implementation targets multiple addresses"
            continue
        if not all(symbol in definitions.get(e["source"], set()) for e in entries):
            raise ValueError("Ownership inventory disagrees with compiled inputs: " + name)
        pairs.append((name, symbol))
        evidence.append(dict(alias=name, target=symbol, address=address, owners=entries))
    registry_path = build.REPO / "tools/ordinary_alias_registry.json"
    registry = json.loads(registry_path.read_text())
    for alias, decision in sorted(registry.items()):
        target, address, owner = (decision[k] for k in ("target", "address", "owner"))
        if (report["symbol_addresses"].get(target) != [address]
                or target not in definitions.get(owner, set())
                or not any(e["symbol"] == target and e["source"] == owner
                           for e in report["addresses"].get(address, []))):
            raise ValueError("Reviewed ordinary owner disagrees with current objects: " + alias)
        if any(alias in names for names in definitions.values()):
            raise ValueError("Reviewed ordinary alias now has a source definition: " + alias)
        for source in decision["evidence"]:
            if not (build.SRC / source).is_file():
                raise ValueError("Missing reviewed declaration evidence: " + source)
        if alias in references:
            pairs.append((alias, target))
            evidence.append(dict(alias=alias, reviewed_decision=decision))
    pairs.sort()
    exact_registry_path = build.REPO / 'tools/exact_call_registry.json'
    if exact_registry_path.exists():
        from exact_call_aliases import discover
        current = discover()
        (build.OUT / 'exact-call-candidates.json').write_text(json.dumps(current, indent=2))
        for alias, decision in json.loads(exact_registry_path.read_text()).items():
            if current['accepted'].get(alias) != decision:
                raise ValueError('Exact-call proof changed; review registry: ' + alias)
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate exact-call alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, exact_call_decision=decision))
        pairs.sort()
    write_coff(pairs, build.OUT / "entry_alias.obj")
    (build.OUT / "entry_alias.json").write_text(json.dumps(dict(
        aliases=pairs, evidence=evidence, skipped=skipped,
        ordinary_registry_sha256=hashlib.sha256(registry_path.read_bytes()).hexdigest(),
        exact_registry_sha256=hashlib.sha256(exact_registry_path.read_bytes()).hexdigest() if exact_registry_path.exists() else None,
        data_sha256=hashlib.sha256(data.read_bytes()).hexdigest()), indent=2))
    print("%d entry/reviewed ordinary aliases; %d skipped data-pointer targets" % (len(pairs), len(skipped)))


if __name__ == "__main__":
    main()
