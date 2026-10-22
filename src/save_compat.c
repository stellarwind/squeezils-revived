/* The original save dialog creates files, but never creates SQZ\SAVE itself. */
#include "save_compat.h"
#include <string.h>

HANDLE WINAPI sqz_save_create_file(LPCSTR path, DWORD access, DWORD sharing,
                                   LPSECURITY_ATTRIBUTES security, DWORD disposition,
                                   DWORD attributes, HANDLE template_file)
{
    HANDLE file =
        CreateFileA(path, access, sharing, security, disposition, attributes, template_file);
    if (file != INVALID_HANDLE_VALUE)
        return file;
    DWORD error = GetLastError();
    if (error == ERROR_PATH_NOT_FOUND && path && (access & GENERIC_WRITE) &&
        (disposition == CREATE_NEW || disposition == CREATE_ALWAYS))
    {
        char directory[MAX_PATH];
        DWORD length = GetModuleFileNameA(NULL, directory, sizeof(directory));
        char *separator = length && length < sizeof(directory) ? strrchr(directory, '\\') : NULL;
        if (separator &&
            (size_t)(separator - directory) + sizeof("\\SQZ\\SAVE\\") < sizeof(directory))
        {
            strcpy(separator, "\\SQZ\\SAVE\\");
            size_t prefix = strlen(directory);
            /* Repair only the game's own save directory. Do not redirect IO or
             * change overwrite prompts, access errors, or unrelated paths. */
            if (!_strnicmp(path, directory, prefix) && path[prefix] &&
                !strpbrk(path + prefix, "\\/:"))
            {
                directory[prefix - 1] = '\0';
                if (CreateDirectoryA(directory, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
                    return CreateFileA(path, access, sharing, security, disposition, attributes,
                                       template_file);
            }
        }
    }
    SetLastError(error);
    return INVALID_HANDLE_VALUE;
}
