"""Validate compiled COFF objects and compiled resources against the input PE."""
import json,struct,pathlib,hashlib
from extract import HERE,ROOT,u16,u32
m=json.loads((HERE/'manifest.json').read_text());pe=(ROOT/'SqueezilsGameFiles/SQZ.EXE').read_bytes();assert hashlib.sha256(pe).hexdigest()==m['sha256']
b=(HERE/'data.obj').read_bytes();ns=u16(b,2);symoff=u32(b,8);count=u32(b,12);strtab=symoff+count*18
sections=[]
for i in range(ns):
 p=20+i*40;sections.append((u32(b,p+16),u32(b,p+20),u32(b,p+24),u16(b,p+32)))
syms={};i=0
while i<count:
 p=symoff+18*i;n=b[p:p+8]
 if n[:4]==bytes(4):
  off=strtab+u32(n,4);name=b[off:b.index(0,off)].decode()
 else:name=n.rstrip(b'\0').decode()
 val,section,typ,storage,aux=struct.unpack_from('<IhHBB',b,p+8);syms[name]=(val,section)
 if name.startswith('?'):syms['_'+name[1:].split('@@')[0]]=(val,section)
 i+=aux+1
relsets=[{u32(b,rel+j*10) for j in range(nrel)} for size,raw,rel,nrel in sections]
checked=0;ptrcount=0
for o in m['objects']:
 if o['skip']:continue
 name=o['name'];val,si=syms['_'+name];size,raw,rel,nrel=sections[si-1];n=o['size'];a=o['address'];s=next(s for s in m['sections'] if s['start']<=a<s['start']+s['size']);off=a-s['start'];disk=max(0,min(n,s['raw']-off));expected=pe[s['off']+off:s['off']+off+disk]+bytes(n-disk);actual=b[raw+val:raw+val+n] if raw else bytes(n)
 assert len(actual)==n,(name,'size')
 mask=set()
 for p in o['pointers']:mask.update(range(p,p+4))
 assert all(x==y for j,(x,y) in enumerate(zip(actual,expected)) if j not in mask),(name,'bytes')
 actualrel={r-val for r in relsets[si-1] if val<=r<val+n}
 assert actualrel==set(o['pointers']),(name,'relocations',actualrel,o['pointers'])
 checked+=1;ptrcount+=len(o['pointers'])
assert (checked,sum(o['size'] for o in m['objects'] if not o['skip']),ptrcount)==(7885,255153,7032), 'coverage changed'
# Parse Win32 .res records; RC can reorder and assign icon IDs.
b=(HERE/'resources.res').read_bytes();p=0;resources={}
def ident(q):
 if u16(b,q)==65535:return u16(b,q+2),q+4
 chars=[]
 while u16(b,q):chars.append(chr(u16(b,q)));q+=2
 return ''.join(chars),q+2
while p+8<=len(b):
 size,hs=struct.unpack_from('<II',b,p);assert hs>=32,('invalid resource header',p,size,hs);typ,q=ident(p+8);name,q=ident(q);q=(q+3)&~3;lang=u16(b,q+6);payload=b[p+hs:p+hs+size]
 if size:resources[(typ,name,lang)]=payload
 p=(p+hs+size+3)&~3
records=json.loads((HERE/'resources.json').read_text())
for r in records:
 key=tuple(r['path']);expected=(HERE/'resources'/r['file']).read_bytes();actual=resources.pop(key)
 assert actual==expected,('resource mismatch',key,len(actual),len(expected),next((i for i,(a,z) in enumerate(zip(actual,expected)) if a!=z),None))
assert len(records)==19, 'resource coverage changed'
assert not resources,('extra resources',list(resources))
print('PASS: %d objects, %d bytes, %d symbolic relocations; %d resources, %d payload bytes'%(checked,sum(o['size'] for o in m['objects'] if not o['skip']),ptrcount,len(records),sum(r['size'] for r in records)))
