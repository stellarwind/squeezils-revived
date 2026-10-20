"""Bind extracted code pointers only to uniquely inventoried function entries.

Run integration.py --inventory-ownership first. Interior/EH labels, ambiguous
targets, multi-address symbols and AutoGPT funclet adapters remain unresolved.
This aliases pointer destinations; it supplies no callable ABI wrapper.
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
    for unit in units:
        if unit["returncode"]:
            continue
        obj = pathlib.Path(unit["object"])
        definitions[unit["source"]] = {
            n for n, k in build.coff_symbols(obj)
            if k in ("strong", "comdat")}
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
    pairs.sort()
    write_coff(pairs, build.OUT / "entry_alias.obj")
    (build.OUT / "entry_alias.json").write_text(json.dumps(dict(
        aliases=pairs, evidence=evidence, skipped=skipped,
        data_sha256=hashlib.sha256(data.read_bytes()).hexdigest()), indent=2))
    print("%d entry aliases; %d unresolved interior/ambiguous targets" % (len(pairs), len(skipped)))


if __name__ == "__main__":
    main()
