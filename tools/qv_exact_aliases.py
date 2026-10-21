"""Prove local parser spellings against exact original and current QvLib code."""
import collections
import csv
import hashlib
import json
import sys

import integration as build
from code_pointer_bindings import original_reader
from exact_call_aliases import discover as discover_calls, exact_body
from link_containers import verify_qv_library
from project_eh import parse


def select(sites, failed, candidates):
    if failed or not sites:
        return None
    addresses = {s['address'] for s in sites}
    if len(addresses) != 1:
        return None
    address = next(iter(addresses))
    rows = candidates.get(address, [])
    return (address, rows[0]) if len(rows) == 1 else None


def discover(existing):
    library_hash = verify_qv_library()
    raw = (build.REPO / 'qvlib/obj/QvLib.obj').read_bytes()
    symbols, sections = parse(raw)
    definitions = collections.defaultdict(list)
    for symbol in symbols.values():
        if symbol['storage'] == 2 and symbol['section'] > 0:
            definitions[symbol['name']].append(symbol)
    read = original_reader()
    candidates = collections.defaultdict(list)
    with (build.REPO / 'qvlib/score_table.tsv').open() as stream:
        for row in csv.DictReader(stream, delimiter='\t'):
            address = int(row['address'], 16)
            if row['score'] != '0' or not 0x47e600 <= address < 0x48e000:
                continue
            defs = definitions.get(row['symbol'], [])
            if len(defs) != 1 or not exact_body(raw, symbols, sections, defs[0], address, read):
                continue
            candidates['%08x' % address].append(row['symbol'])
    calls = discover_calls(include_qv=True)
    accepted, rejected = {}, {}
    for alias, sites in sorted(calls['call_sites'].items()):
        if alias in existing or not any(0x47e600 <= int(s['address'], 16) < 0x48e000 for s in sites):
            continue
        binding = select(sites, calls['failed_call_sources'].get(alias, []), candidates)
        if binding is None:
            rejected[alias] = 'unproven/conflicting calls or no unique exact current QvLib entry'
            continue
        address, target = binding
        if alias == target:
            # Already provided by the canonical library; an identity weak
            # external is unnecessary and cannot be a new binding.
            continue
        accepted[alias] = dict(address=address, target=target, owner='qvlib/obj/QvLib.lib',
            exact_call_sites=sites,
            reason='All original direct-call destinations agree; complete caller and current QvLib callee bodies match original outside COFF relocations; no callable adapter or this adjustment.')
    return dict(accepted=accepted, rejected=rejected,
                qv_object_sha256=library_hash, original_sha256=calls['original_sha256'],
                object_sha256=calls['object_sha256'])


def main():
    existing = {a for a, _ in json.loads((build.OUT / 'qv_alias.json').read_text())['aliases']}
    registry = build.REPO / 'tools/qv_exact_registry.json'
    if registry.exists():
        existing.difference_update(json.loads(registry.read_text()))
    report = discover(existing)
    (build.OUT / 'qv-exact-candidates.json').write_text(json.dumps(report, indent=2))
    print('Exact Qv aliases: %d accepted, %d rejected' % (len(report['accepted']), len(report['rejected'])))
    if '--register' in sys.argv:
        registry.write_text(json.dumps(report['accepted'], indent=2) + '\n')


if __name__ == '__main__':
    main()
