// Slot 8 of the CreateGoodie commands: hand the transform (and the launch velocity) to the level (0x440e50, 0x441550).
#define GOODIE_EXEC
#include "FunctionGoodie/Goodie.h"

struct GoodieLevel {
    void SpawnGoodie(const char *name, GoodieMatrix m);                                  // 0x44d750
    void SpawnGoodie(const char *name, GoodieMatrix m, Vector3f velocity, float speed);  // 0x44d990
};

void CreateGoodieAtPositionFunction::Spawn(int extra, void *level, const char *name, GoodieMatrix *m)
{
    ((GoodieLevel *)level)->SpawnGoodie(name, *m);
}

void CreateGoodieAtPositionWithVelocityFunction::Spawn(int extra, void *level, const char *name, GoodieMatrix *m)
{
    ((GoodieLevel *)level)->SpawnGoodie(name, *m, velocity->Get(extra), speed->Get(extra));
}
