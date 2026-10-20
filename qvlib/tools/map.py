"""Map the untouched unity object to SQZ.EXE; run corpus.pl first in Git Bash.

Only unambiguous exact bodies and relocation evidence establish mappings.
Fuzzy candidates are recorded separately unless supported by relocation evidence.
"""
import collections
import difflib
import hashlib
import json
from pathlib import Path
import re
import struct
import sys
import bisect

HERE = Path(__file__).resolve().parents[1]
ROOT = HERE.parents[1]
U32 = lambda b, o=0: struct.unpack_from('<I', b, o)[0]

class PE:
    def __init__(self, path):
        self.data = path.read_bytes()
        p = U32(self.data, 0x3c)
        self.base = U32(self.data, p+24+28)
        n = struct.unpack_from('<H', self.data, p+6)[0]
        opt = struct.unpack_from('<H', self.data, p+20)[0]
        self.sections = [struct.unpack_from('<IIII', self.data, p+24+opt+i*40+8) for i in range(n)]
    def read(self, a, n):
        for vs, va, rs, rp in self.sections:
            if va <= a-self.base < va+rs:
                return self.data[rp+a-self.base-va:rp+a-self.base-va+n]
        return b''
    def word(self, a):
        b = self.read(a, 4)
        return U32(b) if len(b)==4 else None

def main():
    c = json.loads((HERE/('obj/corpus-parts.json' if '--parts' in sys.argv else 'obj/corpus.json')).read_text())
    fs = c['functions']
    ex = {int(k):v for k,v in c['exe'].items()}
    pe = PE(ROOT/'Exported/SQZ.EXE')
    sections = {s['index']:s for s in c['sections']}
    original_addresses=set(ex)
    def sliced_variant(addr, sz):
        for base in sorted((b for b in ex if b<=addr),reverse=True):
            if addr-base>20000: break
            for v in sorted(ex[base]['variants'],key=lambda x:x['size'],reverse=True):
                off=addr-base
                bb=bytes.fromhex(v['masked'])
                oo=v.get('offsets',[])
                if off not in oo or off+sz>len(bb): continue
                indices=[i for i,x in enumerate(oo) if off<=x<off+sz]
                if off+sz<len(bb) and off+sz not in oo: continue
                nn=[]
                for i in indices:
                    text=v['norm'][i]
                    mm=re.fullmatch(r'(j\w+|loop\w*) \+(\d+)',text)
                    if mm:
                        target=int(mm[2])-off
                        text=mm[1]+(' +'+str(target) if 0<=target<sz else ' X')
                    nn.append(text)
                return {'size':sz,'masked':bb[off:off+sz].hex(),'norm':nn,'offsets':[oo[i]-off for i in indices]}
        return None
    bykey = collections.defaultdict(set)
    key = lambda lines: '\n'.join(lines)
    for a,e in ex.items():
        for v in e['variants']:
            bykey[key(v['norm'])].add(a)
    candidates = {n:set(bykey[key(f['norm'])]) for n,f in fs.items()}
    byte_exact = {}
    for n, aa in candidates.items():
        f = fs[n]
        ob = bytes.fromhex(f['masked'])
        for a in aa:
            for v in ex[a]['variants']:
                eb = bytearray.fromhex(v['masked'])
                if len(eb)!=len(ob): continue
                for r in f['relocs']:
                    o=r['offset']
                    if r['type'] in (6,20) and o+4<=len(eb):
                        eb[o:o+4]=bytes.fromhex('efbeadde')
                if hashlib.sha256(eb).digest()==hashlib.sha256(ob).digest():
                    byte_exact[n,a]=v['size']
                    break
    mapped, inverse, reasons, conflicts = {}, {}, {}, set()
    outside=collections.defaultdict(set)
    def add(n, a, why):
        if n not in fs:
            return False
        if not 0x47e600<=a<0x48e000:
            outside[n].add(a)
            return False
        sz=len(bytes.fromhex(fs[n]['masked']))
        extra=sliced_variant(a,sz)
        if a not in ex:
            if extra is None: return False
            ex[a]={'size':sz,'variants':[extra],'discovered':True}
        elif extra and all(v['size']!=sz for v in ex[a]['variants']):
            ex[a]['variants'].append(extra)
        if extra and extra['norm']==fs[n]['norm']:
            candidates[n].add(a)
        if n in mapped:
            if mapped[n]!=a:
                conflicts.add((n,a,why,'already '+hex(mapped[n])))
            return False
        if a in inverse and inverse[a]!=n:
            conflicts.add((n,a,why,'occupied '+inverse[a]))
            return False
        mapped[n], inverse[a], reasons[n] = a, n, why
        return True
    # Unique on both sides, excluding tiny bodies shared by many methods.
    reverse = collections.defaultdict(set)
    for n, aa in candidates.items():
        for a in aa:
            reverse[a].add(n)
    for n,aa in candidates.items():
        if len(aa)==1 and len(reverse[next(iter(aa))])==1:
            add(n, next(iter(aa)), 'unique exact normalized body')
    # Seed hints are imported by address. Renamed reconstruction classes are
    # translated through exact body candidates, then the call graph resolves them.
    hints = collections.defaultdict(list)
    paths = list((ROOT/'Repo/src/Qv').glob('targets*.txt')) + list((ROOT/'Repo/src/Qv/gen').glob('targets_*.txt')) + [ROOT/'Repo/src/Leaves/targets_qv.txt']
    for path in paths:
        for line in path.read_text().splitlines():
            if line.startswith('#') or not line.strip(): continue
            parts = line.split()
            if len(parts)>=3:
                a = int(parts[1],16)
                if a in ex: hints[a].append({'file':str(path.relative_to(ROOT)), 'regex':parts[0], 'label':parts[2]})
    # Unique string operands anchor methods changed enough to lose their body
    # hash (notably diagnostic output). This also avoids manual seed renaming.
    literals=collections.defaultdict(set)
    text=pe.read(0x47e600,0x48e000-0x47e600)
    def file_va(pos):
        for vs,va,rs,rp in pe.sections:
            if rp<=pos<rp+rs: return pe.base+va+pos-rp
    for n,f in fs.items():
        for r in f['relocs']:
            ss=sections.get(r['section'])
            if r['type']!=6 or not ss: continue
            raw=bytes.fromhex(ss['raw']); start=r.get('value',0)+r['addend']
            literal=raw[start:].split(b'\0',1)[0]
            if len(literal)<8 or any(x not in (9,10,13) and not 32<=x<127 for x in literal): continue
            literals[literal].add(n)
    literal_evidence=collections.defaultdict(list)
    for literal, names in literals.items():
        if len(names)!=1: continue
        pos=pe.data.find(literal+b'\0')
        if pos<0 or pe.data.find(literal+b'\0',pos+1)>=0: continue
        va=file_va(pos)
        if va is None: continue
        pointer=struct.pack('<I',va)
        where=text.find(pointer)
        owners=set()
        while where>=0:
            operand=0x47e600+where
            base=max((a for a in ex if a<=operand),default=0)
            if base and operand<base+ex[base]['size']: owners.add(base)
            where=text.find(pointer,where+1)
        if len(owners)==1:
            n=next(iter(names));literal_evidence[n].append((next(iter(owners)),literal.decode('ascii')))
    for n, evidence in literal_evidence.items():
        aa={a for a,s in evidence}
        if len(aa)==1:
            add(n,next(iter(aa)),'unique literal reference '+repr(evidence[0][1]))
    # Relocations bind code, data and vtables. All references are aligned at
    # identical normalized instructions, so near misses need no byte-offset guess.
    data_map = {}
    examined = set()
    def bind_data(sec, addend, a):
        if sec>0 and a:
            data_map.setdefault((sec,addend), a)
    changed = True
    while changed:
        changed = False
        for n,a in list(mapped.items()):
            if n in examined: continue
            examined.add(n)
            f = fs[n]
            raw = bytes.fromhex(f['raw'])
            variants=ex[a]['variants']
            variant=max(variants,key=lambda v:difflib.SequenceMatcher(None,f['norm'],v['norm'],autojunk=False).ratio())
            aligned={}
            for block in difflib.SequenceMatcher(None,f['norm'],variant['norm'],autojunk=False).get_matching_blocks():
                for i in range(block.size): aligned[block.a+i]=block.b+i
            for r in f['relocs']:
                o,t,s = r['offset'],r['type'],r['symbol']
                # VC emits scalar deleting destructors locally but references
                # vector deleting destructor symbols from vtables. The linker
                # resolves them to the scalar form for these classes.
                if s.startswith('??_E') and s not in fs:
                    s='??_G'+s[4:]
                eo=o
                if 'offsets' in f and 'offsets' in variant:
                    i=bisect.bisect_right(f['offsets'],o)-1
                    if i not in aligned: continue
                    eo=variant['offsets'][aligned[i]]+o-f['offsets'][i]
                elif a not in candidates[n]: continue
                word = pe.word(a+eo)
                if word is None: continue
                if t==20 and o and raw[o-1] in (0xe8,0xe9) and pe.read(a+eo-1,1)==raw[o-1:o]:
                    target = (a+eo+4+word)&0xffffffff
                    if s in fs:
                        changed |= add(s,target,'aligned call from '+n+' +'+hex(eo))
                elif t==6:
                    bind_data(r['section'], r.get('value',0)+r['addend'], (word-r.get('value',0)-r['addend'])&0xffffffff)
        # The source vtable is a separate COMDAT. Its DIR32 entries provide
        # virtual-method names even when every getTypeId body is identical.
        for (sec,off),base in list(data_map.items()):
            ss = sections.get(sec)
            if not ss or not ss['name'].startswith('??_7'): continue
            for r in ss['relocs']:
                s=r['symbol']
                if s.startswith('??_E') and s not in fs:
                    s='??_G'+s[4:]
                target=pe.word(base+r['offset'])
                if s in fs and target is not None:
                    changed |= add(s,target,'vtable '+ss['name']+' +'+hex(r['offset']))
        for n,aa in candidates.items():
            free = aa-set(inverse)
            if n not in mapped and len(free)==1:
                a=next(iter(free))
                owners=[x for x in reverse[a] if x not in mapped]
                if len(owners)==1:
                    changed |= add(n,a,'exact body after graph disambiguation')
    def target_size(n,a):
        def cost(v):
            nn=fs[n]['norm']; ee=v['norm']
            same=sum(b.size for b in difflib.SequenceMatcher(None,nn,ee,autojunk=False).get_matching_blocks())
            return len(nn)+len(ee)-2*same, v['size']
        return min(ex[a]['variants'],key=cost)['size']
    covered={}
    for n,a in mapped.items():
        size=target_size(n,a)
        for b in original_addresses:
            if a<b<a+size and b not in inverse: covered[b]={'parent':n,'parent_address':a,'offset':b-a}
    # Recheck byte hashes after call/vtable propagation has supplied additional
    # sizes (including parent functions with embedded EH cleanup funclets).
    byte_exact={}
    for n,a in mapped.items():
        f=fs[n];ob=bytes.fromhex(f['masked'])
        for v in ex[a]['variants']:
            eb=bytearray.fromhex(v['masked'])
            if len(eb)!=len(ob): continue
            for r in f['relocs']:
                o=r['offset']
                if r['type'] in (6,20) and o+4<=len(eb): eb[o:o+4]=bytes.fromhex('efbeadde')
            if hashlib.sha256(eb).digest()==hashlib.sha256(ob).digest():
                byte_exact[n,a]=v['size'];break
    result = {'mapped':{n:{'address':a,'reason':reasons[n]} for n,a in mapped.items()},
              'unmapped_exe':[a for a in ex if a not in inverse],
              'unmapped_source':[n for n in fs if n not in mapped],
              'conflicts':sorted(conflicts),'hints':{str(a):{'source':inverse.get(a),'hints':hh} for a,hh in hints.items()},
              'byte_exact':{n:sz for (n,a),sz in byte_exact.items() if mapped.get(n)==a},
              'covered_funclets':covered,'outside_range':{n:sorted(aa) for n,aa in outside.items()},
              'discovered_addresses':sorted(set(ex)-original_addresses),
              'candidates':{n:sorted(aa) for n,aa in candidates.items() if n not in mapped}}
    # Record near candidates by normalized instruction similarity, size bounded.
    fuzzy={}
    for a in result['unmapped_exe']:
        best=[]
        for n in result['unmapped_source']:
            nn=fs[n]['norm']
            for v in ex[a]['variants']:
                if abs(len(nn)-len(v['norm']))>max(8,len(nn)//4): continue
                sim=difflib.SequenceMatcher(None,nn,v['norm'],autojunk=False).ratio()
                if sim>=0.75: best.append((round(sim,4),n,v['size']))
        fuzzy[hex(a)]=sorted(best,reverse=True)[:5]
    result['fuzzy']=fuzzy
    (HERE/'mapping.json').write_text(json.dumps(result,indent=2)+'\n')
    (HERE/'obj/mapped-corpus.json').write_text(json.dumps({'functions':fs,'exe':ex},separators=(',',':')))
    flags='-GX -Gy -O2 -G3 -ML -D__ANSI_CPP__ '+ ' '.join('-I'+str(HERE/x).replace('\\','/') for x in ['shim','include','src'])
    # Harness accepts .cpp; wrapper preserves the original unity source verbatim.
    lines=['#FLAGS '+flags]
    for n,a in sorted(mapped.items(),key=lambda p:p[1]):
        # Perl's quotemeta form: Python escapes differ for @, which need no
        # quoting in a regex variable. Whitespace cannot occur in mangled names.
        regex='^'+re.sub(r'([^a-zA-Z0-9_])',r'\\\1',n)+'$'
        sz=target_size(n,a)
        lines.append(f'{regex} {a:x} qv_{a:x} {sz}')
    (HERE/'targets_qvlib.txt').write_text('\n'.join(lines)+'\n')
    print(f'{len(mapped)} mapped; {len(result["unmapped_exe"])} EXE and {len(result["unmapped_source"])} source unmapped; {len(conflicts)} conflicting proposals')

if __name__=='__main__': main()
