"""One bounded compile -> aliases -> projection -> diagnostic pass.

Source flagsearch/env.sh first. Extracted data/resources/QvLib are deliberately
unverified cached inputs; this command is not a release verification gate.
"""
import datetime
import hashlib
import json
import os
import pathlib
import subprocess
import sys
import uuid

import integration as build


def snapshot():
    paths = {build.REPO.parent / 'SqueezilsGameFiles/SQZ.EXE',
             build.REPO.parent / 'nm/full.asm'}
    for folder in (build.SRC, build.REPO / "tools", build.REPO / "data", build.REPO / "qvlib"):
        paths.update(p for p in folder.rglob("*") if p.is_file()
                     and p.suffix.lower() in (".cpp", ".c", ".h", ".py", ".pl", ".sh", ".txt", ".tsv", ".json", ".obj", ".res", ".lib"))
    for folder in os.environ.get("LIB", "").split(";"):
        if folder:
            paths.update(pathlib.Path(folder).glob("*.lib"))
    import shutil
    for name in ("cl", "link", "dumpbin"):
        tool = shutil.which(name)
        if tool:
            paths.add(pathlib.Path(tool))
    return {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)}


def main():
    build.OUT.mkdir(parents=True, exist_ok=True)
    report = dict(run_id=str(uuid.uuid4()), started_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  cached_inputs_verified=False, runtime_validated=False, diagnostic_pass_completed=False,
                  environment={k: os.environ.get(k) for k in ("INCLUDE", "LIB", "CL", "LINK")}, stages=[])
    destination = build.OUT / "pipeline-summary.json"
    try:
        report["input_hashes"] = snapshot()
        # Avoid accepting stale generated aliases during the initial inventory.
        for name in ("data", "entry", "runtime", "qv", "rtti"):
            (build.OUT / (name + "_alias.obj")).unlink(missing_ok=True)
        (build.OUT / 'rtti_data.obj').unlink(missing_ok=True)

        def stage(script, *args, diagnostic=False):
            log = build.OUT / ("pipeline-%02d.log" % len(report["stages"]))
            with log.open("wb") as stream:
                result = subprocess.run([sys.executable, str(build.REPO / "tools" / script), *args],
                                        stdout=stream, stderr=subprocess.STDOUT)
            report["stages"].append(dict(script=script, args=args, returncode=result.returncode, log=str(log)))
            if result.returncode and not diagnostic:
                raise RuntimeError("Stage failed: " + script + "; see " + str(log))

        stage("integration.py", "--compile-only")
        stage("integration.py", "--inventory-ownership")
        for name in ("data", "entry", "runtime", "qv", "rtti"):
            stage(name + "_aliases.py")
        response = build.OUT / "link.rsp"
        lines = response.read_text().splitlines()
        # The compile inventory contains no old aliases; insert current objects.
        insertion = next(i for i, line in enumerate(lines) if line == "libc.lib")
        lines[insertion:insertion] = ['"%s"' % (build.OUT / (name + "_alias.obj"))
                                     for name in ("data", "entry", "runtime", "qv", "rtti")]
        lines.insert(insertion, '"%s"' % (build.OUT / 'rtti_data.obj'))
        response.write_text("\n".join(lines) + "\n", encoding="ascii")
        stage("link_containers.py")
        stage("integration.py", "--diagnose-existing-link", "--diagnostic-response",
              str(build.OUT / "containers" / "link.rsp"), diagnostic=True)
        diagnostic = json.loads((build.OUT / "containers" / "partial-link-summary.json").read_text())
        report["diagnostic"] = diagnostic
        report["diagnostic_pass_completed"] = diagnostic["diagnostic_pass_completed"]
        if not report["diagnostic_pass_completed"]:
            raise RuntimeError("Diagnostic link did not complete; classification withheld")
        stage("classify_code_refs.py")
        stage("inventory_eh.py")
        stage("integration_clusters.py")
        report["compile"] = json.loads((build.OUT / "summary.json").read_text())
        report["inputs_unchanged_during_run"] = snapshot() == report["input_hashes"]
        report["generated_hashes"] = {str(p.relative_to(build.OUT)): hashlib.sha256(p.read_bytes()).hexdigest()
                                      for p in build.OUT.rglob("*") if p.is_file()
                                      and p.suffix in (".obj", ".json", ".rsp") and p != destination}
        return 0 if report["diagnostic_pass_completed"] and report["inputs_unchanged_during_run"] else 1
    except Exception as error:
        report["error"] = str(error)
        return 1
    finally:
        destination.write_text(json.dumps(report, indent=2))
        print(json.dumps({k: v for k, v in report.items() if k not in ("input_hashes", "generated_hashes", "compile")}, indent=2))


if __name__ == "__main__":
    raise SystemExit(main())
