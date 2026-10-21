"""Reviewed Qv string/name/intern bindings to the canonical parser library.

Registry captures current declaration files and loose owners, not linker error
history. Functional upstream entries are allowed: score is diagnostic metadata.
Name-table globals use QvLib storage, with original initializer operand evidence.
"""
import csv
import hashlib
import json
import pathlib
import subprocess
import os

import integration as build
from link_containers import verify_qv_library
from project_eh import parse


def specifications():
    with (build.REPO / 'qvlib/score_table.tsv').open() as stream:
        rows = list(csv.DictReader(stream, delimiter='\t'))
    by_address = {int(r['address'], 16): r['symbol'] for r in rows}
    result = {}
    def add(alias, address, abi):
        result[alias] = dict(address='%08x' % address,
            target='__strdup' if address == 0x49bd70 else by_address[address], abi=abi)
    for alias in ['?QvIntern@@YAPAUQvEntry@@PBD@Z', '?QvIntern@@YAPAXPBD@Z',
                  '?QvInternE@@YAPAXPBD@Z', '?QvInternS@@YAPAUQvEntryS@@PBD@Z', '?QvInternY@@YAPAXPBD@Z']:
        add(alias, 0x482390, 'cdecl; const char pointer; intern-entry pointer result; entry string/hash/next at +0/+4/+8')
    add('?QvInitNames@@YAXXZ', 0x482340, 'cdecl; no arguments; void; initializes the canonical 1999-bucket name table')
    add('?QvHash@@YAIPBD@Z', 0x4827e0, 'cdecl/static; const char pointer; unsigned 32-bit hash in EAX')
    add('?QvStrdup@@YAPADPBD@Z', 0x49bd70, 'cdecl; const char pointer; malloc-owned duplicated char pointer; LIBC _strdup')
    add('_strdup', 0x49bd70, 'cdecl; upstream POSIX spelling strdup to VC4 _strdup; both one pointer argument and malloc-owned pointer result')
    for cls in ('QvStrS', 'QvStrX', 'QvStrC', 'QvStrLocal', 'QvTmpStr'):
        add('??1%s@@QAE@XZ' % cls, 0x482730, 'nonvirtual thiscall destructor; data +0, inline buffer +8; no arguments')
    for cls in ('QvStrS', 'QvStrX', 'QvStrA'):
        add('?Reset@%s@@QAEXH@Z' % cls, 0x482810, 'thiscall; int freeOld; void; 40-byte string with data/capacity/buffer at +0/+4/+8')
    for cls in ('QvStrX', 'QvStrY'):
        add('?Ensure@%s@@QAEXH@Z' % cls, 0x482750, 'thiscall; int additional bytes; void; 40-byte string with 32-byte inline buffer')
    add('?Assign@QvStrX@@QAEAAU1@PBD@Z', 0x482840, 'thiscall; const char pointer; this reference returned in EAX; 40-byte string')
    add('?Assign@QvInfo482840@@QAEXPBD@Z', 0x482840, 'thiscall; const char pointer; discard EAX; non-owning view into a 40-byte Qv field string at node+0x24')
    add('?Append@QvStrX@@QAEAAU1@PBD@Z', 0x482960, 'thiscall; const char pointer; this reference in EAX; 40-byte string')
    add('?Append@QvTmpStr@@QAEXPBD@Z', 0x482960, 'thiscall; const char pointer; discard EAX; 40-byte string')
    add('?QvStrEq@@YAHPAUQvStrX@@PBD@Z', 0x4829c0, 'cdecl; string reference/pointer then const char pointer; int result; reads string data +0')
    add('?QvStrEq@@YAHPAUQvStrC@@PAD@Z', 0x4829c0, 'cdecl; string pointer then char pointer; int result; const qualification changes no argument width')
    for name, address in [('QvIsNameStartS', 0x482620), ('QvIsNameCharS', 0x482670)]:
        add('?%s@@YAHD@Z' % name, address, 'cdecl/static; signed char argument; int result; node-name classification')
    for alias, target, address in [
        ('?g_nameBuckets@@3HA', '?nameTableSize@QvNameEntry@@0HA', 0x4c8870),
        ('?g_nameTable@@3PAPAUQvEntry@@A', '?nameTable@QvNameEntry@@0PAPAV1@A', 0x4c8868),
        ('?g_arena@@3PAUQvArena@@A', '?chunk@QvNameEntry@@0PAUQvNameChunk@@A', 0x4c886c)]:
        result[alias] = dict(address='%08x' % address, target=target,
            abi='one canonical 32-bit zero-initialized pointer/int slot; table entry 12 bytes; arena 4012 bytes (4000-byte buffer, pointer, count, next)', data=True)
    return result


def discover():
    verify_qv_library()
    units = json.loads((build.OUT / 'units.json').read_text())
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    # Consult tracked ownership decisions, never a previous generated alias set.
    existing = json.loads((build.REPO / 'tools/qv_exact_registry.json').read_text())
    specs = specifications()
    sources = {n: set() for n in specs}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compiled inventory')
        for name, kind in build.coff_symbols(pathlib.Path(unit['object'])):
            if name in specs and kind in ('undefined', 'comdat', 'strong'):
                sources[name].add(unit['source'])
    if any(n == '_strdup' and k == 'undefined' for n, k in build.coff_symbols(build.REPO / 'qvlib/obj/QvLib.obj')):
        sources['_strdup'].update(['../qvlib/src/QvName.c++', '../qvlib/src/QvUnknownNode.c++'])
    raw = (build.REPO / 'qvlib/obj/QvLib.obj').read_bytes()
    symbols, sections = parse(raw)
    definitions = {s['name']: s for s in symbols.values() if s['storage'] == 2 and s['section'] > 0}
    original_sites = json.loads((build.OUT / 'exact-data-candidates.json').read_text())['reference_sites']
    init = definitions['?initClass@QvNameEntry@@CAXXZ']
    initialized = {symbols[i]['name'] for _, i, typ in sections[init['section']]['relocations'] if typ == 6}
    expected_state = {d['target'] for d in specs.values() if d.get('data')}
    if not expected_state.issubset(initialized):
        raise ValueError('Canonical initializer does not use all three shared name-state slots')
    # Confirm the runtime library exports the reviewed strdup entry too.
    libc = next(pathlib.Path(p) / 'libc.lib' for p in os.environ['LIB'].split(';')
                if (pathlib.Path(p) / 'libc.lib').is_file())
    members = subprocess.run(['dumpbin', '-linkermember:1', str(libc)], capture_output=True, check=True).stdout.decode(errors='replace')
    if '__strdup' not in {line.split()[-1] for line in members.splitlines() if line.split()}:
        raise ValueError('LIBC lacks __strdup')
    accepted = {}
    for alias, decision in sorted(specs.items()):
        if not sources[alias] or alias in existing:
            continue
        target = decision['target']
        if target != '__strdup' and target not in definitions:
            raise ValueError('Canonical parser owner is missing: ' + target)
        if decision.get('data'):
            sites = original_sites.get(alias, [])
            if not sites or any(int(s['address'], 16) != int(decision['address'], 16) for s in sites):
                raise ValueError('Original initializer operands disagree with state declaration: ' + alias)
            decision = dict(decision, original_state_sites=sites)
        owners = ownership['definitions'].get(alias, [])
        if owners and ownership['symbol_addresses'].get(alias) != [decision['address']]:
            raise ValueError('Loose implementation has conflicting original identity: ' + alias)
        accepted[alias] = dict(**decision, loose_owners=owners,
            source_sha256={s: hashlib.sha256((build.SRC / s).read_bytes()).hexdigest() for s in sorted(sources[alias])},
            owner='LIBC.LIB' if target == '__strdup' else 'qvlib/obj/QvLib.lib',
            reason='Reviewed original entry, argument/result ABI and object offsets; canonical library implementation owns code/state. No callable wrapper or copied globals.')
    return accepted


def reviewed():
    registry = build.REPO / 'tools/qv_name_registry.json'
    if not registry.exists():
        return {}
    saved = json.loads(registry.read_text())
    current = discover()
    for alias, decision in saved.items():
        if current.get(alias) != decision:
            raise ValueError('Qv string/name ownership evidence changed: ' + alias)
    return saved


if __name__ == '__main__':
    import sys
    decisions = discover()
    (build.OUT / 'qv-name-candidates.json').write_text(json.dumps(decisions, indent=2))
    print('%d reviewed Qv string/name/intern aliases' % len(decisions))
    if '--register' in sys.argv:
        (build.REPO / 'tools/qv_name_registry.json').write_text(json.dumps(decisions, indent=2) + '\n')
