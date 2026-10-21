"""Bind global spellings using every current reference and original PE fixups.

No storage is synthesized. A complete exact caller body establishes the
reference offset; a PE HIGHLOW record proves that its original operand is an
address. Only zero-addend COFF DIR32 references to an exact extracted object
entry qualify. Every occurrence of a decorated name must agree.
"""
import collections
import hashlib
import json
import pathlib
import re
import struct
import sys

import integration as build
from code_pointer_bindings import original_reader
from exact_call_aliases import exact_body
from project_eh import parse


def pe_fixups(raw, read):
    pe = struct.unpack_from('<I', raw, 60)[0]
    opt = pe + 24
    base = struct.unpack_from('<I', raw, opt + 28)[0]
    rva, size = struct.unpack_from('<II', raw, opt + 96 + 5 * 8)
    blob = read(base + rva, size)
    result = set()
    offset = 0
    while offset < len(blob):
        page, length = struct.unpack_from('<II', blob, offset)
        if length < 8 or length % 2 or offset + length > len(blob):
            raise ValueError('Invalid original PE relocation block')
        for p in range(offset + 8, offset + length, 2):
            entry = struct.unpack_from('<H', blob, p)[0]
            if entry >> 12 == 3:
                result.add(base + page + (entry & 4095))
            elif entry >> 12:
                raise ValueError('Unsupported PE fixup')
        offset += length
    return result


def resolve(sites, failures, owners):
    if failures or not sites:
        return None
    addresses = {s['address'] for s in sites}
    if len(addresses) != 1:
        return None
    address = next(iter(addresses))
    targets = owners.get(address, [])
    return (address, targets[0]) if len(targets) == 1 else None


def discover(existing):
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    units = json.loads((build.OUT / 'units.json').read_text())
    original = (build.REPO.parent / 'SqueezilsGameFiles/SQZ.EXE').read_bytes()
    read = original_reader()
    fixups = pe_fixups(original, read)
    data = (build.REPO / 'data/data.obj').read_bytes()
    ds, _ = parse(data)
    owners = collections.defaultdict(list)
    for symbol in ds.values():
        match = re.fullmatch(r'\?[gs]_([0-9A-Fa-f]+)@@\S+', symbol['name'])
        if match and symbol['section'] > 0 and symbol['storage'] == 2:
            owners['%08x' % int(match[1], 16)].append(symbol['name'])
    defined = set(ownership['definitions'])
    calls, failures = collections.defaultdict(list), collections.defaultdict(list)
    hashes = {}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compiled inventory')
        raw = pathlib.Path(unit['object']).read_bytes()
        hashes[unit['source']] = hashlib.sha256(raw).hexdigest()
        symbols, sections = parse(raw)
        entries = collections.defaultdict(list)
        for symbol in symbols.values():
            if symbol['storage'] != 2 or symbol['section'] <= 0:
                continue
            addresses = ownership['symbol_addresses'].get(symbol['name'], [])
            if len(addresses) != 1:
                continue
            address = int(addresses[0], 16)
            if not any(e['source'] == unit['source'] and e['symbol'] == symbol['name']
                       for e in ownership['addresses'][addresses[0]]):
                continue
            if exact_body(raw, symbols, sections, symbol, address, read):
                entries[symbol['section']].append((symbol['name'], address))
        for si, sec in sections.items():
            for offset, index, typ in sec['relocations']:
                symbol = symbols[index]
                alias = symbol['name']
                if (symbol['storage'] != 2 or symbol['section'] != 0 or alias in defined
                        or alias in existing or not re.fullmatch(r'\?\w+@@3\S+', alias)):
                    continue
                callers = entries.get(si, [])
                operand = raw[sec['offset'] + offset:sec['offset'] + offset + 4]
                if (len(callers) != 1 or typ != 6 or operand != bytes(4)
                        or callers[0][1] + offset not in fixups):
                    failures[alias].append(unit['source'])
                    continue
                caller, address = callers[0]
                destination = struct.unpack('<I', read(address + offset, 4))[0]
                calls[alias].append(dict(source=unit['source'], caller=caller,
                    caller_address='%08x' % address, offset=offset, address='%08x' % destination))
    accepted, rejected = {}, {}
    for alias in sorted(calls.keys() | failures.keys()):
        binding = resolve(calls[alias], failures[alias], owners)
        if binding is None:
            rejected[alias] = 'unproven/conflicting reference, nonzero addend or no unique extracted entry'
            continue
        address, target = binding
        accepted[alias] = dict(address=address, target=target, owner='data/data.obj',
            evidence=sorted({s['source'] for s in calls[alias]}), exact_data_sites=calls[alias],
            reason='Every referencing relocation has a complete exact caller, zero COFF DIR32 addend and original PE HIGHLOW fixup to this extracted object entry.')
    return dict(accepted=accepted, rejected=rejected, object_sha256=hashes,
                original_sha256=hashlib.sha256(original).hexdigest(),
                data_sha256=hashlib.sha256(data).hexdigest())


def main():
    # Baseline aliases are regenerated from current references and declarations.
    existing = {a for a, _ in json.loads((build.OUT / 'data_alias.json').read_text())['aliases']}
    registry = build.REPO / 'tools/exact_data_registry.json'
    if registry.exists():
        existing.difference_update(json.loads(registry.read_text()))
    report = discover(existing)
    (build.OUT / 'exact-data-candidates.json').write_text(json.dumps(report, indent=2))
    print('Exact data aliases: %d accepted, %d rejected' % (len(report['accepted']), len(report['rejected'])))
    if '--register' in sys.argv:
        registry.write_text(json.dumps(report['accepted'], indent=2) + '\n')


if __name__ == '__main__':
    main()
