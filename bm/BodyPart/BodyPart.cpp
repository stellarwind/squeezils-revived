// BodyPart::Init overloads (0x41d870/0x41d8a0/0x41d8c0) and GetFrame (0x41d890).
#include "BodyPart/BodyPart.h"

BodyPart *BodyPart::Init(void *m)
{
    vtbl = (void *)0x49d7c4;
    model = m;
    morph = 0;
    frame = 0;
    return this;
}

BodyPart *BodyPart::Init()
{
    vtbl = (void *)0x49d7c4;
    model = 0;
    morph = 0;
    frame = 0;
    return this;
}

BodyPart *BodyPart::Init(const BodyPart *like)
{
    vtbl = (void *)0x49d7c4;
    model = like->model;
    morph = 0;
    frame = 0;
    return this;
}

void *BodyPart::GetFrame()
{
    return frame;
}
