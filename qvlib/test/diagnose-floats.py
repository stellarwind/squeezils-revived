"""Keep raw CRT formatting evidence and verify the parsed IEEE values agree."""
from pathlib import Path
import subprocess, os, re, difflib, json
t=Path(__file__).resolve().parent
e=os.environ.copy(); e['QV_FLOAT_BITS']='1'
out={}
for v in 'ABC':
    r=subprocess.run([str(t/'work'/f'{v}.exe'),str(t/'inputs/float_edges.wrl')],env=e,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,check=True)
    out[v]=r.stdout.replace(b'\r\n',b'\n').decode()
    (t/'out'/f'float-bits-{v}.txt').write_text(out[v])
bits={v:re.findall(r'\{0x([0-9a-f]{8})\}',s) for v,s in out.items()}
assert bits['A']==bits['B']==bits['C']
assert '80000000' in bits['A']
d=t/'diagnostics';d.mkdir(exist_ok=True)
(d/'float-format.diff').write_text(''.join(difflib.unified_diff(out['A'].splitlines(True),out['B'].splitlines(True),fromfile='VC4 CRT (raw %.9g + IEEE bits)',tofile='MinGW CRT (raw %.9g + IEEE bits)')))
(d/'float-bits.json').write_text(json.dumps({'all_variants_equal':True,'float_values_checked':len(bits['A']),'bits':bits['A']},indent=2)+'\n')
print('Float diagnostics: all',len(bits['A']),'IEEE float values identical in A/B/C, including negative zero')
