// 0x429dc0: release global prop templates and animation paths.
// 2 spare declarations (register allocation, see padscan.sh).
void PadDecl1();
void PadDecl2();
#include "String/String.h"
#include "MorphCycle/MorphCycle.h"

struct ShutdownProp { virtual ~ShutdownProp(); };
struct ShutdownPropTable {
    int pad0, pad4;
    int count;                             // +0x08
    char pad[0x18];
    ShutdownProp **values;                 // +0x24
    String KeyAt(int index);               // 0x429ed0
    void RemoveAt(int index);              // 0x468a50
};
struct ShutdownPathTable {
    int pad0, pad4;
    int count;                             // +0x08
    char pad[0x18];
    MorphCycle **values;                   // +0x24
    void RemoveAt(int index);              // 0x468e90
};
extern ShutdownPropTable gGlobalProps;      // 0x4c7208
extern ShutdownPathTable gGlobalPaths;      // 0x4c7280
void ClearWorldObjects();                  // 0x42d010

void ShutdownA()
{
    ClearWorldObjects();
    for (int i = 0; i < gGlobalProps.count; i++) {
        gGlobalProps.KeyAt(i).c_str();
        ShutdownProp *prop = gGlobalProps.values[i];
        if (prop)
            delete prop;
    }
    for (int j = gGlobalProps.count - 1; j >= 0; j--)
        gGlobalProps.RemoveAt(j);
    int k = 0;
    int offset = 0;
    for (;;) {
        int count = gGlobalPaths.count;
        if (k >= count)
            break;
        MorphCycle *path = *(MorphCycle **)((char *)gGlobalPaths.values + offset);
        if (path)
            delete path;
        offset += 4;
        k++;
    }
    for (int n = gGlobalPaths.count - 1; n >= 0; n--)
        gGlobalPaths.RemoveAt(n);
}
