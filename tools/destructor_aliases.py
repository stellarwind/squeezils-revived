"""Resolve annotated complete-destructor spellings to exact addressed owners."""
import hashlib
import json
import pathlib
import re

import integration as build
from code_pointer_bindings import original_reader
from exact_call_aliases import exact_body
from exact_data_aliases import pe_fixups
from global_getter_aliases import source_files
from math_lifetime_aliases import annotations
from project_eh import parse

PATTERN = re.compile(r'^\?\?1(\w+)@@[QU]AE@XZ$')


def object_digest(raw):
    # VC4 stamps every compilation. All remaining bytes stay proof inputs.
    return hashlib.sha256(raw[:4] + bytes(4) + raw[8:]).hexdigest()


def relocatable(section, address, fixups):
    absolute = {offset for offset, _, kind in section['relocations'] if kind == 6}
    return all(site - address in absolute for site in fixups
               if address <= site < address + section['size'])


def choose(addresses, sites, owners):
    if len(addresses) != 1:
        return None
    address = next(iter(addresses))
    if any(int(s['address'], 16) != address for s in sites):
        return None
    candidates = {(e['symbol'], e['source']) for e in owners
                  if PATTERN.fullmatch(e['symbol'])}
    return next(iter(candidates)) if len(candidates) == 1 else None


def discover():
    units = json.loads((build.OUT / 'units.json').read_text())
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    exact = json.loads((build.OUT / 'exact-call-candidates.json').read_text())
    existing = set()
    for registry in ('ordinary_alias_registry', 'exact_call_registry', 'math_lifetime_registry'):
        existing.update(json.loads((build.REPO / ('tools/' + registry + '.json')).read_text()))
    objects, references = {}, {}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compiled inventory')
        raw = pathlib.Path(unit['object']).read_bytes()
        symbols, sections = parse(raw)
        objects[unit['source']] = raw, symbols, sections
        for symbol in symbols.values():
            alias = symbol['name']
            if (symbol['section'] == 0 and symbol['storage'] == 2
                    and PATTERN.fullmatch(alias) and alias not in existing
                    and alias not in ownership['definitions']):
                references.setdefault(alias, set()).add(unit['source'])
    read = original_reader()
    fixups = pe_fixups((build.REPO.parent / 'SqueezilsGameFiles/SQZ.EXE').read_bytes(), read)
    accepted, rejected = {}, {}
    for alias, sources in sorted(references.items()):
        declarations, addresses, hashes = {}, set(), {}
        typename = PATTERN.fullmatch(alias)[1]
        for source in sorted(sources):
            evidence = {}
            for path in source_files(source):
                found = annotations(path.read_text(errors='replace'), typename)
                if found:
                    relative = path.relative_to(build.SRC.resolve()).as_posix()
                    evidence[relative] = ['%08x' % a for a in found]
                    hashes[relative] = hashlib.sha256(path.read_bytes()).hexdigest()
                    addresses.update(found)
            if not evidence:
                rejected[alias] = 'Missing explicit destructor annotation: ' + source
                break
            declarations[source] = evidence
        if alias in rejected:
            continue
        address = next(iter(addresses)) if len(addresses) == 1 else None
        if address is not None and address >= 0x47e600:
            rejected[alias] = 'Library lifetime ownership belongs to the reviewed Qv registries'
            continue
        key = '%08x' % address if address is not None else ''
        owner = choose(addresses, exact['call_sites'].get(alias, []), ownership['addresses'].get(key, []))
        if owner is None:
            rejected[alias] = 'Conflicting address/call evidence or no unique complete-destructor owner'
            continue
        target, source = owner
        if ownership['symbol_addresses'].get(target) != [key]:
            rejected[alias] = 'Canonical destructor also targets other addresses'
            continue
        valid = True
        owner_hashes = {}
        for definition in ownership['definitions'][target]:
            raw, symbols, sections = objects[definition]
            defs = [s for s in symbols.values() if s['name'] == target and s['section'] > 0]
            if (len(defs) != 1 or not exact_body(raw, symbols, sections, defs[0], address, read)
                    or not relocatable(sections[defs[0]['section']], address, fixups)):
                valid = False
                break
            owner_hashes[definition] = object_digest(raw)
        if not valid:
            rejected[alias] = 'Canonical destructor body is not completely original'
            continue
        accepted[alias] = dict(address=key, target=target, owner=source,
            declarations=declarations, declaration_sha256=hashes, owner_object_without_timestamp_sha256=owner_hashes,
            abi='Zero-argument complete thiscall destructor, void; no deleting flag or this adjustment.',
            reason='Every referring source annotates one original entry; available exact calls agree; all selectable canonical definitions match the complete original body.')
    return dict(accepted=accepted, rejected=rejected)


if __name__ == '__main__':
    import sys
    result = discover()
    (build.OUT / 'destructor-candidates.json').write_text(json.dumps(result, indent=2))
    print('%d complete-destructor aliases; %d withheld' % (len(result['accepted']), len(result['rejected'])))
    if '--register' in sys.argv:
        (build.REPO / 'tools/destructor_registry.json').write_text(json.dumps(result['accepted'], indent=2) + '\n')
