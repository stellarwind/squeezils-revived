// String-valued getters: return a String member by value (SQZ.EXE 0x421a90 +0xc, 0x40fab0 +0x14, 0x412b50 +0x18), and an element of a String array (0x40f310).
#include "compat41.h"
#include "String/String.h"

struct HasName12 { int pad[3]; String name; String GetName(int unused); };               // +0x0c
struct HasName20 { int pad[5]; String name; String GetName(); };               // +0x14
struct HasName24 { int pad[6]; String name; String GetName(); };               // +0x18
struct NameList  { int pad[3]; String *names; String At(int i); };                       // +0x0c

String HasName12::GetName(int unused) { return name; }
String HasName20::GetName() { return name; }
String HasName24::GetName() { return name; }
String NameList::At(int i) { return names[i]; }
struct HasName8  { int pad[2]; String name; String GetName(); };                          // +0x08
String HasName8::GetName() { return name; }
