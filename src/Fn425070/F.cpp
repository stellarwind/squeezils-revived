// SQZ.EXE 0x425070 (cdecl): load a texture file under a name and register it in the top texture table, replacing an older one; 0 on failure.
#include <string.h>
#include "Vector3f/Vector3f.h"
#include "String/String.h"

struct Texture { virtual long __stdcall q0(); virtual unsigned long __stdcall q1(); virtual unsigned long __stdcall Release(); };
struct TexTableT {
    virtual int Insert(const String &name, Texture *const &tex, int hint);   // slot 0
    virtual int Contains(const String &name);                                // slot 1
    Texture *Get(const String &name);                                        // 0x467e90
};
struct PpmLoader { virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
                   virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
                   virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17();
                   virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21(); virtual void s22();
                   virtual void __stdcall LoadPpm(const char *file, Texture **out); };   // slot 23 (+0x5c)
struct AppT { char pad[0x2c]; PpmLoader *loader; };
extern AppT *g_app;                                      // 0x4c75dc
extern char gColorKey[];                                 // 0x4c7168
extern const char kLoadingTexture[], kPpm[], kEmpty2[], kFailed[], kOk[];   // 0x4bb83c 0x4bb834 0x4bb5f4 0x4ba08c 0x4b9dbc
int DebugLogAll(const char *format, ...);                  // 0x419290
Texture *LoadTextureFile(const char *file, Vector3f *key, char *color, const char *extra);   // 0x415440 (cdecl)
TexTableT *TopTable();                                   // 0x425bf0

Texture *LoadTexture(const char *name, const char *file)
{
    DebugLogAll(kLoadingTexture, name, file);
    Texture *tex = 0;
    if (strstr(file, kPpm))
        g_app->loader->LoadPpm(file, &tex);
    if (!tex) {
        Vector3f key(1.0f, 0.0f, 1.0f);
        tex = LoadTextureFile(file, &key, gColorKey, kEmpty2);
    }
    if (!tex) {
        DebugLogAll(kFailed);
        return 0;
    }
    DebugLogAll(kOk);
    if (TopTable()->Contains(String(name)))
        TopTable()->Get(String(name))->Release();
    TopTable()->Insert(String(name), tex, -1);
    return tex;
}
