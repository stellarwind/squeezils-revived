#!/bin/bash
set -eu
source tools/flagsearch/env.sh
ROOT="$PWD"
cd Repo/build/integration
MSYS2_ARG_CONV_EXCL="*" cl -nologo -GX -O2 -G3 -ML -D__ANSI_CPP__ -I"$(cygpath -m "$ROOT/Repo/qvlib/shim")" -I"$(cygpath -m "$ROOT/Repo/qvlib/include")" "$(cygpath -m "$ROOT/Repo/tools/qv_input_smoke.cpp")" qv_alias.obj "$(cygpath -m "$ROOT/Repo/qvlib/obj/QvLib.lib")" -Feqv-input-smoke.exe > qv-input-smoke-build.log 2>&1 || { tail -15 qv-input-smoke-build.log; exit 1; }
./qv-input-smoke.exe
