// PopTextureTable (SQZ.EXE 0x425ab0): release the top table's textures and drop the table (a String of every key is built and dropped on the way).
#include "compat41.h"
#include "String/String.h"

struct KeySetP { virtual String Get(int i); };
struct Texture { virtual long __stdcall q0(); virtual unsigned long __stdcall q1(); virtual unsigned long __stdcall Release(); };
struct TexTable {
    int vtbl;
    KeySetP keys;                       // 0x04
    int count;                          // 0x08
    char pad[0x18];
    Texture **values;                   // 0x24
    String KeyAt(int i);                // 0x425b80
    ~TexTable();                        // 0x467d20
};
struct TexStack {
    int pad[1];
    int count;                          // 0x04
    int pad8;
    TexTable **data;                    // 0x0c
    void RemoveAt(int i);               // 0x466cf0
};
extern TexStack g_texStack;             // 0x4c7180
TexTable *TopTable();                   // 0x425bf0

void PopTextureTable()
{
    int i = 0;
    for (;;) {
        if (i >= TopTable()->count)
            break;
        TopTable()->KeyAt(i).c_str();
        TopTable()->values[i]->Release();
        i++;
    }
    TexTable *t = TopTable();
    if (t)
        delete t;
    g_texStack.RemoveAt(g_texStack.count - 1);
}
