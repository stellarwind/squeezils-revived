"""Reviewed affine-matrix methods, with explicit source layout and ABI proof.

Includes only exact canonical setters, rotations and orthonormalization. Void
callers may discard the canonical this pointer in EAX; no hidden result or
argument adaptation is permitted. Matrix products/copy routines are separate.
"""
import hashlib
import json
import pathlib
import re

import integration as build
from code_pointer_bindings import original_reader
from exact_call_aliases import exact_body
from global_getter_aliases import source_files
from project_eh import parse


def classify(symbol):
    for axis, address in [('X', 0x47dcd0), ('Y', 0x47dd20), ('Z', 0x47dd70)]:
        match = re.fullmatch(r'\?(Turn' + axis + r'|Rotation' + axis
            + r')@(\w+)@@QAE(?:AAU1@|X)M@Z', symbol)
        if match:
            return dict(method=match[1], typename=match[2], vector=None, address=address,
                target='?Turn%s@Matrix4@@QAEAAU1@M@Z' % axis, owner='Matrix4/Turn.cpp')
    match = re.fullmatch(r'\?SetTranslation@(\w+)@@QAE(?:AAU1@|X)(?:ABU(\w+)@@|PBM)@Z', symbol)
    if match:
        return dict(method='SetTranslation', typename=match[1], vector=match[2], address=0x47e330,
            target='?SetTranslation@Matrix4@@QAEAAU1@PBM@Z', owner='Matrix4/SetTranslation.cpp')
    match = re.fullmatch(r'\?SetRows@(\w+)@@QAE(?:AAU1@|X)ABU(\w+)@@000@Z', symbol)
    if match:
        return dict(method='SetRows', typename=match[1], vector=match[2], address=0x409f00,
            target='?SetRows@Matrix4@@QAEAAU1@ABUVec3@@000@Z', owner='Matrix4/SetRows.cpp')
    match = re.fullmatch(r'\?Orthonormalize@(\w+)@@QAEXXZ', symbol)
    if match:
        return dict(method='Orthonormalize', typename=match[1], vector=None, address=0x47e350,
            target='?Orthonormalize@Frame@@QAEPAU1@XZ', owner='Fn47e350/F.cpp')
    return None


def raw_blocks(text, typename):
    """Unbased structs only; brace counting ignores single-line comments."""
    clean = re.sub(r'//[^\n]*', lambda m: ' ' * len(m[0]), text)
    result = []
    for match in re.finditer(r'\bstruct\s+' + re.escape(typename) + r'\s*\{', clean):
        depth, end = 1, match.end()
        while end < len(clean) and depth:
            depth += (clean[end] == '{') - (clean[end] == '}')
            end += 1
        if depth:
            continue
        line_end = text.find('\n', end)
        if line_end < 0:
            line_end = len(text)
        if re.fullmatch(r'\s*;\s*(?://[^\n]*)?', text[end:line_end]):
            end = line_end
        result.append(text[match.start():end])
    return result


def fields(block, typename):
    code = re.sub(r'//[^\n]*', '', block)
    if any(s in code for s in ('virtual', '/*', '"', "'")):
        return None
    inner = code[code.index('{') + 1:code.rindex('}')]
    flat, depth = '', 0
    for char in inner:
        if char == '{':
            depth += 1
        elif char == '}':
            depth -= 1
            if depth == 0:
                flat += ';'
        elif depth == 0:
            flat += char
    flat = re.sub(r'^\s*#.*$', '', flat, flags=re.M)
    statements = [re.sub(r'^(?:public|private|protected)\s*:\s*', '', s.strip())
                  for s in flat.split(';') if s.strip()]
    method = re.compile(r'^(?:~?' + re.escape(typename)
        + r'|(?:const\s+)?\w+(?:\s*[&*]\s*|\s+)(?:operator\s*[-+*/=]|\w+))\s*\(')
    if any('(' in s and not method.match(s) for s in statements):
        return None
    return [re.sub(r'\s+', ' ', s) for s in statements if '(' not in s]


def layout(text, typename, vector=False, seen=()):
    if typename in seen:
        return []
    blocks = raw_blocks(text, typename)
    if not blocks:
        return []
    evidence = []
    for block in blocks:
        members = fields(block, typename)
        if members is None:
            return []
        if vector:
            if [re.sub(r'\s+', '', s) for s in members] not in (
                    ['floatx,y,z'], ['floatx', 'floaty', 'floatz']):
                return []
        elif len(members) != 1:
            return []
        elif not re.fullmatch(r'float \w+\s*\[\s*16\s*\]', members[0]):
            # A sole nonvirtual matrix aggregate at offset zero is equivalent
            # to its contained 16-float matrix; prove the contained type too.
            nested = re.fullmatch(r'(\w+) \w+', members[0])
            nested_proof = layout(text, nested[1], seen=(*seen, typename)) if nested else []
            if not nested_proof:
                return []
            evidence.extend(nested_proof)
        evidence.append(block)
    return evidence


def annotated_addresses(block, method):
    # Join multiline method declarations with their terminating line comment.
    # Explicitly reject compact multi-method comments rather than guessing.
    pattern = re.compile(r'\b' + re.escape(method) + r'\s*\([^;{}]*\)\s*(?:const\s*)?;')
    result = []
    for match in pattern.finditer(block):
        start = block.rfind('\n', 0, match.start()) + 1
        end = block.find('\n', match.end())
        if end < 0:
            end = len(block)
        line = block[start:end]
        code, _, comment = line.partition('//')
        if len(re.findall(r'\)\s*(?:const\s*)?;', code)) != 1:
            continue
        result.extend(int(a, 16) for a in re.findall(r'\b0x[0-9a-fA-F]{6,8}\b', comment))
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
            if (kind == 'undefined' and classify(symbol) and symbol not in existing
                    and symbol not in ownership['definitions']):
                references.setdefault(symbol, set()).add(unit['source'])
    read = original_reader()
    accepted, rejected = {}, {}
    for alias, sources in sorted(references.items()):
        decision = classify(alias)
        typename, method, address = (decision[k] for k in ('typename', 'method', 'address'))
        evidence = {}
        for source in sorted(sources):
            files = source_files(source)
            text = '\n'.join(p.read_text(errors='replace') for p in files)
            matrix = layout(text, typename)
            vector = layout(text, decision['vector'], vector=True) if decision['vector'] else []
            addresses = [a for block in raw_blocks(text, typename)
                         for a in annotated_addresses(block, method)]
            if (not matrix or (decision['vector'] and not vector)
                    or not addresses or any(a != address for a in addresses)):
                rejected[alias] = 'Missing/conflicting method address or incompatible matrix/vector layout: ' + source
                break
            evidence[source] = dict(
                files=[p.relative_to(build.SRC.resolve()).as_posix() for p in files],
                layout_sha256=hashlib.sha256('\n'.join(matrix + vector).encode()).hexdigest(),
                addresses=['%08x' % a for a in addresses])
        if alias in rejected:
            continue
        if any(int(s['address'], 16) != address for s in exact['call_sites'].get(alias, [])):
            rejected[alias] = 'Original direct call contradicts declaration'
            continue
        target, owner = decision['target'], decision['owner']
        if (ownership['symbol_addresses'].get(target) != ['%08x' % address]
                or ownership['definitions'].get(target) != [owner]):
            raise ValueError('Canonical matrix owner changed: ' + alias)
        raw = objects[owner].read_bytes()
        symbols, sections = parse(raw)
        defs = [s for s in symbols.values() if s['name'] == target and s['storage'] == 2 and s['section'] > 0]
        if len(defs) != 1 or not exact_body(raw, symbols, sections, defs[0], address, read):
            raise ValueError('Canonical matrix method no longer exact: ' + alias)
        accepted[alias] = dict(**decision, declarations=evidence,
            abi='thiscall; 64-byte plain matrix at this+0, 32-bit float or reference/pointer arguments; typed vector references address plain x/y/z floats; canonical this pointer returned in EAX or discarded by void caller; no hidden result or this adjustment.',
            reason='Explicit original method entry and compatible layouts in every referring unit; available exact calls agree; unique canonical method has complete exact original body outside relocations.')
    return dict(accepted=accepted, rejected=rejected)


if __name__ == '__main__':
    import sys
    report = discover()
    (build.OUT / 'matrix-candidates.json').write_text(json.dumps(report, indent=2))
    print('%d affine matrix aliases; %d withheld' % (len(report['accepted']), len(report['rejected'])))
    if '--register' in sys.argv:
        (build.REPO / 'tools/matrix_registry.json').write_text(json.dumps(report['accepted'], indent=2) + '\n')
