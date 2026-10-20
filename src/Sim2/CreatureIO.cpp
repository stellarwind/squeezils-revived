// 1 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
// Creature record (vtable 0x49d5c4, 12 bytes): save / load of the actor and soul, and the empty-record factory.
struct BufferObject { virtual void v0(); };
struct Stream {
    Stream &Write(BufferObject *o);                      // 0x448c60 (ret 4)
    Stream &Read(BufferObject **o);                      // 0x448ca0 (ret 4)
};
struct Creature {
    virtual void v0();
    BufferObject *actor;                                 // 0x04
    BufferObject *soul;                                  // 0x08
    Creature() { actor = 0; soul = 0; }
    void Save(Stream &s);                                // 0x412cc0
    void Load(Stream &s);                                // 0x412cf0
};
void Register(Creature *c);                              // 0x448ce0 (cdecl)

void Creature::Save(Stream &s)
{
    s.Write(actor);
    s.Write(soul);
}

void Creature::Load(Stream &s)
{
    s.Read(&actor);
    s.Read(&soul);
}

void NewCreature()
{
    Register(new Creature);
}
