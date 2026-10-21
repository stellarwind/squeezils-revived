"""Reviewed arithmetic ABI family for nontrivial three-float vector spellings.

No arbitrary method/name matching: the operation table fixes both decorated ABI
and original entry. Every current referring unit must supply a plain x/y/z
float class, a destructor, and an explicitly address-annotated declaration.
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

# operation, alias suffix, declaration token, original entry, canonical symbol/source
OPERATIONS = [
    ('multiply', r'\?\?D(\w+)@@Q[AB]E\?AU0@M@Z', r'operator\s*\*', 0x401b60,
     '??DVector3f@@QBE?AU0@M@Z', 'Vector3f/Vector3fOps.cpp'),
    ('subtract', r'\?\?G(\w+)@@Q[AB]E\?AU0@ABU0@@Z', r'operator\s*-', 0x401ae0,
     '??GVector3f@@QBE?AU0@ABU0@@Z', 'Vector3f/Vector3fOps.cpp'),
    ('add', r'\?\?H(\w+)@@Q[AB]E\?AU0@ABU0@@Z', r'operator\s*\+', 0x403070,
     '??HVector3f@@QBE?AU0@ABU0@@Z', 'Vector3f/Vector3fPlus.cpp'),
    ('divide', r'\?\?K(\w+)@@Q[AB]E\?AU0@M@Z', r'operator\s*/', 0x420100,
     '??KDVec@@QBE?AU0@M@Z', 'Vector3f/Vector3fDiv.cpp'),
    ('cross', r'\?Cross@(\w+)@@Q[AB]E\?AU1@ABU1@@Z', r'\bCross', 0x41a350,
     '?Cross@Vector3f@@QBE?AU1@ABU1@@Z', 'Vector3f/Vector3fCross.cpp'),
    ('length', r'\?Length@(\w+)@@QBEMXZ', r'\bLength', 0x47d5f0,
     '?Length@Vector3f@@QBEMXZ', 'Vector3f/Vector3fLength.cpp'),
    ('lengthsq', r'\?LengthSq@(\w+)@@QBEMXZ', r'\bLengthSq', 0x47d610,
     '?LengthSq@Vector3f@@QBEMXZ', 'Vector3f/Vector3fLength.cpp'),
    ('normalize', r'\?Normalize@(\w+)@@QAEMXZ', r'\bNormalize', 0x47d630,
     '?Normalize@Vector3f@@QAEMXZ', 'Vector3f/Vector3fLength.cpp'),
]


def classify(symbol):
    for operation in OPERATIONS:
        match = re.fullmatch(operation[1], symbol)
        if match:
            return operation, match[1]
    return None


def class_blocks(text, typename):
    """Deliberately limited to unbased, ordinary float-vector structs."""
    result = []
    for match in re.finditer(r'\bstruct\s+' + re.escape(typename) + r'\s*\{', text):
        start, depth, end = match.start(), 1, match.end()
        # These small declarations have no strings or multiline comments.
        # Remove line comments for brace counting, retaining them in evidence.
        clean = re.sub(r'//[^\n]*', lambda m: ' ' * len(m[0]), text)
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
        block = text[start:end]
        code = re.sub(r'//[^\n]*', '', block)
        if any(s in code for s in ('virtual', '/*', '"', "'")):
            continue
        inner = code[code.index('{') + 1:code.rindex('}')]
        # Elide inline method bodies, then inspect every non-method statement.
        flat, nesting = '', 0
        for char in inner:
            if char == '{':
                nesting += 1
            elif char == '}':
                nesting -= 1
                if nesting == 0:
                    flat += ';'
            elif nesting == 0:
                flat += char
        flat = re.sub(r'^\s*#.*$', '', flat, flags=re.M)
        statements = [re.sub(r'^(?:public|private|protected)\s*:\s*', '', s.strip())
                      for s in flat.split(';') if s.strip()]
        # Do not mistake a function-pointer field or an initialized data member
        # for a method merely because its declaration contains parentheses.
        method = re.compile(r'^(?:~?' + re.escape(typename)
            + r'|(?:' + re.escape(typename) + r'(?:\s*&\s*|\s+)|float\s+|void\s+)'
            + r'(?:operator\s*[-+*/=]|Cross|LengthSq|Length|Normalize))\s*\(')
        if any('(' in s and not method.match(s) for s in statements):
            continue
        fields = [s for s in statements if '(' not in s]
        if fields not in (['float x, y, z'], ['float x,y,z'], ['float x', 'float y', 'float z']):
            continue
        if not re.search(r'~' + re.escape(typename) + r'\s*\(\s*\)', code):
            continue
        result.append(block)
    return result


def member_addresses(block, typename, token):
    result = []
    wanted = re.compile(token + r'\s*\([^;{}]*\)\s*(?:const\s*)?;')
    # Compact declarations use one address per declared out-of-line member in
    # declaration order; a named method tag can disambiguate a partial list.
    members = re.compile(r'(?:~?' + re.escape(typename)
                         + r'|operator\s*[-+*/=]|\b(?:Cross|LengthSq|Length|Normalize))'
                         + r'\s*\([^;{}]*\)\s*(?:const\s*)?;')
    for line in block.splitlines():
        code, _, comment = line.partition('//')
        declarations = list(members.finditer(code))
        wanted_here = [i for i, m in enumerate(declarations) if wanted.fullmatch(m[0])]
        if not wanted_here:
            continue
        tags = re.findall(token + r'\s+(0x[0-9a-fA-F]{6,8})\b', comment)
        if tags:
            result.extend(int(a, 16) for a in tags)
            continue
        addresses = re.findall(r'\b0x[0-9a-fA-F]{6,8}\b', comment)
        if len(addresses) == len(declarations):
            result.extend(int(addresses[i], 16) for i in wanted_here)
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
        operation, typename = classify(alias)
        label, _, token, address, target, owner = operation
        evidence = {}
        for source in sorted(sources):
            proof = []
            for path in source_files(source):
                for block in class_blocks(path.read_text(errors='replace'), typename):
                    addresses = member_addresses(block, typename, token)
                    if addresses and any(a != address for a in addresses):
                        rejected[alias] = 'Conflicting method declaration: ' + source
                    if addresses:
                        proof.append(dict(file=path.relative_to(build.SRC.resolve()).as_posix(),
                            class_sha256=hashlib.sha256(block.encode()).hexdigest(),
                            addresses=['%08x' % a for a in addresses]))
            if not proof:
                rejected[alias] = 'Missing annotated method/plain nontrivial vector layout: ' + source
            evidence[source] = proof
        if alias in rejected:
            continue
        if any(int(s['address'], 16) != address for s in exact['call_sites'].get(alias, [])):
            rejected[alias] = 'Original direct call contradicts declaration'
            continue
        if (ownership['symbol_addresses'].get(target) != ['%08x' % address]
                or ownership['definitions'].get(target) != [owner]):
            raise ValueError('Canonical vector owner changed: ' + alias)
        raw = objects[owner].read_bytes()
        symbols, sections = parse(raw)
        defs = [s for s in symbols.values() if s['name'] == target and s['storage'] == 2 and s['section'] > 0]
        if len(defs) != 1 or not exact_body(raw, symbols, sections, defs[0], address, read):
            raise ValueError('Canonical vector method no longer exact: ' + alias)
        accepted[alias] = dict(address='%08x' % address, target=target, owner=owner,
            operation=label, declarations=evidence,
            abi='thiscall; plain nontrivial 12-byte x/y/z float vector; matching decorated scalar/reference arguments and scalar or hidden vector result; const this qualification does not change calling convention; no this adjustment.',
            reason='Every referring unit has the reviewed vector layout and annotated method entry; available exact original direct calls agree; unique canonical method matches the complete original body outside relocations.')
    return dict(accepted=accepted, rejected=rejected)


if __name__ == '__main__':
    import sys
    report = discover()
    (build.OUT / 'vector-candidates.json').write_text(json.dumps(report, indent=2))
    print('%d vector arithmetic aliases; %d withheld' % (len(report['accepted']), len(report['rejected'])))
    if '--register' in sys.argv:
        (build.REPO / 'tools/vector_registry.json').write_text(json.dumps(report['accepted'], indent=2) + '\n')
