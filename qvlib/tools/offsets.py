"""Batch instruction offsets, using the same objdump decoder as dis_norm."""
import bisect
import json
from pathlib import Path
import re
import subprocess
import sys

here=Path(__file__).resolve().parents[1]
path=here/('obj/corpus-parts.json' if '--parts' in sys.argv else 'obj/corpus.json')
c=json.loads(path.read_text())
entries=[]
for f in c['functions'].values(): entries.append(f)
for e in c['exe'].values(): entries.extend(e['variants'])
blob=bytearray(); starts=[]
for f in entries:
    starts.append(len(blob)); blob.extend(bytes.fromhex(f['masked'])); blob.extend(b'\xcc'*32)
temp=here/'obj/offsets.bin';temp.write_bytes(blob)
dump=subprocess.check_output(['D:/Decomp/mingw32/bin/objdump.exe','-D','-b','binary','-m','i386','-M','intel',str(temp)],text=True)
offsets=[[] for _ in entries]
for line in dump.splitlines():
    m=re.match(r'\s*([0-9a-f]+):\t[0-9a-f ]+\t(.+?)\s*$',line)
    if not m: continue
    a=int(m[1],16);i=bisect.bisect_right(starts,a)-1
    if 0<=i<len(entries) and a-starts[i]<len(bytes.fromhex(entries[i]['masked'])):
        offsets[i].append(a-starts[i])
for f,off in zip(entries,offsets):
    assert len(off)==len(f['norm']), (len(off),len(f['norm']))
    f['offsets']=off
path.write_text(json.dumps(c,separators=(',',':')))
temp.unlink()
print('Added instruction offsets to',len(entries),'bodies')
