"""Reviewed input/name-registry/dictionary ownership, independent of byte scores.

The finite ABI table is the review boundary. Candidate symbols come from current
objects, never prior link errors. Projected local definitions retain their exact
original-address identities; all declarations and canonical sources are hashed.
"""
import csv
import hashlib
import json
import pathlib
import re

import integration as build
from code_pointer_bindings import original_reader
from link_containers import verify_qv_library
from project_eh import parse

STATE_ALIAS = '?g_nameDict@@3PAUQvDictN@@A'
STATE_TARGET = '?nameDict@QvNode@@0PAVQvDict@@A'
OWNERS = {'Qv/' + p + '.cpp' for p in (
    'Input', 'Input3', 'Input4', 'Input5', 'Input6', 'Input7', 'InputGet',
    'Dict', 'Names', 'InitNameDict', 'Str2', 'Parse', 'Parse2', 'Parse3',
    'SetName', 'Factory', 'Create', 'Skip', 'Error', 'Version', 'Store')}

# Members use ECX and ordinary 32-bit stack arguments. References and pointers
# both pass addresses. Signed/unsigned long/int are all 32-bit here; the original
# entry fixes parsing semantics (GetUInt/GetLong names do not define semantics).
INPUT = {
    'Init': (0x4810c0, r'QAEXPAU_iobuf@@@Z'),
    'ReadChar': (0x4810f0, r'QAEHPAD@Z'), 'Read': (0x4810f0, r'QAEHPAD@Z'),
    'ReadNext': (0x4811a0, r'QAEHPAD@Z'),
    'ReadString': (0x4811d0, r'QAEHPAUQvStr\w*@@@Z'),
    'ReadName': (0x481300, r'QAEHPAUQvName\w*@@H@Z'),
    'GetInt': (0x481490, r'QAEHPAH@Z'), 'GetShort': (0x4814d0, r'QAEHPAF@Z'),
    'GetUInt': (0x481510, r'QAEHPAI@Z'), 'GetLong': (0x481550, r'QAEHPAJ@Z'),
    'GetFloat': (0x481590, r'QAEHPAM@Z'), 'AtEnd': (0x4815d0, r'QAEHXZ'),
    'LineString': (0x4815e0, r'QAEXPAUQvStr\w*@@@Z'),
    'PutBack': (0x481620, r'QAEXD@Z'), 'SetFileName': (0x481650, r'QAEXP[AB]D@Z'),
    'ReadHeader': (0x481670, r'QAEHXZ'), 'Next': (0x481740, r'QAEHXZ'),
    'ReadInt': (0x481820, r'QAEHPAH@Z'), 'ReadLong': (0x481820, r'QAEHPAJ@Z'),
    'ReadULong': (0x481890, r'QAEHPAK@Z'), 'ReadInteger': (0x4818d0, r'QAEHPAD@Z'),
    'ReadDouble': (0x481950, r'QAEHPAN@Z'), 'ReadDigits': (0x481a40, r'QAEHPAD@Z'),
    'ReadHexDigits': (0x481ad0, r'QAEHPAD@Z'), 'TryChar': (0x481b60, r'QAEHPADD@Z'),
    'Register': (0x481bc0, r'QAEXPAUQvName\w*@@PAUQvNode\w*@@@Z'),
    'Lookup': (0x481bf0, r'QAEPAUQvNode\w*@@PAUQvName\w*@@@Z'),
}
STATIC = {
    'QvInitNameDict': (0x47e5d0, r'YAXXZ'),
    'QvAddName': (0x47e9d0, r'YAXPAUQvNodeX@@I@Z'),
    'QvAddNameS': (0x47e9d0, r'YAXPAVQvNodeS@@PAD@Z'),
    'QvRemoveName': (0x47ea90, r'YAX(?:PAUQvNodeX@@I|PAXPAD)@Z'),
    'QvRemoveNameS': (0x47ea90, r'YAXPAVQvNodeS@@PAD@Z'),
    'QvReadNode': (0x47eb00, r'YAHPAVQvInput\w*@@PAPAUQvNode\w*@@@Z'),
    'QvReadUse': (0x47ed90, r'YA(?:H|PAUQvNodeQ@@)PAVQvInput\w*@@@Z'),
    'QvReadNodeBody': (0x47ee40, r'YAHPAVQvInput\w*@@PAUQvName\w*@@PAPAUQvNode\w*@@@Z'),
    'QvReadNodeBody2': (0x47ee40, r'YAHPAVQvInput\w*@@PAUQvName\w*@@PAPAUQvNode\w*@@@Z'),
    'QvCreateAndRead': (0x47f040, r'YAHPAVQvInputQ@@PAUQvNameQ@@1PAPAUQvNodeQ@@@Z'),
    'QvCreateNode': (0x47f090, r'YAPAUQvNode\w*@@PAVQvInput\w*@@PAUQvName\w*@@@Z'),
    'QvFactory': (0x47f1d0, r'YAPAUQvNode\w*@@(?:PAUQvName\w*@@|PAPAUQvEntryF@@)@Z'),
    'QvSkipBlock': (0x480490, r'YAXPAVQvInput\w*@@@Z'),
    'SkipBlock': (0x480490, r'YAXPAUQvInput@@@Z'),
    'QvReadTop': (0x481c30, r'YAHPAVQvInputT@@PAPAUQvNodeT@@@Z'),
    'QvParseVersion': (0x480f60, r'YAMPAD@Z'),
    'QvError': (0x482bd0, r'YAX(?:PAVQvInput\w*@@|PAVQvIn[346]@@)PBD(?:ZZ|@Z)'),
}


def address_for(symbol):
    if symbol == STATE_ALIAS:
        return 0x4c4238
    match = re.fullmatch(r'\?([^@]+)@@(.+)', symbol)
    if match and match[1] in STATIC:
        address, abi = STATIC[match[1]]
        return address if re.fullmatch(abi, match[2]) else None
    match = re.fullmatch(r'\?(\w+)@(Qv\w+)@@(.+)', symbol)
    if match:
        method, cls, suffix = match.groups()
        if re.fullmatch(r'Qv(?:Input\w*|In[3-7])', cls) and method in INPUT:
            address, abi = INPUT[method]
            return address if re.fullmatch(abi, suffix) else None
        if cls.startswith('QvDict'):
            candidates = {'Clear': (0x4821e0, r'QAEXXZ'),
                'Enter': (0x482230, r'QAEHIPAX@Z'), 'Find': (0x4822e0, r'QAEHIPAPAX@Z'),
                'FindSlot': (0x482310, r'QAEPAPAUQvDictEntry@@I@Z')}
            if method in candidates:
                address, abi = candidates[method]
                return address if re.fullmatch(abi, suffix) else None
        if cls == 'QvNodeS' and method == 'SetName' and suffix == 'QAEXPAPAUQvEntryS@@@Z':
            return 0x47e790
        if cls == 'QvNameY' and method == 'Init' and suffix == 'QAEPAU1@XZ':
            return 0x482560
    if re.fullmatch(r'\?\?1QvName[ENPQR]@@QAE@XZ', symbol):
        return 0x47ec60
    if symbol == '??1QvIn3@@QAE@XZ':
        return 0x481050
    if symbol == '??0QvInput8@@QAE@XZ':
        return 0x480fc0
    return None


def canonical_map(rows):
    result = {int(row['address'], 16): row['symbol'] for row in rows}
    # The parser bootstrap sits below score_table's parser-range cutoff.
    result[0x47e5d0] = '?init@QvNode@@SAXXZ'
    result[0x4c4238] = STATE_TARGET
    return result


def state_proof(read, symbols, sections):
    # Explicit original instruction/operand checks, independent of canonical
    # instruction scheduling: test slot, pass 251, store allocated dictionary.
    for address, expected in ((0x47e5eb, bytes.fromhex('833d38424c0000')),
                              (0x47e614, bytes.fromhex('68fb000000')),
                              (0x47e627, bytes.fromhex('a338424c00'))):
        if read(address, len(expected)) != expected:
            raise ValueError('Original node-name dictionary initialization changed')
    defs = {s['name']: s for s in symbols.values() if s['storage'] == 2 and s['section'] > 0}
    init = defs['?init@QvNode@@SAXXZ']
    refs = {symbols[i]['name'] for _, i, typ in sections[init['section']]['relocations'] if typ == 6}
    if STATE_TARGET not in defs or STATE_TARGET not in refs:
        raise ValueError('Canonical node initializer does not own/use the name dictionary')


def discover():
    verify_qv_library()
    units = json.loads((build.OUT / 'units.json').read_text())
    ownership = json.loads((build.OUT / 'ownership.json').read_text())
    existing = json.loads((build.REPO / 'tools/qv_exact_registry.json').read_text())
    existing.update(json.loads((build.REPO / 'tools/qv_name_registry.json').read_text()))
    with (build.REPO / 'qvlib/score_table.tsv').open() as stream:
        targets = canonical_map(list(csv.DictReader(stream, delimiter='\t')))
    raw = (build.REPO / 'qvlib/obj/QvLib.obj').read_bytes()
    symbols, sections = parse(raw)
    definitions = {s['name'] for s in symbols.values() if s['storage'] == 2 and s['section'] > 0}
    state_proof(original_reader(), symbols, sections)
    sources = {}
    for unit in units:
        if unit['returncode']:
            raise ValueError('Incomplete compiled inventory')
        for alias, kind in build.coff_symbols(pathlib.Path(unit['object'])):
            address = address_for(alias)
            if address is not None and alias not in existing and kind in ('undefined', 'strong', 'comdat'):
                sources.setdefault(alias, set()).add(unit['source'])
    canonical_sources = {s: hashlib.sha256((build.REPO / s).read_bytes()).hexdigest() for s in (
        'qvlib/include/QvInput.h', 'qvlib/include/QvDict.h', 'qvlib/include/QvNode.h',
        'qvlib/src/QvInput.c++', 'qvlib/src/QvDict.c++', 'qvlib/src/QvNode.c++')}
    accepted, withheld = {}, {}
    for alias, referring in sorted(sources.items()):
        address = address_for(alias)
        target = targets.get(address)
        if target not in definitions:
            raise ValueError('Missing canonical parser owner: ' + alias)
        if alias == target:
            continue
        owners = ownership['definitions'].get(alias, [])
        if owners and (not set(owners).issubset(OWNERS)
                       or ownership['symbol_addresses'].get(alias) != ['%08x' % address]):
            withheld[alias] = 'Loose owner/address outside reviewed parser family'
            continue
        accepted[alias] = dict(address='%08x' % address, target=target,
            data=alias == STATE_ALIAS, loose_owners=owners, owner='qvlib/obj/QvLib.lib',
            source_sha256={s: hashlib.sha256((build.SRC / s).read_bytes()).hexdigest() for s in sorted(referring)},
            canonical_sources=canonical_sources,
            abi='Finite reviewed ABI table: thiscall for input/dict/node members; cdecl for static parser and variadic diagnostics. Input prefix: file/line/version/header/ok +0/+4/+8/+12/+16, dict +20, 40-byte string +28, index +68. Dict table size/buckets +0/+4, entries key/value/next +0/+4/+8. Name handles one pointer; reference/out-pointer arguments passed as addresses. No hidden-result adapter or this adjustment.',
            reason='Original-address identity and reviewed arguments/layouts; canonical implementation and shared node-name dictionary replace local parser spellings. Functional owners accepted independently of byte score.')
    return dict(accepted=accepted, withheld=withheld)


def reviewed():
    path = build.REPO / 'tools/qv_input_registry.json'
    if not path.exists():
        return {}
    saved = json.loads(path.read_text())
    current = discover()['accepted']
    for alias, decision in saved.items():
        if current.get(alias) != decision:
            raise ValueError('Qv input/name-registry evidence changed: ' + alias)
    return saved


if __name__ == '__main__':
    import sys
    report = discover()
    (build.OUT / 'qv-input-candidates.json').write_text(json.dumps(report, indent=2))
    print('%d reviewed input/name/dictionary bindings; %d withheld' % (len(report['accepted']), len(report['withheld'])))
    if '--register' in sys.argv:
        (build.REPO / 'tools/qv_input_registry.json').write_text(json.dumps(report['accepted'], indent=2) + '\n')
