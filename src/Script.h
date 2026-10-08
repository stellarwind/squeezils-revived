// Script functions: the named actions the level scripts trigger (the classes derived from Function, RTTI vtable 0x49dabc, whose
// slot 0 is the destructor and slot 1 the pure virtual Execute). Reconstructed from SQZ.EXE 0x422ce0 and its helper 0x423130.
//
// Every place or region keeps a table of its functions by name (Table<String, Function *>). The tables are stacked in a global
// array (one per level, 0x4c7090, like the texture and geometry stacks in Resources.h) and a lookup searches from the top down.
// A call can ask for the top table only ("local").
#pragma once

struct Function;   // the original's script function objects; only their virtual Execute (slot 1) is called from here

enum FunctionSlot { FUNCTION_DESTROY = 0, FUNCTION_EXECUTE = 1 };

// @00422ce0 runs the function called name with (args, extra). local = 0 searches the whole stack, from the top table down; nonzero
// only the top table. Logs "ERROR: couldn't find function %s." and returns 0 when there is none, otherwise returns 1.
// What args and extra mean is up to the function (the callers pass a pointer to a small record and 0 so far).
int PostFunction(const char *name, void *args, int extra, int local);
