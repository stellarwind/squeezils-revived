"""Bind annotated spellings of the two original empty math destructors.

Only complete nonvirtual thiscall destructors qualify. Every referring source
must provide an explicit destructor-address annotation, directly or in a local
header. Never infer a no-op from the type name or from an unrelated RET body.
"""
import json
import pathlib
import re

import integration as build
from code_pointer_bindings import original_reader
from exact_call_aliases import EMPTY_OWNERS, EMPTY_SOURCES, exact_body
from global_getter_aliases import source_files
from project_eh import parse

PATTERN = re.compile(r'^\?\?1(\w+)@@QAE@XZ$')


def annotations(text, typename):
    """Return all explicitly annotated declarations, including contradictions."""
    dtor = re.compile(r'(?<![\w~])~' + re.escape(typename) + r'\s*\(\s*\)\s*;')
    header = re.compile(r'\b(?:struct|class)\s+' + re.escape(typename) + r'\s*\{')
    result = []
    for line in text.splitlines():
        code, separator, comment = line.partition('//')
        if not separator:
            continue
        if dtor.search(code):
            # On compact declarations, the comment must either specify dtor
            # explicitly or contain just one address after the declaration.
            tagged = re.findall(r'\bdtor\s+(0x[0-9a-fA-F]{6,8})\b', comment)
            addresses = tagged or re.findall(r'\b0x[0-9a-fA-F]{6,8}\b', comment)
            result.extend(int(a, 16) for a in addresses)
        elif header.search(code) and dtor.search(text):
            result.extend(int(a, 16) for a in re.findall(
                r'\bdtor\s+(0x[0-9a-fA-F]{6,8})\b', comment))
    return result


def discover():
    units = json.loads((build.OUT / 'units.json').read_text())
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    exact = json.loads((build.OUT / 'exact-call-candidates.json').read_text())
    existing = {}
    for name in ('ordinary_alias_registry.json', 'exact_call_registry.json'):
        existing.update(json.loads((build.REPO / 'tools' / name).read_text()))
    references, objects = {}, {}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compiled inventory')
        objects[unit['source']] = pathlib.Path(unit['object'])
        for symbol, kind in build.coff_symbols(objects[unit['source']]):
            if (kind == 'undefined' and PATTERN.fullmatch(symbol)
                    and symbol not in ownership['definitions'] and symbol not in existing):
                references.setdefault(symbol, set()).add(unit['source'])
    read = original_reader()
    accepted, rejected = {}, {}
    for alias, sources in sorted(references.items()):
        typename = PATTERN.fullmatch(alias)[1]
        declarations, addresses = {}, set()
        for source in sorted(sources):
            evidence = {}
            for path in source_files(source):
                found = annotations(path.read_text(errors='replace'), typename)
                if found:
                    evidence[path.relative_to(build.SRC.resolve()).as_posix()] = ['%08x' % a for a in found]
                    addresses.update(found)
            if not evidence:
                rejected[alias] = 'No annotated destructor declaration: ' + source
                break
            declarations[source] = evidence
        if alias in rejected:
            continue
        if len(addresses) != 1 or not addresses.issubset(EMPTY_OWNERS):
            rejected[alias] = 'Not one of the two reviewed original empty math destructors'
            continue
        address = next(iter(addresses))
        if any(int(s['address'], 16) != address for s in exact['call_sites'].get(alias, [])):
            rejected[alias] = 'Original call contradicts declaration address'
            continue
        target, owner = EMPTY_OWNERS[address], EMPTY_SOURCES[address]
        if (ownership['symbol_addresses'].get(target) != ['%08x' % address]
                or owner not in ownership['definitions'].get(target, [])):
            raise ValueError('Canonical empty destructor ownership changed: ' + alias)
        # Every selectable definition must be exactly RET, with no fixups and
        # no other entry. A matching opcode alone never establishes identity.
        for source in ownership['definitions'][target]:
            raw = objects[source].read_bytes()
            symbols, sections = parse(raw)
            defs = [s for s in symbols.values() if s['name'] == target and s['storage'] == 2 and s['section'] > 0]
            if len(defs) != 1 or not exact_body(raw, symbols, sections, defs[0], address, read):
                raise ValueError('Canonical empty destructor body changed: ' + alias)
            section = sections[defs[0]['section']]
            if section['size'] != 1 or section['relocations'] or raw[section['offset']] != 0xc3:
                raise ValueError('Canonical destructor is no longer relocation-free RET: ' + alias)
        accepted[alias] = dict(address='%08x' % address, target=target, owner=owner,
            referring_sources=sorted(sources), declarations=declarations,
            abi='Complete nonvirtual thiscall destructor, zero arguments, void; canonical original body is RET and does not read or adjust this.',
            reason='Every referring unit explicitly annotates this destructor at the same reviewed math entry; available exact calls agree; all selectable canonical definitions are the original relocation-free RET.')
    return dict(accepted=accepted, rejected=rejected)


if __name__ == '__main__':
    import sys
    report = discover()
    (build.OUT / 'math-lifetime-candidates.json').write_text(json.dumps(report, indent=2))
    print('%d math lifetime aliases; %d withheld' % (len(report['accepted']), len(report['rejected'])))
    if '--register' in sys.argv:
        (build.REPO / 'tools/math_lifetime_registry.json').write_text(json.dumps(report['accepted'], indent=2) + '\n')
