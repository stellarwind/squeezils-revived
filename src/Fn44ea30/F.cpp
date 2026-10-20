// Splash::Load (0x44ea30): makes sure the picture bitmap for slot n is loaded and shown. Returns 0 if the file could not be loaded.
#include "Bitmap2/Bitmap2.h"
extern Renderer *g_renderer;                         // 0x4c75dc
struct SplashL {
    virtual void v0();
    Bitmap *picture;     // 0x04
    char pad8[0x1c - 8];
    int shown;           // 0x1c
    char path[0x3c];     // 0x20
    int current;         // 0x5c
    float scale;         // 0x60
    void Release();      // 0x44ee40
    void BuildPath(int n);   // 0x44eb20
    void Show();         // 0x44edd0
    int Load(int n);
};
int SplashL::Load(int n)
{
    scale = 1.0f;
    if (picture && current != n)
        Release();
    if (!picture)
        picture = new Bitmap(g_renderer);
    if (picture) {
        if (!shown) {
            BuildPath(n);
            if (!picture->Load(path))
                return 0;
        }
        if (current != n)
            Show();
    }
    current = n;
    return 1;
}
