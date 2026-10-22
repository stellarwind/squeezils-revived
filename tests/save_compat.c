#include "save_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    /* tools/test.py runs this executable in a dedicated fixture directory. */
    char path[MAX_PATH];
    assert(GetModuleFileNameA(NULL, path, sizeof(path)));
    char *end = strrchr(path, '\\');
    assert(end);
    strcpy(end, "\\SQZ");
    assert(CreateDirectoryA(path, NULL));
    strcat(path, "\\SAVE\\TEST");
    assert(CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL) == INVALID_HANDLE_VALUE);
    assert(GetLastError() == ERROR_PATH_NOT_FOUND);
    HANDLE file = sqz_save_create_file(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
    assert(file != INVALID_HANDLE_VALUE);
    DWORD written;
    assert(WriteFile(file, "saved", 5, &written, NULL) && written == 5);
    CloseHandle(file);
    assert(sqz_save_create_file(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL) ==
           INVALID_HANDLE_VALUE);
    assert(GetLastError() == ERROR_FILE_EXISTS);
    file = sqz_save_create_file(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    char content[5];
    assert(file != INVALID_HANDLE_VALUE);
    assert(ReadFile(file, content, 5, &written, NULL) && written == 5);
    assert(!memcmp(content, "saved", 5));
    CloseHandle(file);
    assert(DeleteFileA(path));
    strcpy(end, "\\UNRELATED\\TEST");
    assert(sqz_save_create_file(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL) ==
           INVALID_HANDLE_VALUE);
    assert(GetLastError() == ERROR_PATH_NOT_FOUND);
    strcpy(end, "\\SQZ\\SAVE");
    assert(RemoveDirectoryA(path));
    strcpy(end, "\\SQZ");
    assert(RemoveDirectoryA(path));
    puts("PASS: missing save directory repaired; file contents, overwrite protection and unrelated "
         "errors preserved");
    return 0;
}
