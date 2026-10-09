// Bitmap / Scene / Splash trivial getters.
#include "Ui/Ui.h"

void *Bitmap::GetSurface() { return surface; }
int   Bitmap::GetWidth()   { return width; }
int   Bitmap::GetHeight()  { return height; }
void  Bitmap::ReportError(int hr) {}

void *Scene::GetSceneChild()  { return sceneChild; }
void *Scene::GetCameraFrame() { return renderer->cameraFrame; }

int   Splash::IsShown()  { return shown; }
