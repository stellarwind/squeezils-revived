"""Reviewed Activity step ABI bindings, checked against original vtable slots.

All entries take ECX=this and one four-byte argument, return int in EAX and
pop four bytes. Virtualness and the unused Die argument's local int spelling
do not change the entry ABI. No this adjustment or wrapper is introduced.
"""
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


# Full decorated alias, original vtable slot (or same destination for direct call),
# original entry, canonical decorated symbol, canonical source.
BINDINGS = [
    ('?StepAir@WalkActivity@@UAEHPAUAAirControl@@@Z', 0x49d08c, 0x405200,
     '?return24a@CameraSoulStubs@@QAEHH@Z', 'CameraSoul/CameraSoul.cpp'),
    ('?StepGround@MoveActivity@@UAEHPAUAGroundControl@@@Z', 0x49d080, 0x401430,
     '?StepGround@MoveActivityFlat@@QAEHPAUGroundControl@@@Z', 'Souls2/StepGround.cpp'),
    ('?MoveStep@TT@@QAEHPAUGroundControl@@@Z', 0x49d080, 0x401430,
     '?StepGround@MoveActivityFlat@@QAEHPAUGroundControl@@@Z', 'Souls2/StepGround.cpp'),
    ('?StepGround@StopActivity@@UAEHPAUAGroundControl@@@Z', 0x49d0e0, 0x401be0,
     '?StepStand@StandAct@@QAEHPAH@Z', 'Souls2/Misc2.cpp'),
    ('?StepGround@DieActivity@@UAEHPAUAGroundControl@@@Z', 0x49d0a0, 0x401ef0,
     '?Execute@DieActivity@@UAEHH@Z', 'Behavior2/Die.cpp'),
]


def discover():
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    units = json.loads((build.OUT / 'units.json').read_text())
    objects, refs = {}, {}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compiled inventory')
        raw = pathlib.Path(unit['object']).read_bytes()
        symbols, sections = parse(raw)
        objects[unit['source']] = raw, symbols, sections
        for symbol in symbols.values():
            if symbol['section'] == 0 and symbol['storage'] == 2:
                refs.setdefault(symbol['name'], set()).add(unit['source'])
    read = original_reader()
    fixups = pe_fixups((build.REPO.parent / 'SqueezilsGameFiles/SQZ.EXE').read_bytes(), read)
    result = {}
    for alias, slot, address, target, owner in BINDINGS:
        key = '%08x' % address
        if struct.unpack('<I', read(slot, 4))[0] != address:
            raise ValueError('Original Activity slot changed: ' + alias)
        if alias in ownership['definitions'] or alias not in refs:
            raise ValueError('Activity alias no longer exclusively undefined: ' + alias)
        if (ownership['symbol_addresses'].get(target) != [key]
                or not any(e['symbol'] == target and e['source'] == owner
                           for e in ownership['addresses'].get(key, []))):
            raise ValueError('Activity owner changed: ' + alias)
        hashes = {}
        for source in ownership['definitions'][target]:
            raw, symbols, sections = objects[source]
            definitions = [s for s in symbols.values() if s['name'] == target and s['section'] > 0]
            if (len(definitions) != 1
                    or not exact_body(raw, symbols, sections, definitions[0], address, read)
                    or not relocatable(sections[definitions[0]['section']], address, fixups)):
                raise ValueError('Activity owner is not a complete relocatable original body: ' + alias)
            hashes[source] = object_digest(raw)
        evidence = set(refs[alias]) | {owner, 'Activity/Activity.h'}
        if alias.startswith('?StepGround@DieActivity'):
            evidence.add('Behavior2/Activity.h')
        result[alias] = dict(address=key, target=target, owner=owner,
            original_vtable_slot='%08x' % slot,
            source_sha256={s: hashlib.sha256((build.SRC / s).read_bytes()).hexdigest()
                           for s in sorted(evidence)},
            owner_object_without_timestamp_sha256=hashes,
            abi='thiscall; one four-byte argument; int EAX; ret 4; no this adjustment',
            layout=('body +4, soul +8; Move direction/turn/amount/gait +2ac/+2b0/+2b4/+2b8; '
                    'Die done +2ac; ground output two four-byte floats; air step ignores this/argument'),
            reason='Reviewed original vtable entry and local layouts; complete original owner body and symbolic absolute operands rechecked.')
    return result


if __name__ == '__main__':
    import sys
    result = discover()
    if '--register' in sys.argv:
        (build.REPO / 'tools/activity_registry.json').write_text(json.dumps(result, indent=2) + '\n')
    print('%d verified Activity step bindings' % len(result))
