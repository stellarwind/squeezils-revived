// Original 0x422ce0; one script dispatch entry for immediate and delayed callers.
#pragma once
// cdecl; four 32-bit stack arguments; caller cleans 16 bytes; success in EAX.
// caller is opaque here: dispatch forwards it unchanged to the function's Run slot.
int CallFunction(const char *name, void *args, int extra, int local);
