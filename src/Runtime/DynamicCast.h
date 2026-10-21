// VC4 CRT RTTI entry, SQZ.EXE 0x48f060; supplied by the original LIBC.LIB.
// Five 32-bit cdecl arguments. runtime_aliases.py binds this neutral C name
// to ___RTDynamicCast; VC4 reserves the latter's source spelling internally.
#pragma once
extern "C" void *__cdecl SqzRTDynamicCast(void *object, long vfDelta,
    const void *sourceType, const void *destinationType, int isReference);
