#!/bin/bash
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
. "$ROOT/tools/flagsearch/env.sh"
cd "$HERE/obj"
MSYS2_ARG_CONV_EXCL="*" cl -nologo -GX -O2 -G3 -ML -D__ANSI_CPP__ -I"$(cygpath -m "$HERE/shim")" -I"$(cygpath -m "$HERE/include")" "$(cygpath -m "$HERE/tools/smoke.cpp")" QvLib.lib -Fesmoke.exe > smoke-build.log 2>&1 || { tail -30 smoke-build.log; exit 1; }
./smoke.exe > smoke.log 2>&1 || { tail -20 smoke.log; exit 1; }
grep 'QvLib smoke passed' smoke.log
