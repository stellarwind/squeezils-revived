"""Run each fixture once per variant; report the first differing canonical line."""
from pathlib import Path
import subprocess, sys, hashlib, json, difflib, os
t=Path(__file__).resolve().parent
mode=sys.argv[1] if len(sys.argv)>1 else ''
variants=['A'] if mode=='--a-only' else ['A','B','C']
inputs=sorted((t/'inputs').rglob('*.wrl'))+[t/'work/inputs/NUT2.WRL']
invalid={'bad_header.wrl','bad_number.wrl','mf_missing_bracket.wrl','unknown_field.wrl','unknown_no_fields.wrl','unterminated_brace.wrl','use_before_def.wrl'}
baseline=json.loads((t/'golden/manifest.json').read_text()) if (t/'golden/manifest.json').exists() else {}
def digest(b): return hashlib.sha256(b).hexdigest()
def firstdiff(a,b):
    aa=a.decode(errors='replace').splitlines(); bb=b.decode(errors='replace').splitlines()
    for i in range(max(len(aa),len(bb))):
        x=aa[i] if i<len(aa) else '<EOF>'; y=bb[i] if i<len(bb) else '<EOF>'
        if x!=y: return f'line {i+1}: {x!r} != {y!r}'
    return 'byte difference'
results=[]; failures=0
if (t/'out').exists():
    for stale in (t/'out').glob('*.diff'): stale.unlink()
for f in inputs:
    outputs={}; exits={}
    for v in variants:
        out=t/'out'/v; out.mkdir(parents=True,exist_ok=True)
        env=os.environ.copy()
        env.pop('QV_FLOAT_BITS',None); env.pop('QV_RAW_FLOATS',None)
        r=subprocess.run([str(t/'work'/f'{v}.exe'),str(f)],env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=30)
        data=r.stdout.replace(b'\r\n',b'\n')
        (out/(f.name+'.txt')).write_bytes(data)
        outputs[v]=data; exits[v]=r.returncode
    issues=[]
    # Empty input behavior is recorded from the reference, not assumed here.
    expected=1 if f.name in invalid else 0
    if f.name!='empty.wrl' and any(x!=expected for x in exits.values()): issues.append('unexpected exit '+str(exits))
    if any(x not in (0,1) for x in exits.values()): issues.append('crash or dumper failure')
    for v in variants[1:]:
        if outputs['A']!=outputs[v] or exits['A']!=exits[v]:
            issues.append('A/'+v+' '+firstdiff(outputs['A'],outputs[v]))
            (t/'out'/f'{f.name}.A-{v}.diff').write_text(''.join(difflib.unified_diff(outputs['A'].decode(errors='replace').splitlines(True),outputs[v].decode(errors='replace').splitlines(True),fromfile='A',tofile=v)))
    golden=t/'golden'/(f.name+'.txt')
    if mode!='--bootstrap':
        if f.name not in baseline or exits['A']!=baseline[f.name]['exit']: issues.append('A exit differs from B golden')
        if f.name in baseline and digest(outputs['A'])!=baseline[f.name]['sha256']: issues.append('A hash differs from B golden manifest')
        if f.name=='NUT2.WRL':
            hashfile=t/'golden/NUT2.WRL.sha256'
            if not hashfile.exists() or digest(outputs['A'])!=hashfile.read_text().split()[0]: issues.append('game dump hash differs from golden')
        elif not golden.exists(): issues.append('golden missing')
        elif outputs['A']!=golden.read_bytes().replace(b'\r\n',b'\n'): issues.append('A/golden '+firstdiff(outputs['A'],golden.read_bytes().replace(b'\r\n',b'\n')))
    status='FAIL' if issues else 'PASS'
    failures+=bool(issues)
    print(status+' '+f.name+(' '+issues[0] if issues else ''))
    results.append({'file':f.name,'status':status,'exits':exits,'issues':issues,'sha256':{v:digest(d) for v,d in outputs.items()}})
(t/'out'/('results-A.json' if variants==['A'] else 'results.json')).write_text(json.dumps(results,indent=2)+'\n')
sys.exit(bool(failures))
