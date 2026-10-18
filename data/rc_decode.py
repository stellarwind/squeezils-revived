"""Translate native resources to VC4 RC syntax without hand conversion."""
import struct
class Reader:
 def __init__(self,b): self.b=b; self.p=0
 def word(self): v=struct.unpack_from('<H',self.b,self.p)[0];self.p+=2;return v
 def dword(self): v=struct.unpack_from('<I',self.b,self.p)[0];self.p+=4;return v
 def align(self): self.p=(self.p+3)&~3
 def text(self):
  v=self.word()
  if v==65535:return self.word()
  a=[]
  while v:a.append(chr(v));v=self.word()
  return ''.join(a)
def quote(t):
 if isinstance(t,int):return str(t)
 return '"'+t.replace('\\','\\\\').replace('"','""').replace('\n','\\n').replace('\r','\\r').replace('\t','\\t')+'"'
def dialog(num,b):
 r=Reader(b);style=r.dword();ex=r.dword();n=r.word();xy=[r.word() for _ in range(4)];menu=r.text();cls=r.text();title=r.text()
 lines=['%s DIALOG %s'%(quote(num),','.join(map(str,xy))),'STYLE 0x%08XL'%style,'EXSTYLE 0x%08XL'%ex,'CAPTION '+quote(title)]
 if menu:lines.append('MENU '+quote(menu))
 if cls:lines.append('CLASS '+quote(cls))
 if style&64:lines.append('FONT %d, %s'%(r.word(),quote(r.text())))
 lines.append('BEGIN')
 for i in range(n):
  r.align();s=r.dword();e=r.dword();xy=[r.word() for _ in range(4)];cid=r.word();cl=r.text();text=r.text();extra=r.word()
  assert extra==0
  if isinstance(cl,int):cl={128:'BUTTON',129:'EDIT',130:'STATIC',131:'LISTBOX',132:'SCROLLBAR',133:'COMBOBOX'}[cl]
  lines.append(' CONTROL %s,%d,%s,0x%08XL,%s,0x%08XL'%(quote(text),cid,quote(cl),s,','.join(map(str,xy)),e))
 lines.append('END');return lines
def menu(num,b):
 r=Reader(b);assert r.word()==0;off=r.word();r.p+=off;lines=[str(num)+' MENU','BEGIN']
 def level():
  while True:
   flags=r.word();popup=flags&16;cid=None if popup else r.word();text=r.text();opts=[]
   for bit,name in [(1,'GRAYED'),(2,'INACTIVE'),(8,'CHECKED'),(32,'MENUBARBREAK'),(64,'MENUBREAK'),(16384,'HELP')]:
    if flags&bit:opts.append(name)
   if popup:lines.append('POPUP '+quote(text)+(' ,'+','.join(opts) if opts else ''));lines.append('BEGIN');level();lines.append('END')
   elif not text and cid==0:lines.append('MENUITEM SEPARATOR')
   else:lines.append('MENUITEM '+quote(text)+','+str(cid)+(','+','.join(opts) if opts else ''))
   if flags&128:break
 level();lines.append('END');return lines
def strings(num,b):
 r=Reader(b);lines=['STRINGTABLE','BEGIN']
 for i in range(16):
  n=r.word();s=''.join(chr(r.word()) for _ in range(n))
  if s:lines.append('%d %s'%((num-1)*16+i,quote(s)))
 lines.append('END');return lines
