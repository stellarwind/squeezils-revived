// Four spare declarations: register choices depend on the symbol count (padscan; makes StepAll exact).
void PadDeclA(); void PadDeclB(); void PadDeclC(); void PadDeclD();
// Small leaf functions found by address sweep (names are guesses from behaviour).
struct COMObj { virtual long __stdcall Query(); virtual unsigned long __stdcall AddRef(); virtual unsigned long __stdcall Release(); };

struct SInner { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6();
                virtual int IndexOf(int key); };                   // slot 7 (+0x1c)
struct STable {
    int first;                                                     // 0x00
    SInner inner;                                                  // 0x04
    char pad[0x24 - 8];
    float *values;                                                 // 0x24
    float Lookup(int key);                                         // 0x461ea0
};
float STable::Lookup(int key) { return values[inner.IndexOf(key)]; }

extern COMObj *gSound;                                             // 0x4bd95c
void ReleaseSound() { if (gSound) gSound->Release(); }             // 0x449a80

extern int gCount;                                                 // 0x4c7814
void Step();                                                       // 0x44a1a0
void StepAll() { for (int i = 0; i < gCount; i++) Step(); }   // 0x449a60 (4 off: the original compares esi with the memory operand)

struct Timed {
    char pad[0x364];
    int mode, id, ticks;                                           // 0x364
    void Set(int newId, float seconds);                            // 0x42f200
};
void Timed::Set(int newId, float seconds) { id = newId; mode = 0; ticks = (int)(seconds * 1000.0f); }

// 0x42ee90: remember what the sub-object at 0x9c answers
struct SubAsk { int Ask(int a, int b); };                          // 0x4059e0
struct Asker {
    char pad[0x9c];
    SubAsk sub;                                                    // 0x9c
    char pad2[0x378 - 0x9c - 1];
    int answer;                                                    // 0x378
    void Remember(int a, int b);                                   // 0x42ee90
};
void Asker::Remember(int a, int b) { answer = sub.Ask(a, b); }

// 0x43d5e0: destructor body that frees one buffer
struct BufOwner {
    virtual void f0();
    char *buf;                                                     // 0x04
    ~BufOwner();                                                   // 0x43d5e0
};
BufOwner::~BufOwner() { delete buf; }

// 0x448970: destructor that releases one owned object through its slot 4
struct Obj { virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void Destroy(int freeMem); };
struct OwnerObj {
    virtual void f0();
    Obj *owned;                                                    // 0x04
    ~OwnerObj();                                                   // 0x448970
};
OwnerObj::~OwnerObj() { if (owned) owned->Destroy(1); }

// 0x4482e0: reset the game state
extern int gGameState;                                             // 0x4c7798
void InitA(); void InitB(); void InitC();                          // 0x4241d0 0x424e20 0x4495e0
void ResetGame() { gGameState = 0; InitA(); InitB(); InitC(); }

// 0x43ad00: ask the helper object (0x04) twice through its slots, then hand the result to the non-virtual 0x43db60
struct Helper {
    virtual void s0(); virtual int Second(int a, int b); virtual void s2(); virtual int First(int a);
    void Apply(int v);                                             // 0x43db60
};
struct Asker2 {
    int first;
    Helper *helper;                                                // 0x04
    void Run(int a, int b);                                        // 0x43ad00
};
void Asker2::Run(int a, int b) { int r = helper->First(a); helper->Apply(helper->Second(r, b)); }

// 0x422700: start every subsystem in turn
void InitS1(); void InitS2(); void InitS3(); void InitS4(); void InitS5(); void InitS6(); void InitS7();   // 0x423e60 0x445150 0x4311c0 0x43a580 0x44e7f0 0x441d20 0x470680
void InitSubsystems() { InitS1(); InitS2(); InitS3(); InitS4(); InitS5(); InitS6(); InitS7(); }

// 0x43ed80: pause / resume the game (not while the flag the pointer at 0x4c75e4 refers to is set)
struct PauseTarget { void Apply(); };                              // 0x43b980
PauseTarget * __stdcall GetPauseTarget(int on);                    // 0x4206b0 (stdcall: ret 4)
void PauseSounds();                                                // 0x44ad60
void ResumeSounds();                                               // 0x44add0
extern int *gPauseLock;                                            // 0x4c75e4
extern int gPaused;                                                // 0x4c7700
void SetPaused(int on)
{
    if (*gPauseLock == 0) {
        if (on) {
            GetPauseTarget(1)->Apply();
            PauseSounds();
            gPaused = 1;
        } else {
            GetPauseTarget(0)->Apply();
            ResumeSounds();
            gPaused = 0;
        }
    }
}

// 0x47c670 / 0x47c690: byte-order aware read of a 16-bit value from a file
#include <stdio.h>
int HostIsLittleEndian()                                           // 0x47c670
{
    unsigned long v = 0xff;
    unsigned char *p = (unsigned char *)&v;
    v = 0xff;
    return *p == 0xff;
}
int ReadWord16(FILE *f, short *out)                                // 0x47c690
{
    short w;
    if (fread(&w, 2, 1, f) != 1)
        return 0;
    if (HostIsLittleEndian()) {
        *out = w;
    } else {
        *out = (short)((w << 8) | ((unsigned short)w >> 8));
    }
    return 1;
}

// 0x4180e0: give a node its slot and register it
struct SlotNode { char pad[0x38]; int slot; };
int __stdcall AllocSlot(SlotNode *n, int arg);                     // 0x418590
struct SlotOwner {
    void Add(SlotNode *n, int slot);                               // 0x4183e0
    void Link(SlotNode *n, int arg);                               // 0x4180e0
};
void SlotOwner::Link(SlotNode *n, int arg)
{
    int s = AllocSlot(n, arg);
    n->slot = s;
    Add(n, s);
}

// 0x41d990: look up an object by id and hand it the texture/material of the current level
struct ComThing { virtual long __stdcall v0(); virtual long __stdcall v1(); virtual long __stdcall v2(); virtual long __stdcall v3();
                  virtual long __stdcall v4(); virtual long __stdcall v5(); virtual long __stdcall v6(); virtual long __stdcall v7();
                  virtual long __stdcall v8(); virtual long __stdcall v9(); virtual long __stdcall v10(); virtual long __stdcall v11();
                  virtual long __stdcall v12(); virtual long __stdcall v13(); virtual long __stdcall v14(); virtual long __stdcall v15();
                  virtual long __stdcall v16(); virtual long __stdcall v17(); virtual long __stdcall Apply(void *what); };   // slot 18 (+0x48)
struct Registry { ComThing *Find(int id); };                        // 0x476f90
struct Slot8 { int pad[2]; void *handle; };                         // 0x08
Slot8 * __stdcall FindSlot(int key);                                // 0x472f60
struct Level2 { char pad[0x2c]; int key; };
extern Registry *gRegistry;                                         // 0x4c75e0
extern Level2 *gLevel2;                                             // 0x4c75dc
struct Binder {
    int pad0;
    int active;                                                     // 0x04
    Slot8 *slot;                                                    // 0x08
    ComThing *thing;                                                // 0x0c
    void Bind(int id);                                              // 0x41d990
};
void Binder::Bind(int id)
{
    thing = gRegistry->Find(id);
    if (active) {
        slot = FindSlot(gLevel2->key);
        thing->Apply(slot->handle);
    }
}

// 0x421d80: delete every object in a global list
struct Deletable { virtual ~Deletable(); };
extern int gListCount;                                             // 0x4c70a8
extern Deletable **gList;                                          // 0x4c70c4
void DeleteList()
{
    for (int i = 0; i < gListCount; i++)
    {
        Deletable *p = gList[i];
        if (p)
            delete p;
    }
}

// 0x42d210: tell every object of a global list to update
struct Updatable { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6();
                   virtual void Update(int arg); };                // slot 7 (+0x1c)
extern int gUpdCount;                                              // 0x4c71fc
extern Updatable **gUpd;                                           // 0x4c7204
void LockUpdates();                                                // 0x42d2b0
void UpdateAll(int arg)
{
    LockUpdates();
    for (int i = 0; i < gUpdCount; i++)
        gUpd[i]->Update(arg);
    LockUpdates();
}
