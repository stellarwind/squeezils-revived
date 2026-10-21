"""Reconstruct omitted MSVC TypeDescriptors; never hierarchy/vtable code.

Address annotations identify the original descriptor, whose complete name and
two-pointer header are verified. Existing compiler-emitted descriptors own the
name when present. Otherwise generate the real metadata with a symbolic CRT
type_info vtable relocation, not a copied original image pointer.
"""
import collections
import hashlib
import json
import pathlib
import os
import re
import struct
import subprocess
import sys
import bisect

import integration as build
from code_pointer_bindings import original_reader
from data_aliases import write_coff
from project_eh import parse
from source_annotations import annotated

VTABLE = '??_7type_info@@6B@'


def descriptor(read, address):
    header = read(address, 8)
    if struct.unpack('<II', header) != (0x4a177c, 0):
        raise ValueError('Not an original MSVC type descriptor header')
    data = read(address + 8, 256)
    if b'\0' not in data:
        raise ValueError('Unterminated original type name')
    name = data.split(b'\0', 1)[0]
    if not re.fullmatch(rb'\.\?A[VU][!-~]+@@', name):
        raise ValueError('Not an original class/struct type descriptor name')
    return name.decode('ascii')


def emit(records, path):
    payload, relocations, definitions = bytearray(), [], []
    for name, raw_name in sorted(records):
        while len(payload) % 4:
            payload.append(0)
        offset = len(payload)
        definitions.append((name, offset))
        payload.extend(bytes(8) + raw_name.encode('ascii') + b'\0')
        relocations.append(struct.pack('<IIH', offset, 0, 6))
    strings, symbols = bytearray(4), bytearray()
    for name, value, section in [(VTABLE, 0, 0)] + [(n, v, 1) for n, v in definitions]:
        encoded = name.encode('ascii')
        field = struct.pack('<II', 0, len(strings))
        strings.extend(encoded + b'\0')
        symbols.extend(field + struct.pack('<IhHBB', value, section, 0, 2, 0))
    struct.pack_into('<I', strings, 0, len(strings))
    relocation_data = b''.join(relocations)
    header = struct.pack('<HHIIIHH', 0x14c, 1, 0, 60 + len(payload) + len(relocation_data),
                         1 + len(definitions), 0, 0)
    section = b'.data\0\0\0' + struct.pack('<IIIIIIHHI', 0, 0, len(payload), 60,
        60 + len(payload), 0, len(relocations), 0, 0xc0300040)
    path.write_bytes(header + section + payload + relocation_data + symbols + strings)


def existing_owner(raw, symbol, symbols, sections, name):
    sec = sections[symbol['section']]
    offset = symbol['value']
    expected = bytes(8) + name.encode('ascii') + b'\0'
    actual = raw[sec['offset'] + offset:sec['offset'] + offset + len(expected)]
    refs = [(p - offset, symbols[i]['name'], typ) for p, i, typ in sec['relocations']
            if offset <= p < offset + len(expected)]
    return actual == expected and refs == [(0, VTABLE, 6)]


def extracted_descriptors(raw, read, fixups):
    """Prove address-named descriptor references against original PE fixups."""
    symbols, sections = parse(raw)
    owners = collections.defaultdict(dict)
    for symbol in symbols.values():
        match = re.fullmatch(r'\?[gs]_([0-9a-fA-F]+)@@\S+', symbol['name'])
        if match and symbol['storage'] == 2 and symbol['section'] > 0:
            owners[symbol['section']].setdefault(symbol['value'], set()).add(int(match[1], 16))
    entries = {si: sorted(values) for si, values in owners.items()}
    accepted = {}
    for si, section in sections.items():
        for offset, index, typ in section['relocations']:
            symbol = symbols[index]
            match = re.fullmatch(r'\?g_([0-9a-fA-F]+)@@3PAEA', symbol['name'])
            if not match or symbol['section'] != 0 or symbol['storage'] != 2:
                continue
            address = int(match[1], 16)
            try:
                name = descriptor(read, address)
            except ValueError:
                continue
            positions = entries.get(si, [])
            pos = bisect.bisect_right(positions, offset) - 1
            if typ != 6 or pos < 0 or offset + 4 > section['size']:
                raise ValueError('Unproven extracted descriptor relocation: ' + symbol['name'])
            start = positions[pos]
            original_owners = owners[si][start]
            if len(original_owners) != 1:
                raise ValueError('Ambiguous extracted referring object')
            original_site = next(iter(original_owners)) + offset - start
            addend = struct.unpack_from('<I', raw, section['offset'] + offset)[0]
            # These bindings describe exact descriptor entries, never interiors.
            if (addend != 0 or original_site not in fixups
                    or struct.unpack('<I', read(original_site, 4))[0] != address):
                raise ValueError('Extracted descriptor original pointer differs: ' + symbol['name'])
            decision = accepted.setdefault(symbol['name'], dict(address='%08x' % address,
                raw_type_name=name, canonical_symbol='??_R0' + name[1:] + '@8',
                evidence=['data/data.obj'], original_reference_sites=[]))
            decision['original_reference_sites'].append('%08x' % original_site)
    for decision in accepted.values():
        decision['original_reference_sites'].sort()
    return accepted


def discover():
    references = json.loads((build.OUT / 'external-references.json').read_text())
    addresses, evidence = collections.defaultdict(set), collections.defaultdict(set)
    for path in build.SRC.rglob('*'):
        if path.suffix not in ('.cpp', '.h'):
            continue
        for line in path.read_text(encoding='latin-1').splitlines():
            for name, address in annotated(line):
                addresses[name].add(address)
                evidence[name].add(path.relative_to(build.SRC).as_posix())
    read = original_reader()
    from exact_data_aliases import discover as discover_data
    reference_sites = discover_data(set())['reference_sites']
    accepted = {}
    for alias in sorted(references):
        match = re.fullmatch(r'\?(\w+)@@3(?:PADA|PBDB)', alias)
        if not match or len(addresses[match[1]]) != 1:
            continue
        address = next(iter(addresses[match[1]]))
        try:
            name = descriptor(read, int(address, 16))
        except ValueError:
            continue
        sites = reference_sites.get(alias, [])
        if any(site['address'] != address for site in sites):
            continue
        accepted[alias] = dict(address=address, raw_type_name=name,
            canonical_symbol='??_R0' + name[1:] + '@8',
            exact_reference_sites=sites,
            evidence=sorted(evidence[match[1]]))
    from exact_data_aliases import pe_fixups
    original = (build.REPO.parent / 'SqueezilsGameFiles/SQZ.EXE').read_bytes()
    extracted = extracted_descriptors((build.REPO / 'data/data.obj').read_bytes(),
                                     read, pe_fixups(original, read))
    for alias, decision in extracted.items():
        if alias not in references:
            raise ValueError('Extracted descriptor missing from current reference inventory')
        if alias in accepted:
            raise ValueError('Overlapping descriptor discovery families')
        accepted[alias] = decision
    return accepted


def main():
    registry = build.REPO / 'tools/rtti_alias_registry.json'
    current = discover()
    if '--register' in sys.argv:
        registry.write_text(json.dumps(current, indent=2) + '\n')
        print('Registered %d RTTI aliases, %d original descriptors' %
              (len(current), len({v['address'] for v in current.values()})))
        return
    decisions = json.loads(registry.read_text())
    if any(current.get(alias) != d for alias, d in decisions.items()):
        raise ValueError('RTTI address/name/declaration evidence changed')
    for family in ('data', 'entry', 'runtime', 'qv'):
        aliases = json.loads((build.OUT / (family + '_alias.json')).read_text())['aliases']
        if set(decisions).intersection(a for a, _ in aliases):
            raise ValueError('RTTI metadata aliases have another owner: ' + family)
    libc = next((pathlib.Path(p) / 'libc.lib' for p in os.environ['LIB'].split(';')
                 if (pathlib.Path(p) / 'libc.lib').is_file()), None)
    if libc is None:
        raise ValueError('No current CRT library')
    inventory = subprocess.run(['dumpbin', '-linkermember:1', str(libc)],
                              capture_output=True, check=True).stdout.decode('latin-1')
    if VTABLE not in {line.split()[-1] for line in inventory.splitlines() if line.split()}:
        raise ValueError('CRT does not own the type_info vtable')
    units = json.loads((build.OUT / 'units.json').read_text())
    definitions = collections.defaultdict(list)
    wanted = {d['canonical_symbol']: d['raw_type_name'] for d in decisions.values()}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compile inventory')
        raw = pathlib.Path(unit['object']).read_bytes()
        symbols, sections = parse(raw)
        for symbol in symbols.values():
            if symbol['name'] in wanted and symbol['storage'] == 2 and symbol['section'] > 0:
                if not existing_owner(raw, symbol, symbols, sections, wanted[symbol['name']]):
                    raise ValueError('Compiler RTTI metadata differs: ' + symbol['name'])
                definitions[symbol['name']].append(unit['source'])
    generated, pairs, report = {}, [], []
    for alias, d in sorted(decisions.items()):
        canonical = d['canonical_symbol']
        if definitions.get(canonical):
            target = canonical
        else:
            target = '_SqzRtti_' + d['address'].upper()
            generated[target] = d['raw_type_name']
        pairs.append((alias, target))
        report.append(dict(alias=alias, target=target, decision=d,
                           owners=definitions.get(canonical, ['generated original TypeDescriptor metadata'])))
    emit(generated.items(), build.OUT / 'rtti_data.obj')
    write_coff(pairs, build.OUT / 'rtti_alias.obj')
    (build.OUT / 'rtti_alias.json').write_text(json.dumps(dict(aliases=pairs, evidence=report,
        generated_descriptors=generated, registry_sha256=hashlib.sha256(registry.read_bytes()).hexdigest(),
        libc_sha256=hashlib.sha256(libc.read_bytes()).hexdigest(),
        original_sha256=hashlib.sha256((build.REPO.parent / 'SqueezilsGameFiles/SQZ.EXE').read_bytes()).hexdigest()), indent=2))
    print('%d RTTI aliases; %d generated descriptors' % (len(pairs), len(generated)))


if __name__ == '__main__':
    main()
