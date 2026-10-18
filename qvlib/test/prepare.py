"""Prepare local copies and wrappers; never write the source inputs."""
from pathlib import Path
import hashlib, shutil, json
t=Path(__file__).resolve().parent
q=t.parent
root=q.parents[1]
work=t/'work'; work.mkdir(exist_ok=True)
(work/'inputs').mkdir(exist_ok=True)
for row in (q/'upstream.sha256').read_text().splitlines():
    digest,name=row.split('  ',1)
    assert hashlib.sha256((q/name).read_bytes()).hexdigest()==digest,name
samples=root/'archive/qvlib_download/src/QvLib-1.0/test'
sources={name:samples/name for name in ('AllNodes.wrl','Sample.wrl')}
sources['NUT2.WRL']=root/'SqueezilsGameFiles/SQZ/GOODIES/NUT2.WRL'
manifest={}
previous=json.loads((t/'sources.json').read_text()) if (t/'sources.json').exists() else None
for name,source in sources.items():
    fixture=t/'inputs/sgi'/name
    actual=source if source.exists() else fixture
    manifest[name]={'source':source.relative_to(root).as_posix(),'sha256':hashlib.sha256(actual.read_bytes()).hexdigest()}
    if previous: assert previous[name]==manifest[name], 'Source changed: '+name
    shutil.copyfile(actual,work/'inputs'/name)
    if name!='NUT2.WRL':
        fixture.parent.mkdir(exist_ok=True)
        if fixture.exists(): assert fixture.read_bytes()==actual.read_bytes()
        else: shutil.copyfile(actual,fixture)
if not previous: (t/'sources.json').write_text(json.dumps(manifest,indent=2)+'\n')
s=(q/'src/QvTraverse.c++').read_text()
old='#define ANNOUNCE(className) announce(QV__QUOTE(className))'
assert s.count(old)==1
(work/'QvTraverse-silent.inc').write_text(s.replace(old,'#define ANNOUNCE(className) /* silent */'))
(work/'Traverse.cpp').write_text('#include <QvNode.h>\n#include <QvChildList.h>\n#include <QvInput.h>\n#include <QvUnknownNode.h>\n#include <QvSFString.h>\n#include "QvTraverse-silent.inc"\n')
print('Prepared 3 byte-identical input copies; all vendor hashes verified')
