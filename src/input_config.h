#ifndef SQZ_INPUT_CONFIG_H
#define SQZ_INPUT_CONFIG_H
#include <windows.h>
#include <stddef.h>
int __fastcall sqz_input_load(void *object);
int __fastcall sqz_input_save(void *object);
int sqz_input_patch_image(BYTE *base, size_t size);
void sqz_input_install(void);
#endif
