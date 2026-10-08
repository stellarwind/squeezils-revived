// GoodieBody: see GoodieBody.h. Reconstructed from SQZ.EXE 0x45a850-0x45a9f0.
#include "GoodieBody.h"
#include "Prop.h"
#include "PropLoader.h"
#include "Scene.h"

// @0045a850
GoodieBody *GoodieBody::Init(const float *matrix, const char *propName)
{
    Body::Init(matrix);
    vtbl = (void **)0x49ff7c;
    if (propName) {
        Scene *scene = GetScene();
        instance = scene->CreateSceneFrame();
        scene->SetFrameTransform(instance, matrix);
        FindPropTemplate(propName, NULL, 0)->VBuildFrame(instance);
    }
    facing.x = 0.0f;
    facing.y = 0.0f;
    facing.z = 0.0f;
    return this;
}

// @0045a940 (it ends as a plain Body again)
void GoodieBody::Destruct()
{
    vtbl = (void **)0x49ff7c;
    Scene *scene = GetScene();
    DeleteChildFromFrame(scene->GetSceneChild(), instance);
    vtbl = (void **)0x49d484;
}

// @0045a920
GoodieBody *GoodieBody::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        delete this;
    return this;
}

// @0045a9d0
void GoodieBody::UpdateFrame()
{
    GetScene()->SetFrameTransform(instance, matrix);
}

// @0045a9f0
int GoodieBody::HasContact()
{
    return contact.face != -1;
}
