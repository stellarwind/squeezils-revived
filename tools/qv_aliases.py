"""Resolve address-named Qv constructor/destructor declarations to QvLib.

Only direct thiscall constructors and complete destructors with no arguments
qualify. Never map deleting destructors, arbitrary methods or interior labels.
The address encoded by the reconstruction must have a unique zero-score
upstream entry. QvF destructor names identify their type by its constructor.
"""
import csv
import hashlib
import json
import pathlib
import re

import integration as build
from data_aliases import write_coff
from link_containers import verify_qv_library


def main():
    library_hash = verify_qv_library()
    library_definitions = {n for n, k in build.coff_symbols(build.REPO / "qvlib/obj/QvLib.obj")
                           if k in ("strong", "comdat")}
    with (build.REPO / "qvlib/score_table.tsv").open() as stream:
        rows = list(csv.DictReader(stream, delimiter="\t"))
    exact = [r for r in rows if r["score"] == "0" and r["symbol"] in library_definitions]
    units = json.loads((build.OUT / "units.json").read_text())
    references = set()
    for unit in units:
        if unit["returncode"]:
            continue
        references.update(n for n, k in build.coff_symbols(pathlib.Path(unit["object"]))
                          if k == "undefined")
    pairs, evidence, skipped = [], [], {}
    # Reviewed per-type virtual entries: the shared scoring GetTypeId body
    # incorrectly uses one global for every class and is never an alias owner.
    registry = json.loads((build.REPO / "tools/qv_field_data_registry.json").read_text())
    data_references = {n for n, k in build.coff_symbols(build.REPO / "data/data.obj")
                       if k == "undefined"}
    # Address identities come from the existing whole-library body/vtable map.
    # Traversal methods outside the parser range are functional upstream owners;
    # their documented differences include SGI diagnostic ANNOUNCE calls.
    by_address = {}
    for row in rows:
        if row["symbol"] in library_definitions:
            by_address.setdefault(row["address"].lower(), []).append(row)
    registered = {d["alias"] for d in registry["entries"]}
    for alias in sorted(data_references - registered):
        match = re.fullmatch(r"_fn_([0-9A-F]{6})", alias)
        if not match:
            continue
        candidates = by_address.get("00" + match[1].lower(), [])
        if len(candidates) != 1:
            continue
        row = candidates[0]
        functional_traversal = (re.fullmatch(r"\?traverse@Qv\w+@@UAEXPAVQvState@@@Z", row["symbol"])
                                and 0x49bf10 <= int(row["address"], 16) <= 0x49cd00)
        if row["score"] != "0" and not functional_traversal:
            continue
        pairs.append((alias, row["symbol"]))
        evidence.append(dict(alias=alias, mapping=row, owner="qvlib/obj/QvLib.lib",
                             functional_traversal=bool(functional_traversal),
                             review="qvlib/README.md: mapping and traversal modifications"))
    for decision in registry["entries"]:
        target = decision["target"]
        class_name = target.split("@")[1]
        candidates = [r for r in exact if r["symbol"] == target
                      and r["address"].lower() == decision["address"]]
        if (len(candidates) != 1 or not re.fullmatch(
                r"\?getFieldData@Qv\w+@@EAEPAVQvFieldData@@XZ", target)
                or decision["alias"] != "_fn_" + decision["address"].lstrip("0").upper()
                or decision["owner"] != "qvlib/obj/QvLib.obj"):
            raise ValueError("Invalid reviewed Qv field-data owner: " + target)
        if "?fieldData@" + class_name + "@@0PAVQvFieldData@@A" not in library_definitions:
            raise ValueError("Missing class-specific field table: " + class_name)
        if decision["alias"] in data_references:
            pairs.append((decision["alias"], target))
            evidence.append(dict(decision, mapping=candidates[0]))
    for name in sorted(references):
        ctor = re.fullmatch(r"\?\?0QvF([0-9a-f]{6})@@QAE@XZ", name)
        dtor = re.fullmatch(r"\?\?1Qv(?:Mem|Base)([0-9a-f]{6})@@[UQ]AE@XZ", name)
        field_dtor = re.fullmatch(r"\?\?1QvF([0-9a-f]{6})@@[UQ]AE@XZ", name)
        if not (ctor or dtor or field_dtor):
            continue
        match = ctor or dtor or field_dtor
        address = "00" + match[1]
        prefix = "??1" if dtor else "??0"
        candidates = [r for r in exact if r["address"].lower() == address
                      and r["symbol"].startswith(prefix)
                      and re.search(r"@@[UQ]AE@XZ$", r["symbol"])]
        if field_dtor and len(candidates) == 1:
            class_name = candidates[0]["symbol"][3:].split("@@", 1)[0]
            candidates = [r for r in exact if r["symbol"] in
                          ("??1" + class_name + "@@UAE@XZ", "??1" + class_name + "@@QAE@XZ")]
        if len(candidates) != 1:
            skipped[name] = "no unique exact zero-argument entry"
            continue
        entry = candidates[0]
        pairs.append((name, entry["symbol"]))
        evidence.append(dict(alias=name, target=entry["symbol"],
                             original_address=entry["address"], evidence=entry["evidence"]))
    exact_registry_path = build.REPO / 'tools/qv_exact_registry.json'
    if exact_registry_path.exists():
        from qv_exact_aliases import discover
        current = discover({alias for alias, _ in pairs})
        (build.OUT / 'qv-exact-candidates.json').write_text(json.dumps(current, indent=2))
        for alias, decision in json.loads(exact_registry_path.read_text()).items():
            if current['accepted'].get(alias) != decision:
                raise ValueError('Exact Qv caller/library proof changed; review registry: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, exact_call_decision=decision))
    pairs.sort()
    write_coff(pairs, build.OUT / "qv_alias.obj")
    (build.OUT / "qv_alias.json").write_text(json.dumps(dict(
        aliases=pairs, evidence=evidence, skipped=skipped,
        field_data_registry_sha256=hashlib.sha256(
            (build.REPO / "tools/qv_field_data_registry.json").read_bytes()).hexdigest(),
        qv_object_sha256=library_hash), indent=2))
    print("%d verified Qv entry aliases; %d skipped" % (len(pairs), len(skipped)))


if __name__ == "__main__":
    main()
