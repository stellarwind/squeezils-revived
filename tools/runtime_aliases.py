"""Bind verified local runtime declarations to VC4 LIBC entry points."""
import hashlib
import json
import os
import pathlib
import shutil
import subprocess

import integration as build
from data_aliases import write_coff


def main():
    libc = next((pathlib.Path(p) / "libc.lib" for p in os.environ["LIB"].split(";")
                 if (pathlib.Path(p) / "libc.lib").exists()), None)
    if libc is None:
        raise ValueError("VC4 LIBC.LIB is missing")
    result = subprocess.run([shutil.which("dumpbin"), "-linkermember:1", str(libc)],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=True)
    target = "___RTDynamicCast"
    if target not in {line.split()[-1] for line in result.stdout.decode(errors="replace").splitlines() if line.split()}:
        raise ValueError("LIBC does not provide the expected RTTI entry")
    pairs = [("_SqzRTDynamicCast", target)]
    from code_pointer_bindings import bind
    pointer_pairs, pointer_evidence, diagnostics, startup = bind(libc)
    pairs.extend(pointer_pairs)
    write_coff(pairs, build.OUT / "runtime_alias.obj")
    (build.OUT / "runtime_alias.json").write_text(json.dumps(dict(
        aliases=pairs, library=str(libc), sha256=hashlib.sha256(libc.read_bytes()).hexdigest(),
        code_pointer_evidence=pointer_evidence, candidate_diagnostics=diagnostics,
        replaced_startup_records=startup,
        original_sha256=hashlib.sha256((build.REPO.parent / "SqueezilsGameFiles/SQZ.EXE").read_bytes()).hexdigest(),
        original_address="0048f060", abi="cdecl; pointer,32-bit offset,pointer,pointer,32-bit flag; pointer return"), indent=2))
    print("Verified CRT RTTI alias")


if __name__ == "__main__":
    main()
