// 0x40e160: remove the surface-type definitions introduced by the last INI file.
#include "Container/ContainerTable.h"
#include "String/String.h"
typedef Set<String, 11> UnloadNames;
struct UnloadType {
    char data[0x14];
    String activate, deactivate;
};
typedef Table<String, UnloadType *, Array<UnloadType *>, 0> UnloadTypes;
struct NavUnload {
    char path[0x100];
    UnloadNames knownNames;
    Array<UnloadNames *> fileNames;
    char pad124[0x16c - 0x124];
    UnloadTypes types;
    void UnloadLastFile();
};
void NavUnload::UnloadLastFile()
{
    UnloadNames *names = fileNames.Get(fileNames.count - 1);
    for (int i = 0; i < names->count; ++i) {
        String name = names->Get(i);
        knownNames.RemoveKey(name);
        if (types.Contains(name)) {
            delete types.Get(name);
            types.RemoveKey(name);
        }
    }
    delete names;
    fileNames.Remove(fileNames.count - 1);
}
