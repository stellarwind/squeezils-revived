// Alloc: the game's thin allocator wrappers.
#pragma once
#include "compat41.h"

void *GameAlloc(unsigned n);   // 0x448350
void  GameFree(void *p);       // 0x448360
