"""Reviewed surface interfaces; never infer ownership from a similar name alone."""
import hashlib
import json
import pathlib
import re
import integration as build
from code_pointer_bindings import original_reader
from destructor_aliases import object_digest, relocatable
from exact_call_aliases import exact_body
from exact_data_aliases import pe_fixups
from global_getter_aliases import source_files
from project_eh import parse

# Address -> reviewed compiled owner. ABI differences in local type spellings
# are intentional: wrappers forward caller-owned return buffers, not sizeof(Item).
OWNERS = {
    0x40c6b0: ('?GetPath@IniReader@@QBEPBDXZ', 'IniReader/IniReader.cpp'),
    0x40c6d0: ('?Forward0@Holder@@QAEXXZ', 'Misc3/Lists.cpp'),
    0x40c7b0: ('?WasNear@PreviousNearSurface@@QAEHABUPreviousNearVector@@@Z', 'NavSurface/WasNear.cpp'),
    0x40c860: ('?Compare@Cmp64@@QAEHXZ', 'Souls2/Handlers.cpp'),
    0x40f6f0: ('?GetC@Source@@QAE?AUItem@@H@Z', 'Misc14/Wrap.cpp'),
    0x40f850: ('?Get@Holder3@@QAE?AUPart@@H@Z', 'Misc18/Getter.cpp'),
    0x4105f0: ('?GetA@Source@@QAE?AUItem@@H@Z', 'Misc14/Wrap.cpp'),
    0x410660: ('?Get@Holder4@@QAE?AUPart@@H@Z', 'Misc18/Getter.cpp'),
    0x4128a0: ('?ToWorldBefore@InstancedSurface@@QAE?AUVector3f@@ABU2@@Z', 'NavMesh/ToWorldBefore.cpp'),
    0x4110b0: ('?GetB@Source@@QAE?AUItem@@H@Z', 'Misc14/Wrap.cpp'),
    0x40f610: ('?Find@Holder7@@QAEHPAH@Z', 'Fn40f610/F.cpp'),
    0x40f770: ('?Find@Holder8@@QAEHPAH@Z', 'Fn40f610/F.cpp'),
    0x40f8d0: ('?Find@Holder9@@QAEHPAH@Z', 'Fn40f610/F.cpp'),
    0x40f9b0: ('?Find@HolderS@@QAE?AVString@@PAH@Z', 'Fn40f610/F.cpp'),
    0x412bf0: ('?FaceOf@NavSurfaceList@@QAEPAUInstancedSurface@@PBUContact@@@Z', 'NavSurface/NavSurfaceList.cpp'),
}


def bindings():
    result = {}
    def add(cls, method, suffix, address, returned, argument=None, size=None):
        alias = '?' + method + '@' + cls + '@@' + suffix
        result[alias] = dict(typename=cls, method=method, address=address,
                             returned=returned, argument=argument, return_bytes=size)
    for cls, vector, matrix, data in (
            ('MoveSurface', 'MoveVector', 'MoveMatrix', 'MoveSurfaceData'),
            ('FlightSurface', 'FlightVector', 'FlightMatrix', 'FlightSurfaceData')):
        add(cls, 'GetSurface', 'QAEPAU' + data + '@@XZ', 0x40c6b0, data + ' *')
        add(cls, 'TypeFilter', 'QAEPBHXZ', 0x40c6d0, 'const int *')
        add(cls, 'Changed', 'QAEHXZ', 0x40c860, 'int')
        add(cls, 'WasNear', 'QAEHABU' + vector + '@@@Z', 0x40c7b0, 'int', vector)
        for method, address, typ, size in (
                ('ToLocal', 0x40f6f0, vector, 12), ('ToLocalBefore', 0x410660, vector, 12),
                ('ToWorld', 0x4105f0, matrix, 64), ('Relative', 0x4110b0, matrix, 64)):
            add(cls, method, 'QAE?AU' + typ + '@@ABU2@@Z', address, typ, typ, size)
        if cls == 'MoveSurface':
            for method, address in (('ToWorld', 0x40f850), ('ToWorldBefore', 0x4128a0)):
                add(cls, method, 'QAE?AU' + vector + '@@ABU2@@Z', address, vector, vector, 12)
    add('CarrySurface', 'Changed', 'QAEHXZ', 0x40c860, 'int')
    for method, address, typ, size in (
            ('MatrixToWorld', 0x4105f0, 'CarryMatrix', 64),
            ('ToLocalBefore', 0x410660, 'CarryVector', 12), ('ToWorld', 0x40f850, 'CarryVector', 12)):
        add('CarrySurface', method, 'QAE?AU' + typ + '@@ABU2@@Z', address, typ, typ, size)
    for cls, typ in (('InstancedSurface', 'Vector3f'), ('InstancedSurfaceCtor', 'InstanceCtorVector')):
        add(cls, 'ToWorld', 'QAE?AU' + typ + '@@ABU2@@Z', 0x40f850, typ, typ, 12)
    add('NearSurfaceInstance', 'GetSurface', 'QAEPAUNearSurfaceData@@XZ', 0x40c6b0, 'NearSurfaceData *')
    for cls, typ in (('PlacementFace', 'PlacementMatrix'), ('SurfaceFace', 'Matrix16')):
        add(cls, 'Relative', 'QAE?AU' + typ + '@@ABU2@@Z', 0x4110b0, typ, typ, 64)
    def query(cls, method, returned, contact, address, suffix, size=None, const=False):
        add(cls, method, suffix, address, returned, contact, size)
        result['?' + method + '@' + cls + '@@' + suffix].update(
            argument_mode='pointer', argument_const=const, family='navigation-query')
    query('NavSurfaces', 'Bounces', 'int', 'Contact', 0x40f610, 'QAEHPAUContact@@@Z')
    query('NavSurfaces', 'IsSlippery', 'int', 'Contact', 0x40f770, 'QAEHPBUContact@@@Z', const=True)
    for contact in ('Contact', 'MoveContact'):
        query('NavSurfaces', 'FlyThrough', 'int', contact, 0x40f8d0, 'QAEHPAU' + contact + '@@@Z')
        query('NavSurfaces', 'ActivateScript', 'String', contact, 0x40f9b0,
              'QAE?AVString@@PAU' + contact + '@@@Z', 4)
    query('FlightSurfaces', 'FlyThrough', 'int', 'FlightContact', 0x40f8d0, 'QAEHPAUFlightContact@@@Z')
    query('NavSurfaces', 'FaceOf', 'SurfaceFace *', 'Contact', 0x412bf0, 'QAEPAUSurfaceFace@@PAUContact@@@Z')
    query('PlacementSurfaces', 'FaceOf', 'PlacementFace *', 'Contact', 0x412bf0,
          'QAEPAUPlacementFace@@PBUContact@@@Z', const=True)
    return result


def declaration(text, spec):
    """Check the reviewed declaration inside its own class, including overload."""
    header = re.search(r'\b(?:struct|class)\s+' + re.escape(spec['typename']) + r'\s*\{', text)
    if not header:
        return False
    depth, end = 1, header.end()
    while end < len(text) and depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    body = text[header.end():end - 1]
    returned = re.escape(spec['returned']).replace(r'\ ', r'\s*')
    argument = (r'const\s+' + re.escape(spec['argument']) + r'\s*&\s*\w*'
                if spec['argument'] else '')
    if spec.get('argument_mode') == 'pointer':
        argument = (r'const\s+' if spec['argument_const'] else '') + re.escape(spec['argument']) + r'\s*\*\s*\w*'
    elif spec.get('argument_mode') == 'value':
        argument = re.escape(spec['argument']) + r'\s+\w+'
    pattern = (r'\b' + returned + r'\s*' + re.escape(spec['method']) +
               r'\s*\(\s*' + argument + r'\s*\)\s*;\s*//[^\n]*\b0x([0-9a-fA-F]{6,8})\b')
    found = [int(m[1], 16) for m in re.finditer(pattern, body)]
    return found == [spec['address']]


def discover(policy=None, owners=None):
    policy = bindings() if policy is None else policy
    owners = OWNERS if owners is None else owners
    units = json.loads((build.OUT / 'units.json').read_text())
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    exact = json.loads((build.OUT / 'exact-call-candidates.json').read_text())
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
    accepted, rejected = {}, {}
    for alias, spec in sorted(policy.items()):
        if alias not in refs or alias in ownership['definitions']:
            rejected[alias] = 'No exclusively undefined current reference'
            continue
        address = spec['address']; key = '%08x' % address
        target, owner = owners[address]
        evidence = {}
        for source in sorted(refs[alias]):
            files = [p for p in source_files(source) if declaration(p.read_text(errors='replace'), spec)]
            if not files:
                rejected[alias] = 'Reviewed class/signature/address declaration missing: ' + source
                break
            for path in files:
                evidence[path.relative_to(build.SRC.resolve()).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
        if alias in rejected:
            continue
        if any(s['address'] != key for s in exact['call_sites'].get(alias, [])):
            raise ValueError('Exact caller contradicts surface binding: ' + alias)
        if not any(e['symbol'] == target and e['source'] == owner for e in ownership['addresses'].get(key, [])):
            raise ValueError('Addressed surface owner changed: ' + alias)
        hashes = {}
        for source in ownership['definitions'][target]:
            raw, symbols, sections = objects[source]
            defs = [s for s in symbols.values() if s['name'] == target and s['section'] > 0]
            if (len(defs) != 1 or not exact_body(raw, symbols, sections, defs[0], address, read)
                    or not relocatable(sections[defs[0]['section']], address, fixups)):
                raise ValueError('Surface owner is not complete original relocatable code: ' + alias)
            if ownership['symbol_addresses'][target] != [key] and sections[defs[0]['section']]['relocations']:
                raise ValueError('Multi-address owner has relocations: ' + alias)
            hashes[source] = object_digest(raw)
        abi = ('ECX=this, hidden return buffer plus const-reference argument, RET 8, EAX=return buffer'
               if spec['return_bytes'] else 'ECX=this, const-reference argument, RET 4, int EAX'
               if spec['argument'] else 'ECX=this, no stack arguments, scalar/pointer EAX, RET or tail jump')
        layout = ('surface pointer +0; current matrix/identity +4/+44; previous +48/+88; dirty/result +8c/+90; '
                  'type filter virtual tail call via member +c8 slot +10; positions +dc/+e8; vector 12/matrix 64 bytes.')
        if spec.get('family') == 'navigation-query':
            abi = ('ECX=this; hidden caller-owned String buffer plus contact pointer; RET 8; EAX=buffer'
                   if spec['return_bytes'] else 'ECX=this; one contact pointer; RET 4; scalar/pointer EAX')
            layout = ('Contact face/second +0/+4, matrix +8; instance Array +134, registry +16c; '
                      'Bounces/IsSlippery/FlyThrough entry values +4/+8/+c; String is one four-byte rep pointer; '
                      'FaceOf reads contact face at +0 and dispatches instance Get. No this adjustment.')
        if spec.get('family') == 'application':
            abi, layout = spec['abi'], spec['layout']
        accepted[alias] = dict(address=key, target=target, owner=owner, declaration_sha256=evidence,
            referring_sources=sorted(refs[alias]), owner_object_without_timestamp_sha256=hashes,
            signature=spec, abi=abi + '; no this adjustment',
            layout=layout,
            reason='Reviewed declaration, address and ABI; every selectable owner matches the complete original body. '
                'Forwarding wrappers pass the return buffer unchanged; local Item/Part spellings do not allocate or copy it.')
    return dict(accepted=accepted, rejected=rejected)


if __name__ == '__main__':
    result = discover()
    (build.OUT / 'surface-candidates.json').write_text(json.dumps(result, indent=2))
    print('Surface interfaces:', len(result['accepted']), 'accepted;', len(result['rejected']), 'withheld')
