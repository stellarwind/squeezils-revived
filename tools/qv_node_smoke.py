"""Generate/run one bounded lifecycle sweep against the emitted alias object."""
import json
import pathlib
import re
import subprocess
import integration as build


def main():
    registry = json.loads((build.REPO / 'tools/qv_node_registry.json').read_text())
    declarations, checks = [], []
    ctors = dtors = 0
    for alias, d in registry.items():
        ctor = re.fullmatch(r'\?\?0(QvNodeC[0-9a-f]{6})@@QAE@XZ', alias)
        dtor = re.fullmatch(r'\?\?1(QvNode[0-9a-f]{6})@@UAE@XZ', alias)
        if not ctor and not dtor:
            continue
        local = (ctor or dtor)[1]
        canonical = d['canonical_class']
        if canonical == 'QvNode':
            continue
        declarations.append('class %s { public: char storage[sizeof(%s)-4]; %s virtual ~%s(); };\n'
                            'typedef char Size_%s[sizeof(%s)==sizeof(%s) ? 1 : -1];'
                            % (local, canonical, (local + '();') if ctor else '', local, local, local, canonical))
        if ctor:
            ctors += 1
            checks.append('''{
    %s *canonical = new %s;
    QvFieldData *fields = ((QvNode *)canonical)->getFieldData();
    int count = fields->getNumFields();
    %s *local = new %s;
    QvNode *view = (QvNode *)local;
    if (view->getFieldData() != fields || fields->getNumFields() != count) return 1;
    for (int i=0; i<count; i++) {
        char *field = (char *)fields->getField(view, i);
        if (field < (char *)local + 24 || field >= (char *)local + sizeof(%s)) return 2;
    }
    local->%s::~%s(); operator delete((void *)local);
    delete canonical;
}''' % (canonical, canonical, local, local, local, local, local))
        else:
            dtors += 1
            checks.append('''{
    %s *canonical = new %s;
    %s *view = (%s *)canonical;
    view->%s::~%s(); operator delete((void *)canonical);
}''' % (canonical, canonical, local, local, local, local))
    source = '''#include <QvDB.h>
#include <QvNodes.h>
#include <QvUnknownNode.h>
#include <QvFieldData.h>
#include <new.h>
#include <stdio.h>
''' + '\n'.join(declarations) + '\nint main() { QvDB::init();\n' + '\n'.join(checks) + '''
    QvSpotLight spot;
    QvPointLight point;
    QvMatrixTransform matrix;
    if (!spot.on.value || spot.intensity.value != 1 || !point.on.value || point.intensity.value != 1) return 3;
    for (int i=0; i<4; i++) for (int j=0; j<4; j++)
        if (matrix.matrix.value[i][j] != (i==j ? 1 : 0)) return 4;
''' + 'printf("Qv node smoke passed: %d ctor aliases, %d dtor aliases, shared field tables, functional defaults\\n");\nreturn 0; }\n' % (ctors, dtors)
    (build.OUT / 'qv-node-smoke.cpp').write_text(source)
    cmd = ['cl', '-nologo', '-GX', '-O2', '-G3', '-ML', '-D__ANSI_CPP__',
           '-I' + str(build.REPO / 'qvlib/shim'), '-I' + str(build.REPO / 'qvlib/include'),
           'qv-node-smoke.cpp', 'qv_alias.obj', str(build.REPO / 'qvlib/obj/QvLib.lib'), '-Feqv-node-smoke.exe']
    with (build.OUT / 'qv-node-smoke-build.log').open('wb') as log:
        result = subprocess.run(cmd, cwd=build.OUT, stdout=log, stderr=subprocess.STDOUT)
    if result.returncode:
        print('\n'.join((build.OUT / 'qv-node-smoke-build.log').read_text().splitlines()[-15:]))
        return result.returncode
    result = subprocess.run([str(build.OUT / 'qv-node-smoke.exe')], cwd=build.OUT, capture_output=True)
    (build.OUT / 'qv-node-smoke.log').write_bytes(result.stdout + result.stderr)
    print(result.stdout.decode(errors='replace').strip())
    return result.returncode


if __name__ == '__main__':
    raise SystemExit(main())
