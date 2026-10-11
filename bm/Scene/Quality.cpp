// Scene::SetQuality (0x476ea0): picks the render quality of the D3DRM device (wireframe .. Phong).
// (A private view of Scene: only the renderer pointer. The scene header is not included, so that the symbol count stays what it was.)
#include "Renderer2/Renderer2.h"

// two spare declarations: the compiler's register choices follow the symbol count (tools/flagsearch/padscan.sh)
void PadDecl1();
void PadDecl2();

class Scene {
public:
    Renderer *renderer;
    void SetQuality(int level);
};

void Scene::SetQuality(int level)
{
    Renderer *r = renderer;
    if (r) {
        switch (level) {
        case 0:
            r->device->SetQuality((D3DRMRENDERQUALITY)0x40);
            break;
        case 1:
            r->device->SetQuality((D3DRMRENDERQUALITY)0x80);
            break;
        case 2:
            r->device->SetQuality((D3DRMRENDERQUALITY)0x88);
            break;
        case 3:
            r->device->SetQuality((D3DRMRENDERQUALITY)0x89);
            break;
        case 4:
            r->device->SetQuality((D3DRMRENDERQUALITY)0x8a);
            break;
        }
    }
}
