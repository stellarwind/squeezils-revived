"""One-shot VC4 full-link probe. Run with flagsearch/env.sh sourced (Git Bash).

This deliberately keeps every manifest unit: duplicate definitions and unresolved
names are evidence to resolve, never errors to suppress with /FORCE.
Generated files stay in ignored build/integration/; source units are untouched.
"""
import collections
import argparse
import hashlib
import json
import os
import pathlib
import re
import shlex
import shutil
import struct
import subprocess
from target_rows import parse_target_row

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
            try:
                fields = parse_target_row(line)
            except ValueError as error:
                invalid.append([target, number, str(error)])
                continue
            if fields is None:
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
    # Invalid metadata must stop alias generation instead of hiding owners.
    return 1 if invalid else 0


def link_arguments(response):
    """Expand our one-argument-per-line inventory outside VC4's fragile parser.

    The full @file aborts during Pass1; the same inputs supplied directly with
    relative paths complete the diagnostic. Keep the response as the inventory.
    """
    args = []
    for line in response.read_text().splitlines():
        if not line.strip():
            continue
        if not re.fullmatch(r'(?:[^"\s]+|"[^"]*")+', line):
            raise ValueError("Expected one quoted argument per response line: " + line)
        arg = line.replace('"', '')
        prefix = next((p for p in ("-out:", "-map:") if arg.startswith(p)), "")
        path = arg[len(prefix):]
        if pathlib.Path(path).is_absolute():
            arg = prefix + os.path.relpath(path, OUT)
        args.append(arg)
    return args


def launch_link(linker, response, log):
    command = [linker, *link_arguments(response)]
    if os.name == "nt" and len(subprocess.list2cmdline(command)) >= 32767:
        raise ValueError("Direct linker command exceeds Windows' limit; shorten input paths")
    with log.open("wb") as stream:
        return subprocess.run(command, cwd=OUT, stdout=stream, stderr=subprocess.STDOUT)


def diagnose_existing(response=None):
    """Link saved successful objects for diagnostics; never retain an EXE.

    This mode does not validate or rebuild cached objects. Its output is not
    evidence of a complete build, even if the diagnostic link succeeds.
    """
    summary = json.loads((OUT / "summary.json").read_text())
    response = pathlib.Path(response).resolve() if response else OUT / "link.rsp"
    response.relative_to(OUT)  # Diagnostic inventories must live under the build directory.
    destination = response.parent
    result = launch_link(summary["linker"], response, destination / "partial-link.log")
    (OUT / "SQZ.exe").unlink(missing_ok=True)
    log = (destination / "partial-link.log").read_text(errors="replace")
    unresolved = collections.Counter(
        line.split("unresolved external symbol ", 1)[-1]
        for line in log.splitlines() if "LNK2001" in line)
    report = dict(diagnostic_only=True, cached_objects_verified=False,
                  response=str(response), launcher="direct arguments, relative paths",
                  omitted_failed_sources=summary["failed"],
                  link_returncode=result.returncode,
                  internal_linker_error="Internal error" in log,
                  linker_codes=dict(collections.Counter(re.findall(r"\bLNK\d+\b", log))),
                  diagnostic_pass_completed=result.returncode == 0 or "LNK1120" in log,
                  unique_unresolved_messages=(len(unresolved) if result.returncode == 0
                                              or "LNK1120" in log else None))
    (destination / "partial-link-summary.json").write_text(json.dumps(report, indent=2))
    (destination / "unresolved-grouped.json").write_text(json.dumps(dict(unresolved.most_common()), indent=2))
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


def run(compile_only=False):
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
        command = [cl, "-nologo", "-c", "-DSQZ_INTEGRATION", *shlex.split(flags),
                   "-Fo" + str(obj), str(SRC / source)]
        result = subprocess.run(command, cwd=folder, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)
        (folder / "compile.log").write_bytes(result.stdout)
        units.append(dict(source=source, flags=flags, integration_defines=["SQZ_INTEGRATION"], object=str(obj),
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
    alias = OUT / "data_alias.obj"   # tools/data_aliases.py: weak externals, readable name -> data.obj symbol
    entry_alias = OUT / "entry_alias.obj"  # tools/entry_aliases.py: verified entry pointers only
    runtime_alias = OUT / "runtime_alias.obj"  # tools/runtime_aliases.py: verified CRT ABI
    qv_alias = OUT / "qv_alias.obj"  # tools/qv_aliases.py: address-proven constructors/destructors
    inputs = [data, res, qv] + [p for p in (alias, entry_alias, runtime_alias, qv_alias,
        OUT / 'rtti_alias.obj', OUT / 'rtti_data.obj') if p.exists()]
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
    if compile_only:
        (OUT / "link.log").write_text("Link deferred until alias generation and projection.\n")
    elif not failed and not missing:
        result = launch_link(link, rsp, OUT / "link.log")
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
    return (0 if not failed and not missing else 1) if compile_only else (0 if summary["exe_linked"] else 1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--diagnose-existing-link", action="store_true",
                        help="Link saved inputs for diagnostics only; retain no EXE")
    parser.add_argument("--inventory-ownership", action="store_true",
                        help="Map target addresses to compiled external symbols")
    parser.add_argument("--compile-only", action="store_true",
                        help="Compile and inventory inputs; defer linking")
    parser.add_argument("--diagnostic-response", type=pathlib.Path,
                        help="Saved response inventory under build/integration (diagnostic mode only)")
    options = parser.parse_args()
    if options.diagnostic_response and not options.diagnose_existing_link:
        parser.error("--diagnostic-response requires --diagnose-existing-link")
    if options.inventory_ownership:
        raise SystemExit(inventory_ownership())
    raise SystemExit(diagnose_existing(options.diagnostic_response) if options.diagnose_existing_link else run(options.compile_only))
