#ifndef SQZ_SAVE_COMPAT_H
#define SQZ_SAVE_COMPAT_H
#include <windows.h>

HANDLE WINAPI sqz_save_create_file(LPCSTR path, DWORD access, DWORD sharing,
                                   LPSECURITY_ATTRIBUTES security, DWORD disposition,
                                   DWORD attributes, HANDLE template_file);
#endif
