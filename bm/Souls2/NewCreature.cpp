// NewCreature (0x4142b0): `new Creature` with an inlined constructor.
struct Creature {
    void *actor;
    void *soul;
    Creature() { actor = 0; soul = 0; }
    virtual ~Creature();
};

Creature *NewCreature()
{
    return new Creature;
}
