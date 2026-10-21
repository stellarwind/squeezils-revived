"""Group current linker diagnostics by implementation family and referring source.

This is a work inventory, not an ownership or ABI decision generator.
"""
import collections
import hashlib
import json
import re

import integration as build


def main():
    paths = [build.OUT / "containers/unresolved-grouped.json",
             build.OUT / "external-references.json", build.OUT / "units.json"]
    unresolved, external, units = [json.loads(p.read_text()) for p in paths]
    sources = {u["object"].replace("\\", "/").split("/Repo/")[-1]: u["source"] for u in units}
    groups = collections.defaultdict(list)
    for diagnostic, count in unresolved.items():
        symbol = diagnostic.rsplit("(", 1)[-1].rstrip(")") if diagnostic.endswith(")") else diagnostic
        if "DebugLog" in symbol:
            family = "DebugLog"
        elif "Qv" in symbol:
            family = "Qv"
        elif any(token in symbol for token in ("?$Set@", "?$Array@", "?$Table@", "?$PrimitiveArray@", "?$BufferObjectTable@")):
            family = "Set-String-container-templates"
        elif re.fullmatch(r"_fn_[0-9A-Fa-f]+", symbol):
            family = "extracted-code-pointers"
        else:
            family = "ordinary-other"
        referring = sorted({sources.get(obj.replace("\\", "/"), obj) for obj, kind in external.get(symbol, [])})
        groups[family].append(dict(symbol=symbol, diagnostic=diagnostic, messages=count, referring_sources=referring))
    report = dict(warning="Families and referring sources are triage hints, not verified implementation owners.",
                  input_sha256={str(p.relative_to(build.OUT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
                  clusters={family: dict(unique_symbols=len(rows), messages=sum(r["messages"] for r in rows),
                                         symbols=sorted(rows, key=lambda r: (-r["messages"], r["symbol"])))
                            for family, rows in sorted(groups.items())})
    (build.OUT / "owner-clusters.json").write_text(json.dumps(report, indent=2))
    for family, cluster in report["clusters"].items():
        print("%s: %d symbols, %d messages" % (family, cluster["unique_symbols"], cluster["messages"]))


if __name__ == "__main__":
    main()
