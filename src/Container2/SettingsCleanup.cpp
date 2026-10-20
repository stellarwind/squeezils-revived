// 0x4226a0: guarded shutdown of the global script-variable table.
#include "String/String.h"
#include "Container/ContainerTable.h"

typedef BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21> SettingsTable;
extern SettingsTable g_settings;                    // 0x4c7060
extern unsigned char f_4c70dc;

void G4226a0()
{
    if (!(f_4c70dc & 4)) {
        f_4c70dc |= 4;
        g_settings.~SettingsTable();
    }
}
