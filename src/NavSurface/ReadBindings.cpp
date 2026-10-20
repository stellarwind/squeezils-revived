// 0x40e2e0: load the surface-type bindings from an INI section.
#include "IniReader/IniReader.h"
int DebugLogAll(const char *format, ...); // 0x419290
void __stdcall ReadSurfaceBindings(IniReader *ini, IniValueTable *out)
{
    DebugLogAll("Reading surface bindings: ");
    ini->SetSection("SurfaceBindingList");
    StringList *keys = ini->GetKeys();
    if (keys->count == 0) {
        delete keys;
        return;
    }
    for (int i = 0; i < keys->count; ++i) {
        String key;
        String value;
        key = keys->Get(i);
        value = String(ini->GetString(key.c_str()));
        out->Insert(key, value, -1);
    }
    DebugLogAll("OK.\n");
    delete keys;
}
