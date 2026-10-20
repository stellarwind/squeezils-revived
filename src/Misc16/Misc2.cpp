// More leaf functions (names are guesses from behaviour).
#include "compat41.h"

// 0x43ee10: one step of the main loop unless paused
struct Clock { int Tick(); };                                      // 0x41ed40
Clock *GetClock();                                                 // 0x4206d0
void RunFrame();                                                   // 0x4206e0
void RunFrame2();                                                  // 0x420760
extern int gPausedFlag;                                            // 0x4c7700
extern int gWindowHook;                                            // 0x4c76f4
int StepGame()
{
    if (gPausedFlag)
        return 0;
    if (gWindowHook || !GetClock()->Tick())
        RunFrame();
    if (!gWindowHook && !GetClock()->Tick())
        return 0;
    RunFrame2();
    return 0;
}

// 0x45a9d0: hand this object's handle to the global registry
struct Reg { void Register(int handle, void *where); };            // 0x477040
extern Reg *gReg;                                                  // 0x4c75e0
struct Handled { int pad; int slot4; char pad2[0xc0 - 8]; int handle; void RegisterSelf(); };
void Handled::RegisterSelf() { gReg->Register(handle, &slot4); }

// 0x44e9e0: destructor of an object that owns two sub-objects
struct OwnedPart { ~OwnedPart(); };                                // 0x473890
struct TwoOwner {
    virtual ~TwoOwner();                                           // 0x44e9e0 (vtable 0x49ed34)
    OwnedPart *a;                                                  // 0x04
    OwnedPart *b;                                                  // 0x08
    int pad[4];
    int last;                                                      // 0x1c
};
TwoOwner::~TwoOwner()
{
    delete a;
    delete b;
    last = 0;
}

// 0x456b40: put n nuts' worth of the spit goodie into the squeezil's inventory
struct GoodieInv { void Add(const char *goodie); };                // 0x416110 (ret 4)
struct Possess { GoodieInv *GetInventory(); };                     // 0x448990
struct NameSource { const char *Name(); };                         // 0x414250
NameSource *GetNames();                                            // 0x420690
struct NutGiver {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17();
    virtual Possess *Find(const char *name);                       // slot 18 (+0x48)
    void GiveNuts(int n);                                          // 0x456b40
};
void NutGiver::GiveNuts(int n)
{
    if (n > 0) {
        NameSource *names = GetNames();
        while (n > 0) {
            Find("SpitNutGoodie")->GetInventory()->Add(names->Name());
            n--;
        }
    }
}

// 0x44e070 / 0x44e0b0: absorb the items of a source's lists
struct ItemList { virtual int Get(int i); int count; };            // slot 0; count at 0x04
struct Source { int pad[3]; int extra; ItemList *ListA(const char *name); };   // 0x4163d0 ; extra at 0x0c
struct Sink {
    void AddItem(int item);                                        // 0x44e020
    void AddList(ItemList *list);                                  // 0x44e070
    void Absorb(Source *s);                                        // 0x44e0b0
};
void Sink::AddList(ItemList *list)
{
    if (list) {
        for (int i = 0; i < list->count; ) {
            AddItem(list->Get(i));
            i++;
        }
    }
}
void Sink::Absorb(Source *s)
{
    AddList(s->ListA("FoodNutGoodie"));
    AddList(s->ListA("SpitNutGoodie"));
    AddItem(s->extra);
}

// 0x458ec0: stop whatever the object is playing, then tell the registry
struct Player2 { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void Stop();   // slot 4
                 virtual void Pause(int a, int b); };               // slot 5
struct Jukebox {
    char pad[0x100];
    int slot100;                                                   // 0x100
    char padA[0x220 - 0x104];
    int handle;                                                    // 0x220
    char pad2[8];
    Player2 *player;                                               // 0x22c
    int state;                                                     // 0x230
    int IsPlaying();                                               // 0x4589a0
    void Halt();                                                   // 0x458ec0
};
void Jukebox::Halt()
{
    if (state != 0x31 && IsPlaying()) {
        if (state == 0x10)
            player->Stop();
        else
            player->Pause(0, 0);
    }
    gReg->Register(handle, &slot100);
}

// 0x41dd30: format a message through the sink at 0x04
#include "String/String.h"
struct TextSink { virtual void Write(const String &text, int *args, int mode); };   // slot 0
struct Logger {
    int pad;
    TextSink sink;                                                 // 0x04
    void Say(const char *fmt, int arg);                            // 0x41dd30
};
void Logger::Say(const char *fmt, int arg)
{
    sink.Write(String(fmt), &arg, -1);
}

// 0x416bf0: factory for a 0x70-byte object whose constructor is 0x415ef0
struct Made70 { char pad[0x70]; Made70(); };
Made70 *NewMade70() { return new Made70; }

// 0x4276b0: value of the named entry of the linked table, or the object's own value when nothing is linked
struct Entry { virtual int v0(); virtual int v1(); virtual int Value(); };   // slot 2 (+8)
struct Table2 { int pad; Entry *Find(const String &key); };                 // sub-object at +4, 0x46b110
struct Valued {
    char pad[0x34];
    int own;                                                       // 0x34
    char pad2[0x94 - 0x38];
    struct Link { int pad; Table2 table; } *link;                  // 0x94 (table at +4)
    int Get();                                                     // 0x4276b0
};
int Valued::Get()
{
    if (!link)
        return own;
    Entry *e = link->table.Find(String("part"));
    return e->Value();
}

// 0x425a30: a record of three vectors
struct PlainVec { float x, y, z; PlainVec() {} ~PlainVec(); };      // dtor 0x401660
struct ThreeVecs {
    int id;                                                        // 0x00
    PlainVec a;                                                    // 0x04
    PlainVec b;                                                    // 0x10
    int pad;                                                       // 0x1c
    PlainVec c;                                                    // 0x20
    ~ThreeVecs();                                                  // 0x425a30
};
ThreeVecs::~ThreeVecs() {}

// 0x424c80: release every item of the global table, then delete the table and drop its slot from the owner list
struct ComItem { virtual long __stdcall Q(); virtual unsigned long __stdcall A(); virtual unsigned long __stdcall Release(); };
struct KeyList { virtual String KeyAt(int i); };                   // slot 0, embedded at +4
struct ItemTable {
    int pad;
    KeyList keys;                                                  // 0x04
    char pad2[0x8 - 0x8];
};
struct BigTable {
    int pad;
    KeyList keys;                                                  // 0x04
    int count;                                                     // 0x08
    char padB[0x24 - 0x0c];
    ComItem **items;                                               // 0x24
    ~BigTable();                                                   // 0x4678e0
    String KeyAt(int i);                                           // 0x424d50
};
struct OwnerList { void RemoveAt(int i); };                        // 0x466b40
BigTable *GetBigTable();                                           // 0x424dc0
extern OwnerList gOwners;                                          // 0x4c71c0
extern int gOwnerCount;                                            // 0x4c71c4
void ShutdownTable()
{
    int i = 0;
    for (;;) {
        if (!(i < GetBigTable()->count))
            break;
        else
            GetBigTable()->KeyAt(i).c_str();
        GetBigTable()->items[i]->Release();
        i++;
    }
    delete GetBigTable();
    gOwners.RemoveAt(gOwnerCount - 1);
}

// 0x462cc0: name of the entry for a key (index lookup through the embedded key set, then the String getter 0x40f2a0)
struct KeyIndex7 { virtual int v0(); virtual int v1(); virtual int v2(); virtual int v3(); virtual int v4(); virtual int v5(); virtual int v6();
                   virtual int IndexOf(int key); };                // slot 7 (+0x1c)
struct NameTable {
    int pad;
    KeyIndex7 keys;                                                // 0x04
    String NameAtIndex(int index);                                 // 0x40f2a0
    String NameOf(int key);                                        // 0x462cc0
};
String NameTable::NameOf(int key) { return NameAtIndex(keys.IndexOf(key)); }

// 0x40f2a0 (the String getter used above): element i of the String array embedded at 0x18 (accessor 0x40f310)
struct NameArray { int pad[3]; String *names; String At(int i); };  // 0x40f310 (Fam112c/StrGet.cpp)
struct NameTable2 {
    char pad[0x18];
    NameArray list;                                                // 0x18
    String NameAtIndex(int index);                                 // 0x40f2a0
};
String NameTable2::NameAtIndex(int index) { return list.At(index); }

// 0x45e970: Array<Vector3f>::Replace (12-byte elements copied inline)
struct V12 { float x, y, z; V12 &operator=(const V12 &o) { x = o.x; y = o.y; z = o.z; return *this; } };
struct ArrayV12 {
    int vtbl, count, cap;
    V12 *data;                                                     // 0x0c
    void Replace(int i, const V12 &v);                             // 0x45e970
};
void ArrayV12::Replace(int i, const V12 &v) { data[i] = v; }
