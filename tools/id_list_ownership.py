"""Canonical Set<int,10> constructor/destructor and both complete vtables."""
import hashlib
import json
import pathlib
import struct
import integration as build
from code_pointer_bindings import original_reader
from destructor_aliases import object_digest, relocatable
from exact_call_aliases import exact_body
from exact_data_aliases import pe_fixups
from project_eh import parse

BINDINGS = {
    '??0IdList@@QAE@XZ': (0x45eef0, '??0?$Set@H$09@@QAE@XZ', 'Container2/gen/SC_i10.cpp'),
    '??1IdList@@QAE@XZ': (0x45efa0, '??1?$Set@H$09@@QAE@XZ', 'Container/gen/SetDtor.cpp'),
}
TABLES = {
    '??_7?$Set@H$09@@6B?$Array@H@@@': (0x4a009c, 9),
    '??_7?$Set@H$09@@6BBufferObject@@@': (0x4a0084, 4),
}
TABLE_OWNER = 'Container2/gen/SC_i10.cpp'


def require_slot(symbol, address, mapped):
    if '%08x' % address not in mapped.get(symbol, []):
        raise ValueError('Original vtable slot identity disagrees with compiled target: ' + symbol)


def discover():
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    units = json.loads((build.OUT / 'units.json').read_text())
    objects, references = {}, {}
    for unit in units:
        if unit['returncode'] or unit['source'] == 'Container2/IdListCtor.cpp':
            raise ValueError('Incomplete inventory or retired ID-list constructor still compiled')
        raw = pathlib.Path(unit['object']).read_bytes()
        symbols, sections = parse(raw)
        objects[unit['source']] = raw, symbols, sections
        for symbol in symbols.values():
            if symbol['section'] == 0 and symbol['storage'] == 2:
                references.setdefault(symbol['name'], set()).add(unit['source'])
    read = original_reader()
    fixups = pe_fixups((build.REPO.parent / 'SqueezilsGameFiles/SQZ.EXE').read_bytes(), read)
    raw, symbols, sections = objects[TABLE_OWNER]
    table_proof = {}
    for name, (address, count) in TABLES.items():
        definitions = [s for s in symbols.values() if s['name'] == name and s['section'] > 0]
        if len(definitions) != 1:
            raise ValueError('Canonical ID-list table missing: ' + name)
        symbol = definitions[0]
        section = sections[symbol['section']]
        slots = sorted((off - symbol['value'], symbols[index]['name'], kind)
                       for off, index, kind in section['relocations'] if off >= symbol['value'])
        if len(slots) != count or [x[0] for x in slots] != list(range(0, count * 4, 4)):
            raise ValueError('Unexpected ID-list vtable layout: ' + name)
        evidence = []
        for offset, target, kind in slots:
            if kind != 6:
                raise ValueError('Vtable entry is not an absolute symbolic relocation')
            payload = section['offset'] + symbol['value'] + offset
            if raw[payload:payload + 4] != bytes(4):
                raise ValueError('Vtable entry has a nonzero interior addend')
            destination = struct.unpack('<I', read(address + offset, 4))[0]
            require_slot(target, destination, ownership['symbol_addresses'])
            evidence.append(dict(offset=offset, target=target, original_address='%08x' % destination))
        table_proof[name] = dict(address='%08x' % address, owner=TABLE_OWNER, slots=evidence,
                                owner_object_without_timestamp_sha256=object_digest(raw))
    result = {}
    for alias, (address, target, owner) in BINDINGS.items():
        key = '%08x' % address
        if alias in ownership['definitions'] or alias not in references:
            raise ValueError('ID-list alias is no longer exclusively undefined: ' + alias)
        if key not in ownership['symbol_addresses'].get(target, []):
            raise ValueError('Canonical ID-list entry mapping changed: ' + alias)
        if not any(e['symbol'] == target and e['source'] == owner for e in ownership['addresses'].get(key, [])):
            raise ValueError('Canonical ID-list source is not addressed owner')
        hashes = {}
        for source in ownership['definitions'][target]:
            code, syms, secs = objects[source]
            defs = [s for s in syms.values() if s['name'] == target and s['section'] > 0]
            if (len(defs) != 1 or not exact_body(code, syms, secs, defs[0], address, read)
                    or not relocatable(secs[defs[0]['section']], address, fixups)):
                raise ValueError('ID-list owner no longer matches complete original code')
            for offset, index, kind in secs[defs[0]['section']]['relocations']:
                name = syms[index]['name']
                if name in TABLES and (kind != 6 or struct.unpack('<I', read(address + offset, 4))[0] != TABLES[name][0]):
                    raise ValueError('ID-list constructor/destructor stores different vtable')
                # The legacy SetDtor scoring map also lists string variants.
                # Pin this int/ID=10 lifetime by its actual original vtables AND
                # typed base destructor, not by relocation-masked similarity.
                if alias.startswith('??1') and kind == 20:
                    expected = {'??1?$PrimitiveArray@H$09@@QAE@XZ': 0x46ebf0,
                                '___CxxFrameHandler': 0x48e5f0}.get(name)
                    destination = address + offset + 4 + struct.unpack('<i', read(address + offset, 4))[0]
                    if expected is None or destination != expected:
                        raise ValueError('ID-list destructor calls a different typed lifetime/runtime entry')
            hashes[source] = object_digest(code)
        sources = sorted(references[alias])
        if not set(sources) <= {'Sim2/LocCtor.cpp', 'Sim2/LandInit.cpp'}:
            raise ValueError('Unreviewed ID-list caller declaration')
        result[alias] = dict(address=key, target=target, owner=owner,
            source_sha256={s: hashlib.sha256((build.SRC / s).read_bytes()).hexdigest() for s in sources},
            owner_object_without_timestamp_sha256=hashes, vtable_policy=table_proof,
            abi='Complete thiscall constructor/destructor; no stack arguments or this adjustment; no deleting flag.',
            layout='Set<int,10> size 0x14; Array primary vptr/count/capacity/data +0/+4/+8/+c; '
                   'BufferObject secondary vptr +10. LocationSimulation ID member at +c; active follows at +20.',
            reason='Retired exact duplicate constructor; canonical complete bodies and all 13 original vtable slots verified.')
    return result


def reviewed():
    current = discover()
    registry = json.loads((build.REPO / 'tools/id_list_registry.json').read_text())
    if current != registry:
        raise ValueError('ID-list body/caller/vtable proof changed; review registry')
    return current


if __name__ == '__main__':
    import sys
    result = discover()
    (build.OUT / 'id-list-candidates.json').write_text(json.dumps(dict(accepted=result, rejected={}), indent=2))
    if '--register' in sys.argv:
        (build.REPO / 'tools/id_list_registry.json').write_text(json.dumps(result, indent=2) + '\n')
    print('Two ID-list lifecycle bindings and 13 canonical vtable slots verified')
