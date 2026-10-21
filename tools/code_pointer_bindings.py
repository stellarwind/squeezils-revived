"""Bind reviewed CRT entries and shared empty methods using current code bytes."""
import json
import pathlib
import struct

import integration as build
from project_eh import parse


def original_reader():
    raw = (build.REPO.parent / "SqueezilsGameFiles/SQZ.EXE").read_bytes()
    pe = struct.unpack_from("<I", raw, 60)[0]
    opt = pe + 24
    base = struct.unpack_from("<I", raw, opt + 28)[0]
    sections = [struct.unpack_from("<IIII", raw, opt + struct.unpack_from("<H", raw, pe + 20)[0] + i * 40 + 8)
                for i in range(struct.unpack_from("<H", raw, pe + 6)[0])]
    def read(address, size):
        for _, va, disk, offset in sections:
            if base + va <= address and address + size <= base + va + disk:
                p = offset + address - base - va
                return raw[p:p + size]
        raise ValueError("Original address outside raw image")
    return read


def archive_members(path):
    raw = path.read_bytes()
    if raw[:8] != b"!<arch>\n":
        raise ValueError("Not a COFF archive")
    position = 8
    while position < len(raw):
        header = raw[position:position + 60]
        if header[58:60] != b"`\n":
            raise ValueError("Invalid archive member")
        size = int(header[48:58])
        content = raw[position + 60:position + 60 + size]
        if content[:2] == b"\x4c\x01":
            yield position, content
        position += 60 + size + size % 2


def bind(libc):
    read = original_reader()
    reviewed = {
        0x48e4d0: ["__purecall"],
        0x48e930: ["__fpclear"],
        0x48ee10: ["??0__non_rtti_object@@QAE@PBD@Z", "??0__non_rtti_object@@QAE@ABV0@@Z"],
        0x4904c0: ["_malloc"],
        0x490540: ["_free"],
        0x490560: ["_realloc"],
        0x492e50: ["?terminate@@YAXXZ"],
        0x497e00: ["?__CxxUnhandledExceptionFilter@@YGJPAU_EXCEPTION_POINTERS@@@Z"],
    }
    targets = {s for values in reviewed.values() for s in values}
    archive = libc.read_bytes()
    index_size = int(archive[8 + 48:8 + 58])
    linker_index = archive[68:68 + index_size]
    count = struct.unpack_from(">I", linker_index)[0]
    member_offsets = struct.unpack_from(">%dI" % count, linker_index, 4)
    names = linker_index[4 + count * 4:].split(b"\0")
    selected_members = {offset for name, offset in zip(names, member_offsets)
                        if name.decode("latin-1") in targets}
    matched = {}
    diagnostics = []
    startup = []
    for member, raw in archive_members(libc):
        if member not in selected_members:
            continue
        symbols, sections = parse(raw)
        if any(s["name"] == "?__CxxUnhandledExceptionFilter@@YGJPAU_EXCEPTION_POINTERS@@@Z"
               and s["section"] > 0 for s in symbols.values()):
            # This library member owns the replacement .CRT$XCC initializer.
            # Prove both its thunk and constructor against the original code.
            checks = []
            for si, sec in sections.items():
                if raw[sec["position"]:sec["position"] + 8] != b".CRT$XCC":
                    continue
                if sec["size"] != 4 or len(sec["relocations"]) != 1:
                    raise ValueError("Unexpected CRT startup record")
                init = symbols[sec["relocations"][0][1]]
                code = sections[init["section"]]
                for address, section in ((0x497dd0, code), (0x497de0, sections[4])):
                    body = raw[section["offset"]:section["offset"] + section["size"]]
                    mask = {i for offset, _, _ in section["relocations"] for i in range(offset, offset + 4)}
                    expected = read(address, len(body))
                    checks.append(all(a == b for i, (a, b) in enumerate(zip(body, expected)) if i not in mask))
            if checks != [True, True]:
                raise ValueError("Replacement CRT startup differs from original")
            startup.append(dict(address="004b9004", size=4, initializer="00497dd0",
                                member=member, validation="original initializer/constructor match current LIBC .CRT$XCC owner"))
        for index, symbol in symbols.items():
            if symbol["name"] not in targets or symbol["section"] <= 0 or symbol["storage"] != 2:
                continue
            sec = sections[symbol["section"]]
            flags = struct.unpack_from("<I", raw, sec["position"] + 36)[0]
            if not flags & 0x20:
                raise ValueError("Reviewed CRT function target is not executable code")
            lo = symbol["value"]
            ends = [s["value"] for s in symbols.values() if s["section"] == symbol["section"]
                    and s["storage"] == 2 and s["value"] > lo]
            hi = min(ends, default=sec["size"])
            body = raw[sec["offset"] + lo:sec["offset"] + hi].rstrip(b"\xcc")
            mask = set()
            for offset, _, typ in sec["relocations"]:
                if lo <= offset < lo + len(body):
                    if typ not in (6, 20):
                        raise ValueError("Unsupported CRT code relocation")
                    mask.update(range(offset - lo, offset - lo + 4))
            for address, names in reviewed.items():
                if symbol["name"] not in names:
                    continue
                original = read(address, len(body))
                eligible = (len(body) - len(mask) >= 3
                            or (address == 0x48e930 and body == b"\xc3" and not mask))
                equal = eligible and all(a == b for i, (a, b) in enumerate(zip(body, original)) if i not in mask)
                diagnostics.append(dict(address="%08x" % address, target=symbol["name"],
                                        member=member, size=len(body), equal=equal))
                if equal:
                    matched.setdefault(address, []).append((symbol["name"], member, len(body)))
    pairs, evidence = [], []
    for address, matches in matched.items():
        if len(matches) != 1:
            raise ValueError("CRT entry has ambiguous exact owners")
        target, member, size = matches[0]
        alias = "_fn_%06X" % address
        pairs.append((alias, target))
        evidence.append(dict(alias=alias, target=target, member=member, size=size,
                             validation="whole original body equals LIBC body outside COFF relocations"))
    units = json.loads((build.OUT / "units.json").read_text())
    shared = [
        (0x471af0, "CameraSoul/CameraSoul.cpp", "?void2@CameraSoulStubs@@QAEXHH@Z", bytes.fromhex("c20800")),
        (0x471b10, "CameraSoul/CameraSoul.cpp", "?void2@CameraSoulStubs@@QAEXHH@Z", bytes.fromhex("c20800")),
        (0x471b00, "Bitmap2/Bitmap2.cpp", "?ReportError@Bitmap@@QAEXJ@Z", bytes.fromhex("c20400")),
    ]
    for address, source, target, expected in shared:
        unit = next(u for u in units if u["source"] == source)
        raw = pathlib.Path(unit["object"]).read_bytes()
        symbols, sections = parse(raw)
        symbol = next(s for s in symbols.values() if s["name"] == target and s["section"] > 0)
        offset = sections[symbol["section"]]["offset"] + symbol["value"]
        if unit["returncode"] or raw[offset:offset + len(expected)] != expected or read(address, len(expected)) != expected:
            raise ValueError("Shared cleanup ABI/body disagrees")
        alias = "_fn_%06X" % address
        pairs.append((alias, target))
        evidence.append(dict(alias=alias, target=target, owner=source,
                             validation="identical empty thiscall body and exact ret argument cleanup"))
    return pairs, evidence, diagnostics, startup
