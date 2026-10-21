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
    getter_registry = build.REPO / 'tools/global_getter_registry.json'
    if getter_registry.exists():
        from global_getter_aliases import discover as discover_getters
        current_getters = discover_getters()
        for alias, decision in json.loads(getter_registry.read_text()).items():
            if current_getters['accepted'].get(alias) != decision:
                raise ValueError('Global getter proof changed; review registry: ' + alias)
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate global getter alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, global_getter_decision=decision))
        pairs.sort()
    lifetime_registry = build.REPO / 'tools/math_lifetime_registry.json'
    if lifetime_registry.exists():
        from math_lifetime_aliases import discover as discover_lifetimes
        current_lifetimes = discover_lifetimes()
        for alias, decision in json.loads(lifetime_registry.read_text()).items():
            if current_lifetimes['accepted'].get(alias) != decision:
                raise ValueError('Math destructor proof changed; review registry: ' + alias)
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate math destructor alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, math_lifetime_decision=decision))
        pairs.sort()
    vector_registry = build.REPO / 'tools/vector_registry.json'
    if vector_registry.exists():
        from vector_aliases import discover as discover_vectors
        current_vectors = discover_vectors()
        for alias, decision in json.loads(vector_registry.read_text()).items():
            if current_vectors['accepted'].get(alias) != decision:
                raise ValueError('Vector arithmetic proof changed; review registry: ' + alias)
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate vector arithmetic alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, vector_decision=decision))
        pairs.sort()
    matrix_registry = build.REPO / 'tools/matrix_registry.json'
    if matrix_registry.exists():
        from matrix_aliases import discover as discover_matrices
        current_matrices = discover_matrices()
        for alias, decision in json.loads(matrix_registry.read_text()).items():
            if current_matrices['accepted'].get(alias) != decision:
                raise ValueError('Affine matrix proof changed; review registry: ' + alias)
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate affine matrix alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, matrix_decision=decision))
        pairs.sort()
    destructor_registry = build.REPO / 'tools/destructor_registry.json'
    if destructor_registry.exists():
        from destructor_aliases import discover as discover_destructors
        current_destructors = discover_destructors()
        (build.OUT / 'destructor-candidates.json').write_text(json.dumps(current_destructors, indent=2))
        for alias, decision in json.loads(destructor_registry.read_text()).items():
            if current_destructors['accepted'].get(alias) != decision:
                raise ValueError('Complete-destructor proof changed; review registry: ' + alias)
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate complete-destructor alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, destructor_decision=decision))
        pairs.sort()
    activity_registry = build.REPO / 'tools/activity_registry.json'
    if activity_registry.exists():
        from activity_aliases import discover as discover_activities
        current_activities = discover_activities()
        if current_activities != json.loads(activity_registry.read_text()):
            raise ValueError('Activity step proof changed; review activity registry')
        for alias, decision in sorted(current_activities.items()):
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate Activity step alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, activity_decision=decision))
        pairs.sort()
    surface_registry = build.REPO / 'tools/surface_registry.json'
    if surface_registry.exists():
        from surface_aliases import discover as discover_surfaces
        surfaces = discover_surfaces()
        (build.OUT / 'surface-candidates.json').write_text(json.dumps(surfaces, indent=2))
        for alias, decision in json.loads(surface_registry.read_text()).items():
            if surfaces['accepted'].get(alias) != decision:
                raise ValueError('Surface interface proof changed; review registry: ' + alias)
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate surface interface alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, surface_decision=decision))
        pairs.sort()
    application_registry = build.REPO / 'tools/application_registry.json'
    if application_registry.exists():
        from application_aliases import discover as discover_application
        application = discover_application()
        (build.OUT / 'application-candidates.json').write_text(json.dumps(application, indent=2))
        for alias, decision in json.loads(application_registry.read_text()).items():
            if application['accepted'].get(alias) != decision:
                raise ValueError('Application interface proof changed; review registry: ' + alias)
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate application interface alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, application_decision=decision))
        pairs.sort()
    if (build.REPO / 'tools/id_list_registry.json').exists():
        from id_list_ownership import reviewed as reviewed_id_list
        decisions = reviewed_id_list()
        (build.OUT / 'id-list-candidates.json').write_text(json.dumps(dict(accepted=decisions, rejected={}), indent=2))
        for alias, decision in decisions.items():
            if alias in {a for a, _ in pairs}:
                raise ValueError('Duplicate ID-list alias: ' + alias)
            pairs.append((alias, decision['target']))
            evidence.append(dict(alias=alias, id_list_decision=decision))
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
