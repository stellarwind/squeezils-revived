#!/bin/bash
set -eu
T="$(cd "$(dirname "$0")" && pwd)"
Q="$(cd "$T/.." && pwd)"
ROOT="$(cd "$Q/../.." && pwd)"
MODE="${1:-}"
case "$MODE" in ''|--a-only|--run-only|--bootstrap) ;; *) echo 'Usage: run.sh [--a-only|--run-only|--bootstrap]'; exit 2;; esac
failed() { tail -12 "$1" | cut -c1-90; exit 1; }
python "$T/prepare.py"
. "$ROOT/tools/flagsearch/env.sh"
if [ "$MODE" != --run-only ]; then
  if [ "${QV_REUSE_VC:-0}" != 1 ]; then
    if [ "${QV_REUSE_LIBS:-0}" != 1 ]; then
      bash "$Q/build.sh" --parts > "$T/work/vc-library.log" 2>&1 || failed "$T/work/vc-library.log"
    fi
    cd "$T/work"
    M="$(cygpath -m "$Q")"
    MSYS2_ARG_CONV_EXCL="*" cl -nologo -c -GX -Gy -O2 -G3 -ML -D__ANSI_CPP__ -I"$M/shim" -I"$M/include" -I"$M/src" "$M/test/dump.cpp" "$M/test/work/Traverse.cpp" > vc-dump.log 2>&1 || failed vc-dump.log
    PARTS=()
    for o in "$Q"/obj/parts/*.obj; do case "$o" in */QvTraverse.obj) ;; *) PARTS+=("$(cygpath -m "$o")");; esac; done
    MSYS2_ARG_CONV_EXCL="*" link -nologo -subsystem:console -opt:ref -out:A.exe dump.obj Traverse.obj "${PARTS[@]}" > link-A.log 2>&1 || failed link-A.log
    if [ "$MODE" != --a-only ]; then
      MSYS2_ARG_CONV_EXCL="*" link -nologo -subsystem:console -opt:ref -out:C.exe dump.obj "$M/obj/QvLib.lib" > link-C.log 2>&1 || failed link-C.log
    fi
  fi
  if [ "$MODE" != --a-only ]; then
    cd "$T/work"
    python "$T/shim-mingw/prepare.py"
    /d/Decomp/mingw32/bin/g++ -O0 -std=gnu++98 -fpermissive -D__ANSI_CPP__ -I"$T/shim-mingw" -I"$Q/include" -I"$Q/src" -x c++ "$T/work/mingw-unity.cpp" "$T/dump.cpp" -o B.exe > mingw-build.log 2>&1 || failed mingw-build.log
  fi
fi
python "$T/verify.py" "$MODE"
