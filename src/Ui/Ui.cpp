// Scene / Splash trivial getters.
#include "Ui/Ui.h"

void *Scene::GetSceneChild()  { return sceneChild; }
void *Scene::GetCameraFrame() { return renderer->cameraFrame; }

int   Splash::IsShown()  { return shown; }
void  Splash::Reset()    { step = 0; }
