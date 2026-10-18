import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import ghidra.program.model.pcode.*;
public class Snapshot extends GhidraScript {
 public void run() throws Exception {
 PrintWriter w=new PrintWriter(getScriptArgs()[0]);
 SymbolIterator si=currentProgram.getSymbolTable().getAllSymbols(true);
 while(si.hasNext()){Symbol s=si.next(); if(s.getAddress().isMemoryAddress())w.println("S\t"+s.getAddress()+"\t"+s.getName().replace('\t',' '));}
 DataIterator di=currentProgram.getListing().getDefinedData(true);
 while(di.hasNext()){Data d=di.next();w.println("D\t"+d.getAddress()+"\t"+d.getLength()+"\t"+d.getDataType().getName());}
 ReferenceIterator ri=currentProgram.getReferenceManager().getReferenceIterator(currentProgram.getMinAddress());
 while(ri.hasNext()){Reference r=ri.next();if(r.getToAddress().isMemoryAddress())w.println("R\t"+r.getFromAddress()+"\t"+r.getToAddress());}
 InstructionIterator ii=currentProgram.getListing().getInstructions(true);
 while(ii.hasNext()) {
  Instruction ins=ii.next(); String mnemonic=ins.getMnemonicString().toUpperCase();
  for(int op=0;op<ins.getNumOperands();op++) {
   String operand=ins.getDefaultOperandRepresentation(op).toLowerCase();
   int width=(operand.contains("qword ptr") || operand.contains("double ptr"))?8:(operand.contains("dword ptr") || operand.contains("float ptr"))?4:operand.contains("word ptr")?2:0;
   if(width==0)for(PcodeOp operation:ins.getPcode()) {
    if(operation.getOpcode()==PcodeOp.LOAD && operation.getOutput()!=null)width=operation.getOutput().getSize();
    else if(operation.getOpcode()==PcodeOp.STORE)width=operation.getInput(2).getSize();
   }
   if(width!=2 && width!=4 && width!=8)continue;
   String kind=null;
   if(mnemonic.matches("F(LD|ST|STP|ADD|ADDP|SUB|SUBR|MUL|DIV|DIVR|COM|COMP).*"))kind="F";
   else if(mnemonic.matches("(MOV|MOVSX|MOVZX|ADD|SUB|CMP|TEST|AND|OR|XOR|INC|DEC|IMUL|MUL|IDIV|DIV|SHL|SHR|SAR)"))kind="I";
   if(kind!=null)for(Reference ref:ins.getOperandReferences(op))if(ref.getToAddress().isMemoryAddress())
    w.println(kind+"\t"+ref.getToAddress()+"\t"+width+"\t"+ins.getAddress());
  }
 }
 w.close();
 }
}
