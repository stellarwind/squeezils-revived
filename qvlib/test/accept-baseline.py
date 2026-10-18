"""Explicitly record saved B results; normal test runs never overwrite goldens."""
from pathlib import Path
import json, hashlib
t=Path(__file__).resolve().parent
results=json.loads((t/'out/results.json').read_text())
assert len(results)==16 and all(r['status']=='PASS' and set(r['exits'])==set('ABC') for r in results)
g=t/'golden'; g.mkdir(exist_ok=True)
manifest={}
for r in results:
    name=r['file']; data=(t/'out/B'/(name+'.txt')).read_bytes()
    assert hashlib.sha256(data).hexdigest()==r['sha256']['B']
    if name=='NUT2.WRL':
        (g/(name+'.sha256')).write_text(r['sha256']['B']+'  NUT2.WRL.txt\n')
    else: (g/(name+'.txt')).write_bytes(data)
    manifest[name]={'exit':r['exits']['B'],'sha256':r['sha256']['B']}
(g/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Recorded 15 B golden dumps and the NUT2 dump hash (no game dump content)')
