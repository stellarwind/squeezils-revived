"""Project verified addressed owners into diagnostic link objects.

VC4 emits NODUPLICATES COMDATs for incidental template instantiations. Keep
each addressed implementation in its target-owning source and make other
copies external references. Do not use this for unverified name collisions.
No code bytes from the original EXE are used; input objects are compiled source.
"""
import collections
import csv
import hashlib
import json
import pathlib
import struct
import integration as build

POLICY = {
    "?Grow@?$Array@VString@@@@UAEHH@Z": ("0045e370", "Container/ArrayString.cpp"),
    "?IndexOf@?$Set@VString@@$0BH@@@UAEHABVString@@@Z": ("00462710", "Container/SetString23.cpp"),
    # Same base destructor, using the complete hierarchy's vtable owner.
    "??1Function@@UAE@XZ": ("00422ea0", "Function/Function.cpp"),
    # Both scoring files contain both bodies with identical layouts and code.
    "?PeekA@Bindings43b@@QAEHH@Z": ("0043de80", "Tiny4/F43de80.cpp"),
    "?PeekW@Bindings43b@@QAEHH@Z": ("0043eb30", "Tiny4/F43eb30.cpp"),
    # Empty destructor at 0x401660, also emitted by a cleanup adapter.
    "??1Vector3f@@QAE@XZ": ("00401660", "Vector3f/Vector3fOps.cpp"),
    # Explicit String Read specializations supersede incidental instantiations
    # from Set::Create and table factories. Their layout comes from PrimBuf.h.
    "?Read@?$PrimitiveArray@VString@@$02@@UAEXPAX@Z": ("0045e620", "Container2/gen/PA_s3.cpp"),
    "?Read@?$PrimitiveArray@VString@@$0BH@@@UAEXPAX@Z": ("004622e0", "Container2/gen/PA_s23.cpp"),
    # Same Array pointer-copy constructor in the shared template and its
    # addressed specialization, with identical count/capacity/data layout.
    "??0?$Array@PAV?$PrimitiveArray@M$01@@@@QAE@HPBQAV?$PrimitiveArray@M$01@@@Z":
        ("0046a6c0", "Container/ArrayFloatArrayCopy.cpp"),
}

# These nonvirtual dictionary/list/field-data methods have the same layouts
# and zero-score upstream mappings. The complete QvTranslation class also
# owns its virtual table, so discard that entire constructor-only shadow TU.
QV_POLICY = {
    "??0QvDict@@QAE@H@Z": ("00482180", "Qv/Dict.cpp"),
    "??1QvDict@@QAE@XZ": ("004821c0", "Qv/Dict.cpp"),
    "??1QvFieldData@@QAE@XZ": ("00480630", "Qv/FieldDataDtor.cpp"),
    "??0QvPList@@QAE@XZ": ("00482a60", "Qv/List.cpp"),
    "??1QvPList@@QAE@XZ": ("00482a80", "Qv/List.cpp"),
    "??0QvTranslation@@QAE@XZ": ("00483460", "Qv/Translation.cpp"),
}


def verify_qv_library():
    obj = build.REPO / "qvlib/obj/QvLib.obj"
    archive = (build.REPO / "qvlib/obj/QvLib.lib").read_bytes()
    raw = obj.read_bytes()
    if archive[:8] != b"!<arch>\n":
        raise ValueError("Invalid QvLib archive")
    position, found = 8, False
    while position < len(archive):
        header = archive[position:position + 60]
        if len(header) != 60 or header[58:60] != b"`\n":
            raise ValueError("Malformed QvLib archive member")
        size = int(header[48:58])
        start = position + 60
        found |= archive[start:start + size] == raw
        position = start + size + size % 2
    if not found:
        raise ValueError("QvLib.obj does not match any library member")
    definitions = dict(build.coff_symbols(obj))
    with (build.REPO / "qvlib/score_table.tsv").open() as stream:
        scores = {row["symbol"]: row for row in csv.DictReader(stream, delimiter="\t")}
    for name, (address, _) in QV_POLICY.items():
        row = scores.get(name, {})
        if (row.get("address", "").lower() != address or row.get("score") != "0"
                or definitions.get(name) not in ("strong", "comdat")):
            raise ValueError("QvLib ownership lacks exact entry evidence: " + name)
    return hashlib.sha256(raw).hexdigest()


def project(raw, names):
    b = bytearray(raw)
    _, ns, _, sp, count, opt, _ = struct.unpack_from("<HHIIIHH", b)
    st = sp + count * 18
    symbols, i = {}, 0
    while i < count:
        p = sp + i * 18
        word = b[p:p + 8]
        if word[:4] == bytes(4):
            start = st + struct.unpack_from("<I", word, 4)[0]
            name = b[start:b.index(0, start)].decode("ascii", "replace")
        else:
            name = word.rstrip(b"\0").decode("ascii", "replace")
        val, section, typ, storage, aux = struct.unpack_from("<IhHBB", b, p + 8)
        symbols[i] = (p, name, val, section, storage, aux)
        i += 1 + aux
    removed = {s for p, name, val, s, storage, aux in symbols.values()
               if name in names and storage == 2 and s > 0}
    if len(removed) != len(names):
        raise ValueError("Each selected symbol must own a distinct code section")
    # Follow IMAGE_COMDAT_SELECT_ASSOCIATIVE metadata sections to their parent.
    associates = {}
    for p, name, val, section, storage, aux in symbols.values():
        if storage == 3 and section > 0 and aux and name.startswith("."):
            if b[p + 18 + 14] == 5:
                associates[section] = struct.unpack_from("<h", b, p + 18 + 12)[0]
    while True:
        more = {s for s, parent in associates.items() if parent in removed} - removed
        if not more:
            break
        removed.update(more)
    for p, name, val, section, storage, aux in symbols.values():
        if section in removed and storage == 2:
            if name not in names or val != 0 or aux:
                raise ValueError("Unexpected additional external definition in removed section: " + name)
            struct.pack_into("<Ih", b, p + 8, 0, 0)
        elif section in removed and storage == 3 and aux and name.startswith("."):
            # A removed section is no longer a COMDAT: its leading external
            # symbol is now undefined. VC4 otherwise rejects it during Pass1.
            b[p:p + 8] = b".sqzdrop"
            b[p + 18:p + 36] = bytes(18)
    # Kept sections must not require a local label inside discarded code. Direct
    # calls through the selected external symbols are explicitly allowed.
    for si in range(1, ns + 1):
        p = 20 + opt + (si - 1) * 40
        if si in removed:
            flags = struct.unpack_from("<I", b, p + 36)[0]
            if not flags & 0x1000:
                raise ValueError("Refusing to discard a non-COMDAT section")
            b[p:p + 8] = b".sqzdrop"
            struct.pack_into("<II", b, p + 16, 0, 0)
            struct.pack_into("<I", b, p + 24, 0)
            struct.pack_into("<H", b, p + 32, 0)
            struct.pack_into("<I", b, p + 36, 0x800)
            continue
        rel = struct.unpack_from("<I", b, p + 24)[0]
        nr = struct.unpack_from("<H", b, p + 32)[0]
        for ri in range(nr):
            sym = struct.unpack_from("<I", b, rel + ri * 10 + 4)[0]
            _, name, _, section, _, _ = symbols[sym]
            if section in removed and name not in names:
                raise ValueError("Retained section references discarded local symbol: " + name)
    return b, sorted(removed)


def run():
    report = json.loads((build.OUT / "ownership.json").read_text())
    units = json.loads((build.OUT / "units.json").read_text())
    qv_hash = verify_qv_library()
    for name, (address, owner) in dict(POLICY, **QV_POLICY).items():
        if report["symbol_addresses"].get(name) != [address]:
            raise ValueError("Address identity is not unique: " + name)
        if not any(e["symbol"] == name and e["source"] == owner
                   for e in report["addresses"][address]):
            raise ValueError("Missing target owner: " + name)
    destination = build.OUT / "containers"
    destination.mkdir(exist_ok=True)
    rsp = (build.OUT / "link.rsp").read_text()
    changed, excluded, totals = [], [], collections.Counter()
    for index, unit in enumerate(units):
        if unit["returncode"]:
            continue
        source = unit["source"]
        if source == "Qv/Translation.cpp":
            targeted = {a for a, entries in report["addresses"].items()
                        if any(e["source"] == source for e in entries)}
            if targeted != {"00483460"}:
                raise ValueError("Unexpected additional QvTranslation target")
            original = pathlib.Path(unit["object"])
            line = '"%s"\n' % original
            if rsp.count(line) != 1:
                raise ValueError("Missing QvTranslation input")
            rsp = rsp.replace(line, "")
            excluded.append(source)
            continue
        names = {name for name, (_, owner) in POLICY.items()
                 if source != owner and source in report["definitions"].get(name, [])}
        names.update(name for name in QV_POLICY
                     if source in report["definitions"].get(name, []))
        if not names:
            continue
        original = pathlib.Path(unit["object"])
        projected = destination / ("%04d_%s.obj" % (index, pathlib.Path(source).stem))
        data, removed = project(original.read_bytes(), names)
        projected.write_bytes(data)
        # Verify the selected definitions really became undefined references.
        output_symbols = dict(build.coff_symbols(projected))
        if any(output_symbols.get(name) != "undefined" for name in names):
            raise ValueError("Projection did not externalize every selected symbol")
        old = '"%s"' % original
        if rsp.count(old) != 1:
            raise ValueError("Expected exactly one response-file input: " + source)
        rsp = rsp.replace(old, '"%s"' % projected)
        totals.update(names)
        changed.append(dict(source=source, object=str(projected),
                            externalized=sorted(names), discarded_sections=removed,
                            input_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),
                            output_sha256=hashlib.sha256(data).hexdigest()))
    # Check ownership across the complete projected loose-object collection,
    # not just each modified object in isolation.
    projected_by_source = {entry["source"]: pathlib.Path(entry["object"]) for entry in changed}
    owners = collections.defaultdict(list)
    for unit in units:
        if unit["returncode"] or unit["source"] in excluded:
            continue
        path = projected_by_source.get(unit["source"], pathlib.Path(unit["object"]))
        for name, kind in build.coff_symbols(path):
            if name in dict(POLICY, **QV_POLICY) and kind != "undefined":
                owners[name].append(unit["source"])
    for name, (_, owner) in POLICY.items():
        if owners[name] != [owner]:
            raise ValueError("Projected collection does not have exactly its canonical owner: " + name)
    for name in QV_POLICY:
        if owners[name]:
            raise ValueError("Loose definition still competes with QvLib: " + name)
        owners[name] = ["qvlib/obj/QvLib.lib(QvLib.obj)"]
    (destination / "link.rsp").write_text(rsp, encoding="ascii")
    summary = dict(diagnostic_only=True, policy=POLICY, qv_policy=QV_POLICY,
                   qv_object_sha256=qv_hash, excluded_sources=excluded, modified_objects=len(changed),
                   removed_duplicate_definitions=dict(totals), verified_owners=dict(owners), objects=changed)
    (destination / "projection.json").write_text(json.dumps(summary, indent=2))
    print(json.dumps({k: v for k, v in summary.items() if k != "objects"}, indent=2))


if __name__ == "__main__":
    run()
