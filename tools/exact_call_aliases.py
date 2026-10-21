"""Discover ordinary aliases from complete relocation-masked original bodies.

Every referring relocation and the canonical callee must be proven against
current objects. This deliberately rejects partial windows, interior entries,
unmapped callers, non-call references, nonzero addends and conflicting owners.
"""
import collections
import hashlib
import json
import pathlib
import re
import struct

import integration as build
from code_pointer_bindings import original_reader
from project_eh import parse

# Already reviewed original owners in ordinary_alias_registry.json. Short RET
# bodies cannot establish identity alone; only these explicit addresses qualify.
EMPTY_OWNERS = {
    0x401660: '??1Vector3f@@QAE@XZ',
    0x402310: '??1MatrixLifetime@@QAE@XZ',
}
EMPTY_SOURCES = {
    0x401660: 'Vector3f/Vector3fOps.cpp',
    0x402310: 'Matrix4/MatrixDtor.cpp',
}
# Canonical pointer accessors: exact opcode/RET and the one named global
# relocation are checked, including its original absolute operand.
POINTER_GETTERS = {
    0x420680: ('?GetLand@@YAPAXXZ', '?g_land@@3PAXA', 0x4badb8),
    0x420690: ('?GetCurrentSimulation@@YAPAXXZ', '?g_currentSimulation@@3PAXA', 0x4badbc),
    0x4206a0: ('?GetWorld@@YAPAXXZ', '?g_world@@3PAXA', 0x4bada8),
    0x4206b0: ('?GetApplication@@YAPAXXZ', '?g_application@@3PAXA', 0x4badac),
    0x4206c0: ('?GetPanel@@YAPAXXZ', '?g_panel@@3PAXA', 0x4badb0),
    0x4206d0: ('?GetGameClock@@YAPAXXZ', '?g_gameClock@@3PAXA', 0x4badc0),
}


def pointer_getter_abi(alias):
    return bool(re.search(r'@@YAPA(?:X|[UV]\w+@@)XZ$', alias))


def exact_body(raw, symbols, sections, symbol, address, read):
    sec = sections[symbol['section']]
    flags = struct.unpack_from('<I', raw, sec['position'] + 36)[0]
    # Require a function packaged alone at its actual entry, never a suffix.
    if not flags & 0x20 or symbol['value'] != 0:
        return False
    if any(s['storage'] == 2 and s['section'] == symbol['section']
           and s['value'] != 0 for s in symbols.values()):
        return False
    body = raw[sec['offset']:sec['offset'] + sec['size']]
    if not body:
        return False
    mask = set()
    for offset, _, typ in sec['relocations']:
        if typ not in (6, 20) or offset + 4 > len(body):
            return False
        mask.update(range(offset, offset + 4))
    try:
        expected = read(address, len(body))
    except ValueError:
        return False
    reviewed_empty = (EMPTY_OWNERS.get(address) == symbol.get('name')
                      and body == b'\xc3' and expected == body and not mask)
    reviewed_getter = False
    if address in POINTER_GETTERS:
        name, global_name, global_address = POINTER_GETTERS[address]
        relocs = sec['relocations']
        if len(relocs) == 1 and relocs[0][0] == 1 and relocs[0][2] == 6:
            target = symbols[relocs[0][1]]
            reviewed_getter = (symbol.get('name') == name and body == b'\xa1' + bytes(4) + b'\xc3'
                and expected == b'\xa1' + struct.pack('<I', global_address) + b'\xc3'
                and target['name'] == global_name and target['storage'] == 2 and target['section'] == 0)
    return len(expected) == len(body) and (len(body) - len(mask) >= 3 or reviewed_empty or reviewed_getter) and all(
        a == b for i, (a, b) in enumerate(zip(body, expected)) if i not in mask)


def exact_shared_leaf(raw, symbols, sections, symbol, addresses, read):
    """An identical full body with no fixups can own several original entries.

    Never use multi-address code to infer a caller's destination. Only the
    callee is shared here, and every possible compiled definition must still
    pass the destination-specific owner check in discover().
    """
    if len(addresses) < 2 or sections[symbol['section']]['relocations']:
        return []
    return [a for a in addresses if exact_body(raw, symbols, sections, symbol, int(a, 16), read)]


def discover(include_qv=False):
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    units = json.loads((build.OUT / 'units.json').read_text())
    read = original_reader()
    defined = set(ownership['definitions'])
    existing = json.loads((build.REPO / 'tools/ordinary_alias_registry.json').read_text())
    for name in ('data', 'qv', 'runtime'):
        if name == 'qv' and include_qv:
            continue
        path = build.OUT / (name + '_alias.obj')
        if path.exists():
            existing.update({n: {} for n, kind in build.coff_symbols(path) if kind != 'undefined'})
    # Use the archived instruction boundaries only when all five instruction
    # bytes also agree with the actual original image being verified.
    direct_instructions = {}
    for line in (build.REPO.parent / 'nm/full.asm').read_text().splitlines():
        match = re.match(r'\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} ){4}[0-9a-f]{2})\s+(?:call|jmp)\s', line)
        if match:
            direct_instructions[int(match[1], 16)] = bytes.fromhex(match[2])
    calls = collections.defaultdict(list)
    failures = collections.defaultdict(list)
    exact_owners = collections.defaultdict(list)
    objects = {}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compiled inventory')
        raw = pathlib.Path(unit['object']).read_bytes()
        objects[unit['source']] = hashlib.sha256(raw).hexdigest()
        symbols, sections = parse(raw)
        entries = collections.defaultdict(list)
        for symbol in symbols.values():
            if symbol['storage'] != 2 or symbol['section'] <= 0:
                continue
            addresses = ownership['symbol_addresses'].get(symbol['name'], [])
            if len(addresses) > 1:
                mapped = [a for a in addresses if any(
                    e['symbol'] == symbol['name'] and e['source'] == unit['source']
                    for e in ownership['addresses'][a])]
                for address in exact_shared_leaf(raw, symbols, sections, symbol, mapped, read):
                    exact_owners[int(address, 16)].append(dict(
                        target=symbol['name'], owner=unit['source'], shared_leaf=True))
                continue
            if len(addresses) != 1:
                continue
            address = int(addresses[0], 16)
            if (EMPTY_OWNERS.get(address) != symbol['name']
                    and not any(e['symbol'] == symbol['name'] and e['source'] == unit['source']
                                for e in ownership['addresses'][addresses[0]])):
                continue
            if exact_body(raw, symbols, sections, symbol, address, read):
                entries[symbol['section']].append((symbol['name'], address))
                exact_owners[address].append(dict(target=symbol['name'], owner=unit['source']))
        for si, sec in sections.items():
            for offset, index, typ in sec['relocations']:
                target = symbols[index]
                alias = target['name']
                if target['section'] != 0 or target['storage'] != 2 or alias in defined or alias in existing:
                    continue
                if not alias.startswith('?'):
                    continue
                callers = entries.get(si, [])
                body = raw[sec['offset']:sec['offset'] + sec['size']]
                if (len(callers) != 1 or typ != 20 or offset < 1
                        or body[offset - 1] not in (0xe8, 0xe9)
                        or body[offset:offset + 4] != bytes(4)):
                    failures[alias].append(unit['source'])
                    continue
                caller, address = callers[0]
                instruction = address + offset - 1
                if direct_instructions.get(instruction) != read(instruction, 5):
                    failures[alias].append(unit['source'])
                    continue
                destination = address + offset + 4 + struct.unpack('<i', read(address + offset, 4))[0]
                calls[alias].append(dict(source=unit['source'], caller=caller,
                                         caller_address='%08x' % address, offset=offset,
                                         address='%08x' % destination))
    accepted, rejected = {}, {}
    for alias, sites in sorted(calls.items()):
        addresses = {s['address'] for s in sites}
        if alias in failures or len(addresses) != 1:
            rejected[alias] = 'unproven references or conflicting original destinations'
            continue
        address = next(iter(addresses))
        if int(address, 16) in POINTER_GETTERS and not pointer_getter_abi(alias):
            rejected[alias] = 'pointer getter requires cdecl, no arguments and pointer return'
            continue
        if int(address, 16) in EMPTY_OWNERS and not re.fullmatch(r'\?\?1\w+@@QAE@XZ', alias):
            rejected[alias] = 'reviewed empty owner requires a complete nonvirtual zero-argument destructor'
            continue
        if not 0x401000 <= int(address, 16) < 0x47e600:
            rejected[alias] = 'outside this game-method integration family'
            continue
        owners = exact_owners.get(int(address, 16), [])
        if int(address, 16) in EMPTY_OWNERS:
            owners = [o for o in owners if o['target'] == EMPTY_OWNERS[int(address, 16)]]
        targets = {o['target'] for o in owners}
        if len(targets) != 1:
            rejected[alias] = 'callee lacks unique complete exact-body owner'
            continue
        target = next(iter(targets))
        if set(ownership['definitions'].get(target, [])) != {o['owner'] for o in owners}:
            rejected[alias] = 'not every possible compiled callee definition is exact'
            continue
        # A multi-address leaf is safe only when every selectable compiled
        # definition has the exact complete original body and no relocations.
        shared_leaf = ownership['symbol_addresses'].get(target) != [address]
        if shared_leaf and not all(o.get('shared_leaf') for o in owners):
            rejected[alias] = 'multi-address callee is not an exact relocation-free body'
            continue
        owner = EMPTY_SOURCES.get(int(address, 16), sorted(o['owner'] for o in owners)[0])
        if owner not in {o['owner'] for o in owners}:
            raise ValueError('Reviewed empty destructor owner is absent')
        if int(address, 16) in POINTER_GETTERS:
            _, global_name, global_address = POINTER_GETTERS[int(address, 16)]
            data_aliases = dict(json.loads((build.OUT / 'data_alias.json').read_text())['aliases'])
            if not re.fullmatch(r'\?[gs]_%X@@\S+' % global_address, data_aliases.get(global_name, '')):
                raise ValueError('Canonical pointer getter global has no current original storage owner')
        accepted[alias] = dict(address=address, target=target, owner=owner,
            abi='Identical complete original caller and callee machine bodies outside COFF relocations; direct entry call/tail jump, zero addend, no this adjustment introduced.',
            evidence=sorted({s['source'] for s in sites}),
            reason='All current referring relocations resolve to this original entry; complete canonical callee body matches original.',
            exact_call_sites=sites)
        if shared_leaf:
            accepted[alias]['shared_leaf_evidence'] = dict(
                compared_original_entry=address,
                verified_definitions=sorted(o['owner'] for o in owners),
                declared_original_entries=ownership['symbol_addresses'][target],
                reason='Every possible compiled definition has the complete identical original bytes at this entry and zero relocations; sharing introduces no changed address operands or callees.')
    return dict(accepted=accepted, rejected=rejected, object_sha256=objects,
                call_sites=dict(calls), failed_call_sources=dict(failures),
                original_sha256=hashlib.sha256((build.REPO.parent / 'SqueezilsGameFiles/SQZ.EXE').read_bytes()).hexdigest())


def main():
    report = discover()
    (build.OUT / 'exact-call-candidates.json').write_text(json.dumps(report, indent=2))
    print('Exact call aliases: %d accepted, %d rejected' % (len(report['accepted']), len(report['rejected'])))
    import sys
    if '--register' in sys.argv:
        (build.REPO / 'tools/exact_call_registry.json').write_text(json.dumps(report['accepted'], indent=2) + '\n')


if __name__ == '__main__':
    main()
