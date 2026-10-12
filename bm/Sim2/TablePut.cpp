// 20 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
void PadDecl18();
void PadDecl19();
void PadDecl20();
// CreatureTable::Put (SQZ.EXE 0x412da0): file the actor and soul under the id; a creature already there is destroyed first.
struct Object { virtual void Destroy(int flags); };
struct Creature {
    virtual void Describe();
    Object *actor;                                       // 0x04
    Object *soul;                                        // 0x08
    Creature(Object *a, Object *s) { actor = a; soul = s; }
};
struct CreatureTable {
    virtual int Insert(const int &id, Creature *const &value, int index);   // slot 0 (+0)
    virtual int Contains(const int &id);                                    // slot 1 (+4)
    Creature *Get(const int &id);                                           // 0x466820
    void Put(int id, Object *actor, Object *soul);
};

inline void Kill(Object *o) { if (o) o->Destroy(1); }
void CreatureTable::Put(int id, Object *actor, Object *soul)
{
    if (Contains(id)) {
        Creature *old = Get(id);
        Kill(old->actor);
        Kill(old->soul);
        delete old;
    }
    Insert(id, new Creature(actor, soul), -1);
}
