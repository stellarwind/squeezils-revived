"""Canonical Qv node lifetimes and supporting list/field-table interfaces.

Generated class identity is recovered from its addressed ctor/dtor entry, never
similar method names or RET bodies. Ordinary COMDAT projection removes local
implementations, preserving scoring fixtures and existing relocation checks.
"""
import csv
import hashlib
import json
import pathlib
import re

import integration as build
from link_containers import verify_qv_library

BASES = {'QvNodeB': 'QvNode', 'QvNodeU': 'QvNode',
         'QvGroupBase': 'QvGroup', 'QvGroupBaseU': 'QvGroup', 'QvGroupX': 'QvGroup',
         'QvInter': 'QvGroup', 'QvGroupN': 'QvUnknownNode',
         'QvUnknownU': 'QvUnknownNode', 'QvGenericC': 'QvUnknownNode'}
OWNERS = {'Qv/' + s + '.cpp' for s in ('Group', 'Group2', 'Unknown', 'Inter', 'NodeBase', 'NodeRead')}
DIRECT = {
    '?ReadChildren@QvGroupG2@@QAEHPAVQvInputG2@@@Z': 0x482110,
    '?Read@QvNodeN@@QAEHPAUQvInputN@@@Z': 0x481fd0,
    '?ReadFieldsOnly@QvNodeN2@@QAEHPAUQvInputN@@@Z': 0x47ec70,
    '?SetClass@QvUnknownU@@QAEXPAD@Z': 0x482df0,
    '?SetClass@QvGenericC@@QAEXPAD@Z': 0x482df0,
    '?GetName@QvNode@@QAEPAXXZ': 0x47e780,
    '?ReadWrlTop@@YAHPAUWrlInput@@PAPAUQvNode@@@Z': 0x481c30,
    '??0QvPListN@@QAE@XZ': 0x482a60, '??0QvPListU@@QAE@XZ': 0x482a60,
    '??1QvPListU@@QAE@XZ': 0x482a80,
    '??1QvFieldDataU@@QAE@XZ': 0x480630, '??1QvKids@@QAE@XZ': 0x480630,
    '?Grow@QvEntryList@@QAEXH@Z': 0x482b10, '?Grow@QvNodeList@@QAEXH@Z': 0x482b10,
    '?Grow@QvPListN@@QAEXH@Z': 0x482b10,
    '?Find@QvPListN@@QAEHPAX@Z': 0x482a90, '?Remove@QvPListN@@QAEXH@Z': 0x482ac0,
}


def tables(rows):
    by_address, ctors, dtors = {}, {}, {}
    for row in rows:
        address = int(row['address'], 16)
        if address in by_address:
            raise ValueError('Ambiguous canonical entry address')
        by_address[address] = row['symbol']
        match = re.fullmatch(r'\?\?([01])(Qv\w+)@@[QU]AE@XZ', row['symbol'])
        if match:
            (ctors if match[1] == '0' else dtors)[match[2]] = address
    return by_address, ctors, dtors


def class_identity(local, maps):
    by_address, ctors, dtors = maps
    if local in BASES:
        return BASES[local]
    match = re.fullmatch(r'Qv(?:NodeC|C)([0-9a-f]{6})', local)
    kind = '0'
    if not match:
        match = re.fullmatch(r'QvNode([0-9a-f]{6})', local)
        kind = '1'
    if not match:
        return None
    address = int(match[1], 16)
    name = by_address.get(address, '')
    entry = re.fullmatch(r'\?\?' + kind + r'(Qv\w+)@@[QU]AE@XZ', name)
    if not entry:
        raise ValueError('Generated class does not identify a canonical lifetime entry: ' + local)
    canonical = entry[1]
    ctor = ctors.get(canonical, 0)
    if not (ctor in (0x47e660, 0x481e00, 0x482c70) or 0x482ed0 <= ctor <= 0x488820):
        raise ValueError('Generated identity is not a node class: ' + local)
    return canonical


def binding(alias, maps):
    by_address, ctors, dtors = maps
    if alias in DIRECT:
        address = DIRECT[alias]
        return dict(address='%08x' % address, target=by_address[address], canonical_class=None)
    match = re.fullmatch(r'\?\?([01])(Qv\w+)@@([QU]AE)@XZ', alias)
    if not match:
        return None
    kind, local, abi = match.groups()
    canonical = class_identity(local, maps)
    if not canonical:
        return None
    if (kind == '0' and abi != 'QAE') or (kind == '1' and abi != 'UAE'):
        raise ValueError('Node lifetime has incompatible constructor/virtual-destructor ABI: ' + alias)
    address = (ctors if kind == '0' else dtors).get(canonical)
    if address is None:
        raise ValueError('Canonical class has no matching lifetime entry: ' + canonical)
    return dict(address='%08x' % address, target=by_address[address], canonical_class=canonical)


def discover():
    library_hash = verify_qv_library()
    units = json.loads((build.OUT / 'units.json').read_text())
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    existing = {}
    for name in ('qv_exact_registry.json', 'qv_name_registry.json', 'qv_input_registry.json'):
        existing.update(json.loads((build.REPO / 'tools' / name).read_text()))
    with (build.REPO / 'qvlib/score_table.tsv').open() as stream:
        maps = tables(list(csv.DictReader(stream, delimiter='\t')))
    definitions = {n for n, k in build.coff_symbols(build.REPO / 'qvlib/obj/QvLib.obj') if k in ('strong', 'comdat')}
    sources = {}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compiled inventory')
        for alias, kind in build.coff_symbols(pathlib.Path(unit['object'])):
            if kind in ('undefined', 'strong', 'comdat') and alias not in existing:
                decision = binding(alias, maps)
                if decision and decision['target'] != alias:
                    sources.setdefault(alias, set()).add(unit['source'])
    source_hashes = {p.relative_to(build.REPO).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                    for folder in ('qvlib/include', 'qvlib/src', 'qvlib/shim')
                    for p in sorted((build.REPO / folder).rglob('*')) if p.is_file()}
    canonical_sources = hashlib.sha256(json.dumps(source_hashes, sort_keys=True).encode()).hexdigest()
    accepted = {}
    for alias, referring in sorted(sources.items()):
        decision = binding(alias, maps)
        if decision['target'] not in definitions:
            raise ValueError('Canonical node/list owner is not exported: ' + alias)
        owners = ownership['definitions'].get(alias, [])
        if owners and (ownership['symbol_addresses'].get(alias) != [decision['address']]
                       or any(s not in OWNERS and not re.fullmatch(r'Qv/gen/[CD]_[0-9a-f]{6}\.cpp', s) for s in owners)):
            raise ValueError('Loose node implementation has an unreviewed address/owner: ' + alias)
        accepted[alias] = dict(**decision, loose_owners=owners, owner='qvlib/obj/QvLib.lib',
            source_sha256={s: hashlib.sha256((build.SRC / s).read_bytes()).hexdigest() for s in sorted(referring)},
            canonical_source_tree_sha256=canonical_sources, qv_object_sha256=library_hash,
            abi='Ctor and complete virtual dtor: zero-argument thiscall, same addressed node class, no this adjustment. Node prefix 24 bytes; canonical factory/lifetime owners initialize canonical vtables and class-specific field tables. Direct helpers retain reviewed thiscall/cdecl pointer and integer widths; GetName returns the name-handle address in EAX, not character data. Pointer lists are {items,count,capacity} (12 bytes); field table holds two lists (24 bytes); WrlInput is 72 bytes.',
            reason='Generated class identity comes from its explicit original ctor/dtor entry and canonical class mapping; current loose owners and source hashes agree. Functional canonical code is accepted without a byte-score gate.')
    return accepted


def reviewed():
    path = build.REPO / 'tools/qv_node_registry.json'
    if not path.exists():
        return {}
    saved = json.loads(path.read_text())
    current = discover()
    for alias, decision in saved.items():
        if current.get(alias) != decision:
            raise ValueError('Qv node/lifetime ownership evidence changed: ' + alias)
    return saved


if __name__ == '__main__':
    import sys
    decisions = discover()
    (build.OUT / 'qv-node-candidates.json').write_text(json.dumps(decisions, indent=2))
    print('%d reviewed node/lifetime/list bindings' % len(decisions))
    if '--register' in sys.argv:
        (build.REPO / 'tools/qv_node_registry.json').write_text(json.dumps(decisions, indent=2) + '\n')
