"""One-shot VC4 full-link probe. Run with flagsearch/env.sh sourced (Git Bash).

This deliberately keeps every manifest unit: duplicate definitions and unresolved
names are evidence to resolve, never errors to suppress with /FORCE.
Generated files stay in ignored build/integration/; source units are untouched.
"""
import collections
import argparse
import hashlib
import json
import pathlib
import re
import shlex
import shutil
import struct
import subprocess

REPO = pathlib.Path(__file__).resolve().parents[1]
SRC = REPO / "src"
OUT = REPO / "build" / "integration"
DEFAULT = "-GX -GR -Gy -O2 -G5 -ML"


def inventory_ownership():
    """Map scoring targets to actual external definitions, without guessing aliases."""
    units = json.loads((OUT / "units.json").read_text())
    by_source, definitions = {}, collections.defaultdict(list)
    for unit in units:
        if unit["returncode"]:
            continue
        names = {name for name, kind in coff_symbols(pathlib.Path(unit["object"]))
                 if kind != "undefined"}
        by_source[unit["source"]] = names
        for name in names:
            definitions[name].append(unit["source"])
    addresses = collections.defaultdict(list)
    unmatched, invalid = [], []
    symbol_addresses = collections.defaultdict(set)
    for row in (SRC / "manifest.txt").read_text().splitlines():
        if not row.strip() or row.lstrip().startswith("#"):
            continue
        target, *sources = row.split()
        for number, line in enumerate((SRC / target).read_text().splitlines(), 1):
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            fields = line.split()
            if len(fields) < 3:
                invalid.append([target, number, "missing fields"])
                continue
            expression, address, label = fields[:3]
            try:
                address = "%08x" % int(address, 16)
                # Perl (used by flagsearch.pl) permits optional anchors; Python
                # requires grouping them. Preserve that matching behavior.
                expression = re.sub(r"(?<!\\)\^(\?\??)", r"(?:^)\1", expression)
                pattern = re.compile(expression)
            except (ValueError, re.error) as error:
                invalid.append([target, number, str(error)])
                continue
            matches = [(source, name) for source in sources
                       for name in by_source.get(source, ()) if pattern.search(name)]
            if not matches:
                unmatched.append([target, number, address, label])
            for source, name in matches:
                entry = dict(symbol=name, source=source, target=target, label=label)
                if entry not in addresses[address]:
                    addresses[address].append(entry)
                symbol_addresses[name].add(address)
    symbol_addresses = {name: sorted(values) for name, values in symbol_addresses.items()}
    report = dict(addresses=dict(sorted(addresses.items())),
                  symbol_addresses=symbol_addresses,
                  definitions=dict(sorted(definitions.items())),
                  unmatched=unmatched, invalid=invalid,
                  warning="Target regex matches are candidates, not ABI or ownership proof.")
    (OUT / "ownership.json").write_text(json.dumps(report, indent=2))
    summary = dict(mapped_addresses=len(addresses), targeted_symbols=len(symbol_addresses),
                   unmatched_target_lines=len(unmatched), invalid_lines=len(invalid),
                   symbols_targeting_multiple_addresses=sum(len(v) > 1 for v in symbol_addresses.values()))
    print(json.dumps(summary, indent=2))
    return 0


def diagnose_existing():
    """Link saved successful objects for diagnostics; never retain an EXE.

    This mode does not validate or rebuild cached objects. Its output is not
    evidence of a complete build, even if the diagnostic link succeeds.
    """
    summary = json.loads((OUT / "summary.json").read_text())
    with (OUT / "partial-link.log").open("wb") as stream:
        result = subprocess.run([summary["linker"], "@" + str(OUT / "link.rsp")],
                                cwd=OUT, stdout=stream, stderr=subprocess.STDOUT)
    (OUT / "SQZ.exe").unlink(missing_ok=True)
    log = (OUT / "partial-link.log").read_text(errors="replace")
    unresolved = collections.Counter(
        line.split("unresolved external symbol ", 1)[-1]
        for line in log.splitlines() if "LNK2001" in line)
    report = dict(diagnostic_only=True, cached_objects_verified=False,
                  omitted_failed_sources=summary["failed"],
                  link_returncode=result.returncode,
                  internal_linker_error="Internal error" in log,
                  linker_codes=dict(collections.Counter(re.findall(r"\bLNK\d+\b", log))),
                  diagnostic_pass_completed=result.returncode == 0 or "LNK1120" in log,
                  unique_unresolved_messages=(len(unresolved) if result.returncode == 0
                                              or "LNK1120" in log else None))
    (OUT / "partial-link-summary.json").write_text(json.dumps(report, indent=2))
    (OUT / "unresolved-grouped.json").write_text(json.dumps(dict(unresolved.most_common()), indent=2))
    print(json.dumps(report, indent=2))
    return result.returncode


def coff_symbols(path):
    """Read external COFF symbols; common data and COMDATs may legally repeat."""
    b = path.read_bytes()
    machine, ns, _, sp, count, opt, _ = struct.unpack_from("<HHIIIHH", b)
    if machine != 0x14c:
        raise ValueError("Expected i386 COFF: " + str(path))
    st = sp + count * 18
    flags = [struct.unpack_from("<I", b, 20 + opt + i * 40 + 36)[0]
             for i in range(ns)]
    i = 0
    while i < count:
        p = sp + i * 18
        raw = b[p:p + 8]
        if raw[:4] == bytes(4):
            start = st + struct.unpack_from("<I", raw, 4)[0]
            name = b[start:b.index(0, start)].decode("ascii", "replace")
        else:
            name = raw.rstrip(b"\0").decode("ascii", "replace")
        value, section, _, storage, aux = struct.unpack_from("<IhHBB", b, p + 8)
        if storage == 2:
            kind = ("comdat" if flags[section - 1] & 0x1000 else "strong") if section > 0 else (
                "absolute" if section == -1 else "common" if value else "undefined")
            yield name, kind
        i += aux + 1


def run():
    OUT.mkdir(parents=True, exist_ok=True)
    cl, link = shutil.which("cl"), shutil.which("link")
    if not cl or not link:
        raise SystemExit("Source tools/flagsearch/env.sh before running this script.")
    # Avoid accidentally selecting Git's POSIX link utility.
    if pathlib.Path(link).parent != pathlib.Path(cl).parent:
        raise SystemExit("cl and link must come from the same VC4 BIN directory.")
    jobs = {}
    conflicts = []
    rows = 0
    for line in (SRC / "manifest.txt").read_text().splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        target, *sources = line.split()
        rows += 1
        text = (SRC / target).read_text()
        match = re.search(r"^#FLAGS (.+)$", text, re.M)
        flags = match.group(1).strip() if match else DEFAULT
        for source in sources:
            if source in jobs and jobs[source] != flags:
                conflicts.append([source, jobs[source], flags])
                # The only supported reconciliation is default vs an explicit
                # per-target flag set. This probe prefers the explicit set.
                if DEFAULT not in (jobs[source], flags):
                    raise SystemExit("Conflicting explicit flags for " + source)
                jobs[source] = flags if flags != DEFAULT else jobs[source]
            jobs.setdefault(source, flags)
    if conflicts:
        (OUT / "flag-conflicts.json").write_text(json.dumps(conflicts, indent=2))

    objects, failed, units = [], [], []
    for index, (source, flags) in enumerate(jobs.items()):
        folder = OUT / "obj" / ("%04d" % index)
        folder.mkdir(parents=True, exist_ok=True)
        obj = folder / ("%04d_%s.obj" % (index, pathlib.Path(source).stem))
        obj.unlink(missing_ok=True)
        command = [cl, "-nologo", "-c", *shlex.split(flags),
                   "-Fo" + str(obj), str(SRC / source)]
        result = subprocess.run(command, cwd=folder, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)
        (folder / "compile.log").write_bytes(result.stdout)
        units.append(dict(source=source, flags=flags, object=str(obj),
                          returncode=result.returncode))
        if result.returncode or not obj.exists():
            failed.append(source)
        else:
            objects.append(obj)
    (OUT / "units.json").write_text(json.dumps(units, indent=2))

    # Cached inputs are allowed for diagnosis, explicitly recorded as unverified.
    data = REPO / "data" / "data.obj"
    res = REPO / "data" / "resources.res"
    qv = REPO / "qvlib" / "obj" / "QvLib.lib"
    inputs = [data, res, qv]
    missing = [str(p) for p in inputs if not p.exists()]
    definitions = collections.defaultdict(list)
    references = collections.defaultdict(list)
    for obj in objects + ([data] if data.exists() else []):
        for name, kind in coff_symbols(obj):
            entry = [str(obj.relative_to(REPO)), kind]
            (references if kind == "undefined" else definitions)[name].append(entry)
    duplicates = {n: v for n, v in definitions.items()
                  if sum(kind == "strong" for _, kind in v) > 1}
    external = {n: v for n, v in references.items() if n not in definitions}
    (OUT / "duplicate-strong.json").write_text(json.dumps(duplicates, indent=2))
    (OUT / "external-references.json").write_text(json.dumps(external, indent=2))
    # This inventory intentionally does not resolve archive/import-library members.
    libraries = ["libc.lib", "kernel32.lib", "user32.lib", "gdi32.lib",
                 "comdlg32.lib", "advapi32.lib", "winmm.lib", "msacm32.lib",
                 "ddraw.lib", "dsound.lib", "d3drm.lib"]
    exe = OUT / "SQZ.exe"
    exe.unlink(missing_ok=True)
    rsp = OUT / "link.rsp"
    args = ["-nologo", "-machine:IX86", "-subsystem:windows",
            '-out:"%s"' % exe, '-map:"%s"' % (OUT / "SQZ.map")]
    args += ['"%s"' % p for p in objects + inputs]
    args += libraries
    rsp.write_text("\n".join(args) + "\n", encoding="ascii")
    link_result = None
    if not failed and not missing:
        with (OUT / "link.log").open("wb") as stream:
            result = subprocess.run([link, "@" + str(rsp)], cwd=OUT,
                                    stdout=stream, stderr=subprocess.STDOUT)
        link_result = result.returncode
    else:
        (OUT / "link.log").write_text("Link skipped: failed compilations or missing inputs.\n")
    log = (OUT / "link.log").read_text(errors="replace")
    if link_result != 0:
        exe.unlink(missing_ok=True)
    errors = collections.Counter(re.findall(r"\b(LNK\d+)\b", log))
    summary = dict(manifest_rows=rows, unique_sources=len(jobs),
                   reconciled_default_flag_conflicts=conflicts,
                   compiled=len(objects), failed=failed, missing_inputs=missing,
                   strong_duplicate_names=len(duplicates),
                   external_names_before_libraries=len(external),
                   link_returncode=link_result, linker_codes=dict(errors),
                   exe_linked=link_result == 0 and exe.exists(),
                   cached_inputs_verified=False,
                   cached_inputs={str(p.relative_to(REPO)): hashlib.sha256(p.read_bytes()).hexdigest()
                                  for p in inputs if p.exists()},
                   compiler=cl, linker=link)
    (OUT / "summary.json").write_text(json.dumps(summary, indent=2))
    print(json.dumps(summary, indent=2), flush=True)
    return 0 if summary["exe_linked"] else 1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--diagnose-existing-link", action="store_true",
                        help="Link saved inputs for diagnostics only; retain no EXE")
    parser.add_argument("--inventory-ownership", action="store_true",
                        help="Map target addresses to compiled external symbols")
    options = parser.parse_args()
    if options.inventory_ownership:
        raise SystemExit(inventory_ownership())
    raise SystemExit(diagnose_existing() if options.diagnose_existing_link else run())
