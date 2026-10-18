#!/bin/bash
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
. "$ROOT/tools/flagsearch/env.sh"
mkdir -p "$HERE/obj"
cd "$HERE/obj"
MSYS2_ARG_CONV_EXCL="*" cl -nologo -c -GX -Gy -O2 -G3 -ML -D__ANSI_CPP__ -I"$(cygpath -m "$HERE/shim")" -I"$(cygpath -m "$HERE/include")" -I"$(cygpath -m "$HERE/src")" -Tp"$(cygpath -m "$HERE/src/QvLib.c++")" -FoQvLib.obj > build.log 2>&1 || { tail -60 build.log; exit 1; }
echo "Built qvlib/obj/QvLib.obj"
MSYS2_ARG_CONV_EXCL="*" lib -nologo -out:QvLib.lib QvLib.obj > library.log 2>&1 || { tail -20 library.log; exit 1; }
if [ "${1:-}" = "--parts" ]; then
  mkdir -p parts
  cd parts
  for source in "$HERE"/src/*.c++; do
    name="$(basename "$source" .c++)"
    case "$name" in Qv|QvLib|QvNodes|QvFields|QvErrors) continue;; esac
    case "$name" in
      QvLists|QvTraverse|QvWWWInline)
        printf '#include <QvNode.h>\n#include <QvChildList.h>\n#include <QvInput.h>\n#include <QvUnknownNode.h>\n#include <QvSFString.h>\n' > "$name.cpp";;
      *) : > "$name.cpp";;
    esac
    printf '#include "%s.c++"\n' "$name" >> "$name.cpp"
  done
  MSYS2_ARG_CONV_EXCL="*" cl -nologo -c -GX -Gy -O2 -G3 -ML -D__ANSI_CPP__ -I"$(cygpath -m "$HERE/shim")" -I"$(cygpath -m "$HERE/include")" -I"$(cygpath -m "$HERE/src")" ./*.cpp > build.log 2>&1 || { tail -60 build.log; exit 1; }
  objects=(./*.obj)
  echo "Built ${#objects[@]} separate source objects in qvlib/obj/parts"
fi
