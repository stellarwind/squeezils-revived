"""Make a local harness adapter; shared project tools remain untouched."""
from pathlib import Path
here=Path(__file__).resolve().parents[1]
root=here.parents[1]
out=here/'obj/harness'
out.mkdir(exist_ok=True)
shared=root/'tools/flagsearch'
script=(shared/'flagsearch.pl').read_text()
def posix(p):
    s=p.as_posix()
    return '/'+s[0].lower()+s[2:] if len(s)>1 and s[1]==':' else s
for old,new in [('$here/../../Exported/SQZ.EXE',posix(root/'Exported/SQZ.EXE')),
                ('$here/../../nm/fns_all.txt',posix(root/'nm/fns_all.txt')),
                ('$here/env.sh',posix(shared/'env.sh')),
                ('$cls == 2 && $type == 0x20','($cls == 2 || $cls == 3) && $type == 0x20')]:
    assert script.count(old)==1,old
    script=script.replace(old,new)
(out/'flagsearch.pl').write_text(script)
(out/'match.sh').write_text((shared/'match.sh').read_text())
print('Prepared harness with static-function support (no scoring changes)')
