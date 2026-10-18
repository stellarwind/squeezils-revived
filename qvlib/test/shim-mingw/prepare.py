"""Preserve pre-standard for-scope semantics in two generated source functions."""
from pathlib import Path
t=Path(__file__).resolve().parents[1]
q=t.parent
out=t/'work/mingw'; out.mkdir(exist_ok=True)
s=(q/'src/QvFieldData.c++').read_text()
replacements={
    '    for (int i=0; name[i]; i++) {':'    int i;\n    for (i=0; name[i]; i++) {',
    '    for (int i = 0; i < fields.getLength(); i++) {':'    int i;\n    for (i = 0; i < fields.getLength(); i++) {',
}
for old,new in replacements.items():
    assert s.count(old)==1,old
    s=s.replace(old,new)
(out/'QvFieldData.c++').write_text(s)
s=(q/'src/QvLib.c++').read_text()
s=s.replace('#include "QvFieldData.c++"','#include "mingw/QvFieldData.c++"')
(t/'work/mingw-unity.cpp').write_text(s)
