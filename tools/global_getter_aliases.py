"""Reviewed six-getter family; no new code or storage is emitted.

Discover from current loose-object references. Require a matching address-annotated
declaration in every referring translation unit (including its local headers),
the exact original getter instruction, and the canonical extracted global owner.
"""
import json
import pathlib
import re

import integration as build
from code_pointer_bindings import original_reader
from exact_call_aliases import POINTER_GETTERS, exact_body
from project_eh import parse

NAMES = dict(GetLand=0x420680, GetWorld=0x4206a0,
             GetSimulation=0x420690, GetCurrentSimulation=0x420690,
             GetApplication=0x4206b0, GetPanel=0x4206c0,
             GetClock=0x4206d0, GetGameClock=0x4206d0)
PATTERN = re.compile(r'^\?(\w+)@@YAPA(?:X|[UV](\w+)@@)XZ$')


def declarations(text, name, pointee):
    result = []
    pattern = re.compile(r'^\s*(?:extern\s+)?' + re.escape(pointee or 'void')
                         + r'\s*\*\s*' + re.escape(name)
                         + r'\s*\(\s*(?:void)?\s*\)\s*;\s*//\s*0x([0-9a-fA-F]{6,8})\b')
    for line in text.splitlines():
        match = pattern.match(line)
        if match:
            result.append(int(match[1], 16))
    return result


def source_files(source):
    pending, found = [build.SRC / source], set()
    while pending:
        path = pending.pop().resolve()
        if path in found or not path.is_file() or not path.is_relative_to(build.SRC.resolve()):
            continue
        found.add(path)
        text = path.read_text(errors='replace')
        for header in re.findall(r'^\s*#include\s*"([^"]+)"', text, re.M):
            local = path.parent / header
            pending.append(local if local.is_file() else build.SRC / header)
    return sorted(found)


def discover():
    units = json.loads((build.OUT / 'units.json').read_text())
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    existing = json.loads((build.REPO / 'tools/ordinary_alias_registry.json').read_text())
    existing.update(json.loads((build.REPO / 'tools/exact_call_registry.json').read_text()))
    data = dict(json.loads((build.OUT / 'data_alias.json').read_text())['aliases'])
    exact = json.loads((build.OUT / 'exact-call-candidates.json').read_text())
    references = {}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compiled inventory')
        for symbol, kind in build.coff_symbols(pathlib.Path(unit['object'])):
            match = PATTERN.fullmatch(symbol)
            if (kind == 'undefined' and match and match[1] in NAMES
                    and symbol not in ownership['definitions'] and symbol not in existing):
                references.setdefault(symbol, set()).add(unit['source'])
    read = original_reader()
    accepted, rejected = {}, {}
    canonical = next(u for u in units if u['source'] == 'Globals/Globals.cpp')
    raw = pathlib.Path(canonical['object']).read_bytes()
    symbols, sections = parse(raw)
    for alias, sources in sorted(references.items()):
        match = PATTERN.fullmatch(alias)
        name, pointee = match.groups()
        address = NAMES[name]
        target, global_name, slot = POINTER_GETTERS[address]
        evidence = []
        for source in sorted(sources):
            sites = [(str(p.relative_to(build.SRC.resolve())).replace('\\', '/'),
                      declarations(p.read_text(errors='replace'), name, pointee))
                     for p in source_files(source)]
            sites = [(p, addresses) for p, addresses in sites if addresses]
            if not sites or any(a != address for _, addresses in sites for a in addresses):
                rejected[alias] = 'Missing or conflicting declaration: ' + source
                break
            evidence.extend(p for p, _ in sites)
        if alias in rejected:
            continue
        if any(int(s['address'], 16) != address for s in exact['call_sites'].get(alias, [])):
            raise ValueError('Original direct call contradicts getter declaration: ' + alias)
        defs = [s for s in symbols.values() if s['name'] == target and s['section'] > 0 and s['storage'] == 2]
        if (ownership['symbol_addresses'].get(target) != ['%08x' % address]
                or ownership['definitions'].get(target) != ['Globals/Globals.cpp']
                or len(defs) != 1 or not exact_body(raw, symbols, sections, defs[0], address, read)):
            raise ValueError('Canonical getter owner/body changed: ' + alias)
        if not re.fullmatch(r'\?[gs]_%X@@\S+' % slot, data.get(global_name, '')):
            raise ValueError('Canonical getter storage changed: ' + alias)
        accepted[alias] = dict(address='%08x' % address, target=target,
            owner='Globals/Globals.cpp', referring_sources=sorted(sources),
            evidence=sorted(set(evidence)), global_address='%08x' % slot,
            abi='cdecl, zero arguments, 32-bit object pointer return in EAX; no adjustment; canonical body is original mov eax,[slot]; ret',
            reason='Every referring unit has an address-annotated matching pointer declaration; available exact direct-call destinations agree; canonical code and extracted storage verified.')
    return dict(accepted=accepted, rejected=rejected)


if __name__ == '__main__':
    import sys
    report = discover()
    (build.OUT / 'global-getter-candidates.json').write_text(json.dumps(report, indent=2))
    print('%d global getter aliases; %d withheld' % (len(report['accepted']), len(report['rejected'])))
    if '--register' in sys.argv:
        (build.REPO / 'tools/global_getter_registry.json').write_text(json.dumps(report['accepted'], indent=2) + '\n')
