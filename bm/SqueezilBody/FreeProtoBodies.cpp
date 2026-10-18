// FreeProtoBodies, SQZ.EXE 0x4580a0.
// Spare declarations retain the closest VC++ 4.1 register allocation.
void FreeProtosPad1();
void FreeProtosPad2();
void FreeProtosPad3();
void FreeProtosPad4();
void FreeProtosPad5();
void FreeProtosPad6();
void FreeProtosPad7();
void FreeProtosPad8();
void FreeProtosPad9();
void FreeProtosPad10();
#include <new.h>
#include "AnimatedBody/AnimatedBody.h"

struct ProtoBodyModel { ~ProtoBodyModel(); }; // 0x471a00
struct ProtoBodyModelArray {
    void *vtable;
    int count, capacity;
    ProtoBodyModel **data;
};
extern AnimatedBody *gProtoBodies[7]; // 0x4bf138
extern ProtoBodyModelArray gBodyModels; // 0x4c7b98
extern MorphCycle *gBodyClips[50]; // 0x4bf070

void FreeProtoBodies()
{
    for (int color = 0; color < 7; ++color) {
        AnimatedBody *body = gProtoBodies[color];
        if (body) {
            body->Destruct();
            operator delete(body);
            gProtoBodies[color] = 0;
        }
    }
    for (int model = 0; model < gBodyModels.count; ++model)
        delete gBodyModels.data[model];
    for (int clip = 0; clip < 50; ++clip)
        delete gBodyClips[clip];
}
