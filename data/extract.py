"""Regenerate all artifacts using only the PE, Ghidra snapshot and RTTI map."""
import argparse, collections, hashlib, json, pathlib, re, struct, subprocess
HERE=pathlib.Path(__file__).resolve().parent
ROOT=HERE.parents[1]
def u16(b,o): return struct.unpack_from('<H',b,o)[0]
def u32(b,o): return struct.unpack_from('<I',b,o)[0]
def c_character(value, quote='"'):
 """Keep printable ASCII readable; fixed-width octal avoids escape ambiguity."""
 escapes={9:r'\t',10:r'\n',13:r'\r',92:r'\\'}
 if value in escapes: return escapes[value]
 if value==ord(quote): return '\\'+quote
 return chr(value) if 32<=value<127 else '\\%03o'%value

def scalar_literal(raw, typ):
 if typ in ('float','double'):
  import math
  value=struct.unpack('<f' if typ=='float' else '<d',raw)[0]
  if not math.isfinite(value): return None
  return repr(value)+('f' if typ=='float' else '')
 signed=typ in ('int','short','long','char')
 value=int.from_bytes(raw,'little',signed=signed)
 if signed and len(raw)==4 and value==-2147483648: return '(-2147483647 - 1)'
 return str(value)+('U' if typ.startswith('unsigned') else '')

def typed_value(address, raw, ghidra_type, accesses):
 """Infer scalars/arrays only from defined types or matching memory operands."""
 aliases={'uint':'unsigned int','ulong':'unsigned long','ushort':'unsigned short',
          'word':'unsigned short','dword':'unsigned int','byte':'unsigned char'}
 sizes={'float':4,'double':8,'int':4,'unsigned int':4,'long':4,'unsigned long':4,
        'short':2,'unsigned short':2,'char':1,'unsigned char':1}
 t=ghidra_type.lower(); match=re.fullmatch(r'(.+?)\[(\d+)\]',t)
 if match:t=match[1]
 t=aliases.get(t,t)
 if t not in sizes:
  t=None
  # A full-width FPU read is stronger evidence than a bitwise MOV of a float.
  for width, candidate in ((8,'double'),(4,'float')):
   if len(raw)%width==0 and all(('F',width) in accesses.get(address+i,set()) for i in range(0,len(raw),width)):
    t=candidate;break
  if t is None:
   for width,candidate in ((4,'unsigned int'),(2,'unsigned short')):
    if len(raw)%width==0 and all(('I',width) in accesses.get(address+i,set()) for i in range(0,len(raw),width)):
     # Conflicting floating/integer widths are deliberately left untyped.
     if not any(kind=='F' for i in range(len(raw)) for kind,_ in accesses.get(address+i,set())): t=candidate
     break
 if t not in sizes or len(raw)%sizes[t]: return None
 width=sizes[t];values=[scalar_literal(raw[i:i+width],t) for i in range(0,len(raw),width)]
 if any(v is None for v in values): return None
 return t,values

def generate():
 b=(ROOT/'SqueezilsGameFiles/SQZ.EXE').read_bytes(); pe=u32(b,60); opt=pe+24; base=u32(b,opt+28)
 secs=[]
 for i in range(u16(b,pe+6)):
  p=opt+u16(b,pe+20)+40*i; name=b[p:p+8].rstrip(b'\0').decode(); vs,va,rs,rp=struct.unpack_from('<IIII',b,p+8); secs.append(dict(name=name,start=base+va,size=vs,raw=rs,off=rp))
 def sec(a): return next((s for s in secs if s['start']<=a<s['start']+max(s['size'],s['raw'])),None)
 def read(a,n):
  s=sec(a); o=a-s['start']; disk=max(0,min(n,s['raw']-o)); return b[s['off']+o:s['off']+o+disk]+bytes(n-disk)
 ranges=[s for s in secs if s['name'] in ('.rdata','.data')]
 def indata(a): return any(s['start']<=a<s['start']+s['size'] for s in ranges)
 reloc={}; rr=next(s for s in secs if s['name']=='.reloc'); p=rr['off']; end=p+rr['raw']
 while p+8<=end:
  page,size=struct.unpack_from('<II',b,p)
  if size<8: break
  for q in range(p+8,p+size,2):
   v=u16(b,q); a=base+page+(v&4095)
   if v>>12==3 and indata(a): reloc[a]=u32(read(a,4),0)
   elif v>>12 not in (0,3): raise ValueError('unsupported relocation')
  p+=size
 labels={}; defs={}; refs=collections.defaultdict(set); accesses=collections.defaultdict(set)
 for line in (HERE/'snapshot.tsv').read_text().splitlines():
  x=line.split('\t'); a=int(x[1],16)
  if x[0] in ('F','I'): accesses[a].add((x[0],int(x[2])))
  elif x[0]=='S': labels[a]=x[2]
  elif x[0]=='D' and indata(a): defs[a]=(int(x[2]),x[3])
  elif x[0]=='R' and indata(int(x[2],16)): refs[int(x[2],16)].add(a)
 bounds={s['start']+s['raw'] for s in ranges if s['raw']<s['size']}|{s['start'] for s in ranges}|{s['start']+s['size'] for s in ranges}
 bounds|={a for a in labels if indata(a)}|set(defs)|set(refs)
 bounds|={v for v in reloc.values() if indata(v)}
 for a,(n,t) in defs.items():
  if indata(a+n-1): bounds.add(a+n)
 # Relocations must remain whole even when an analyzer places a boundary inside one.
 bounds={a for a in bounds if not any(a-k in reloc for k in (1,2,3))}
 objects=[]
 for s in ranges:
  bs=sorted(a for a in bounds if s['start']<=a<=s['start']+s['size'])
  for a,z in zip(bs,bs[1:]): objects.append(dict(address=a,size=z-a,section=s['name']))
 starts=[o['address'] for o in objects]
 import bisect
 def owner(a):
  i=bisect.bisect_right(starts,a)-1
  return objects[i] if i>=0 and a<objects[i]['address']+objects[i]['size'] else None
 # Attribute direct and transitive incoming references; shared game/library objects stay.
 users={o['address']:set() for o in objects}; edges=[]
 for target,srcs in refs.items():
  dst=owner(target)
  if not dst: continue
  for src in srcs:
   so=owner(src)
   if so: edges.append((so['address'],dst['address']))
   elif 0x401000<=src<0x49d000: users[dst['address']].add('game' if src<0x47e600 else 'qv' if src<0x48e000 else 'crt')
 for src,target in reloc.items():
  so,to=owner(src),owner(target)
  if so and to: edges.append((so['address'],to['address']))
 changed=True
 while changed:
  changed=False
  for a,z in edges:
   n=len(users[z]); users[z]|=users[a]; changed|=len(users[z])!=n
 vtables=set()
 for line in (ROOT/'tools/rtti.tsv').read_text().splitlines():
  x=line.split('\t')
  if len(x)>1:
   try: vtables.add(int(x[1],16))
   except ValueError: pass
 # Ghidra's MSVC RTTI analyzer supplies exact defined-data extents.
 skipranges=[]
 for a,(n,t) in defs.items():
  text=(t+' '+labels.get(a,'')).lower()
  if ('string' in t.lower() and read(a,n).startswith((b'.?AV',b'.?AU'))) or any(k in text for k in ('rtti','vftable','type_descriptor','typedescriptor','vtable')): skipranges.append((a-8 if read(a,min(n,4)).startswith(b'.?A') else a,a+n,'compiler RTTI/vtable'))
 for v in vtables:
  if not indata(v): continue
  end=v
  while end in reloc and 0x401000<=reloc[end]<0x49d000: end+=4
  skipranges.append((v-4,end,'CLASS_MAP RTTI/vtable'))
 for o in objects:
  a=o['address']; reason=next((r for x,z,r in skipranges if x<=a<z),None)
  if not reason and users[a] and users[a]<={'crt'}: reason='CRT-only incoming references'
  if not reason and users[a] and users[a]<={'qv','crt'} and 'qv' in users[a]: reason='QvLib/runtime-only incoming references'
  o['skip']=reason; o['users']=sorted(users[a]); o['ghidra']=labels.get(a,''); o['type']=defs.get(a,(0,''))[1]
  raw=read(a,o['size']); o['name']=('s_' if 'string' in o['type'].lower() and not any(a<=p<a+o['size'] for p in reloc) else 'g_')+format(a,'06X')
 funcs={v for v in reloc.values() if 0x401000<=v<0x49d000}; ext={}
 def expr(v):
  if v in funcs: return '(const void *)&fn_%06X'%v
  ob=owner(v)
  if ob:
   if ob['skip']: ext[ob['name']]=True
   return '(const void *)((const unsigned char *)&%s + %d)'%(ob['name'],v-ob['address'])
  name='external_%06X'%v; ext[name]=True; return '(const void *)&'+name
 h=['/* Generated by extract.py; do not edit. */','#ifndef SQZ_DATA_H','#define SQZ_DATA_H','#pragma pack(push, 1)']
 c=['/* Generated by extract.py; do not edit. */','#include "data.h"']; checks=[]
 for v in sorted(funcs): h.append('extern "C" void fn_%06X(void);'%v)
 for o in objects:
  if o['skip']: continue
  a,n,name=o['address'],o['size'],o['name']; raw=read(a,n); ptrs=sorted(p for p in reloc if a<=p<a+n)
  o['pointers']=[p-a for p in ptrs]
  typed=typed_value(a,raw,o['type'],accesses) if not ptrs and not name.startswith('s_') else None
  # Keep original object boundaries, but type known fields inside mixed objects.
  fields_by_offset={p-a:(4,'const void *',expr(reloc[p])) for p in ptrs}
  if not name.startswith('s_') and typed is None:
   for address in sorted(k for k in accesses if a<=k<a+n):
    offset=address-a
    for width in (8,4,2):
     if offset+width>n or any(offset<off+size and off<offset+width for off,(size,_,_) in fields_by_offset.items()): continue
     field=typed_value(address,raw[offset:offset+width],defs.get(address,(0,''))[1],accesses)
     if field and len(field[1])==1:
      fields_by_offset[offset]=(width,field[0],field[1][0]);break
  if ptrs or fields_by_offset:
   fields=[]; vals=[]; pos=0
   for idx,(offset,(size,field_type,value)) in enumerate(sorted(fields_by_offset.items())):
    if offset>pos:
     fields.append('unsigned char b%d[%d];'%(idx,offset-pos));vals.append('{'+', '.join(str(v) for v in raw[pos:offset])+'}')
    fields.append('%s v%d;'%(field_type,idx));vals.append(value);pos=offset+size
   if pos<n:
    fields.append('unsigned char tail[%d];'%(n-pos));vals.append('{'+', '.join(str(v) for v in raw[pos:])+'}')
   categories={t for _,t,_ in fields_by_offset.values()}
   o['emission']='pointer' if ptrs else 'float' if categories & {'float','double'} else 'int'
   o['fields']=[dict(offset=off,size=size,type=t) for off,(size,t,_) in sorted(fields_by_offset.items())]
   typ='T_'+name;h.append('struct '+typ+' {\n    '+'\n    '.join(fields)+'\n};')
   decl=typ+' '+name;init='{\n    '+',\n    '.join(vals)+'\n}'
  elif name.startswith('s_'):
   o['emission']='string';decl='const char %s[%d]'%(name,n)
   if raw.endswith(b'\0'): init='"'+''.join(c_character(v) for v in raw[:-1])+'"'
   else: init='{'+', '.join("'"+c_character(v,"'")+"'" for v in raw)+'}'
  elif typed:
   typ,values=typed;o['cpp_type']=typ;o['emission']='float' if typ in ('float','double') else 'int'
   decl=typ+' '+name+('[%d]'%len(values) if len(values)>1 else '')
   init='{'+', '.join(values)+'}' if len(values)>1 else values[0]
  else:
   o['emission']='bytes';decl='unsigned char %s[%d]'%(name,n);init='{'+', '.join(str(v) for v in raw)+'}'
  h.append('extern '+decl+';'); s=sec(a); bss=a>=s['start']+s['raw']; o['bss']=bss
  c.append(decl+(';' if bss else ' = '+init+';'))
  checks.append(o)
 for name in sorted(ext): h.append('extern unsigned char '+name+'[];')
 h+=['#pragma pack(pop)','#endif']; (HERE/'data.h').write_text('\n'.join(h)+'\n'); (HERE/'data.cpp').write_text('\n'.join(c)+'\n')
 (HERE/'function_targets.tsv').write_text('address\tghidra_name\n'+''.join('%08X\t%s\n'%(v,labels.get(v) or 'fn_%06X'%v) for v in sorted(funcs)))
 (HERE/'ambiguous.tsv').write_text('address\tsize\treason\n'+''.join('%08X\t%d\tno classified code-reference root; retained conservatively\n'%(o['address'],o['size']) for o in objects if not o['skip'] and not o['users']))
 (HERE/'manifest.json').write_text(json.dumps(dict(sha256=hashlib.sha256(b).hexdigest(),sections=secs,objects=objects,relocations=reloc),indent=2))
 (HERE/'skipped.tsv').write_text('address\tsize\treason\n'+''.join('%08X\t%d\t%s\n'%(o['address'],o['size'],o['skip']) for o in objects if o['skip'])+''.join('%08X\t%d\timport table section (.idata)\n'%(s['start'],s['size']) for s in secs if s['name']=='.idata'))
 resources(b,base,secs,read)
 print('object types',dict(sorted(collections.Counter(o['emission'] for o in checks).items())))
 print('emitted',sum(o['size'] for o in checks),'skipped',sum(o['size'] for o in objects if o['skip']),'objects',len(checks),'function targets',len(funcs))
def resources(b,base,secs,read):
 s=next(s for s in secs if s['name']=='.rsrc'); root=s['start']; out=HERE/'resources'; out.mkdir(exist_ok=True); lines=['/* Generated resource syntax; binary payloads retained for verification. */']; records=[]
 def ident(v):
  if v&0x80000000:
   a=root+(v&0x7fffffff); n=u16(read(a,2),0); return read(a+2,2*n).decode('utf-16le')
  return v
 def token(v): return str(v) if isinstance(v,int) else v
 def walk(off,path):
  h=read(root+off,16); n=u16(h,12)+u16(h,14)
  for i in range(n):
   name,target=struct.unpack('<II',read(root+off+16+8*i,8)); p=path+[ident(name)]
   if target&0x80000000: walk(target&0x7fffffff,p)
   else:
    rva,size,cp,res=struct.unpack('<IIII',read(root+target,16)); data=read(base+rva,size); file='resource_%03d.bin'%len(records); (out/file).write_bytes(data)
    typ,num,lang=p
    rcfile=file
    if typ==3:
     # RC recognizes numeric RT_ICON and expects a complete ICO container.
     width,height=struct.unpack_from('<ii',data,4); height//=2; bits=u16(data,14)
     ico=struct.pack('<HHH',0,1,1)+struct.pack('<BBBBHHII',width%256,height%256,(1<<bits) if bits<8 else 0,0,1,bits,len(data),22)+data
     rcfile=file+'.ico'; (out/rcfile).write_bytes(ico)
    from rc_decode import dialog, menu, strings
    lines.append('LANGUAGE %d, %d'%(lang&1023,lang>>10))
    if typ==4: lines.extend(menu(num,data))
    elif typ==5: lines.extend(dialog(num,data))
    elif typ==6: lines.extend(strings(num,data))
    elif typ==14: lines.append(token(num)+' ICON \"resources/resource_000.bin.ico\"')
    elif typ!=3: lines.append(token(num)+' '+token(typ)+' \"resources/'+rcfile+'\"')
    records.append(dict(path=p,file=file,size=size,sha256=hashlib.sha256(data).hexdigest()))

 walk(0,[]); (HERE/'resources.rc').write_text('\n'.join(lines)+'\n'); (HERE/'resources.json').write_text(json.dumps(records,indent=2))
if __name__=='__main__': generate()
