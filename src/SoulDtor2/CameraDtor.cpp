// The soul destructor chain (SQZ.EXE 0x40b340-0x40b440) and ~CameraSoul (0x40b510).
#include "compat41.h"
struct MMap { virtual ~MMap(); };                                  // 0x40b340 (vtable 0x49d2b4), scalar deleting 0x40b360
struct Soul : MMap { virtual ~Soul(); };                           // 0x40b380 (0x49d2a4)
struct SimulationSoul : Soul { virtual ~SimulationSoul(); };       // 0x40b440 (0x49d2ec)
struct InputTarget { virtual void Input(); };                       // second base (vtable 0x49d314)
struct CamVec { float x, y, z; CamVec() {} ~CamVec(); };           // dtor 0x401660
struct CamMat { float m[16]; CamMat() {} ~CamMat(); };             // dtor 0x402310
struct CameraSoul : SimulationSoul, InputTarget {
    int pad8[4];                                                   // 0x08
    CamVec focus;                                                  // 0x18
    int pad24[9];                                                  // 0x24
    CamMat mat0, mat1, mat2, mat3;                                 // 0x48 0x88 0xc8 0x108
    virtual ~CameraSoul();                                         // 0x40b510
};
struct RegionCameraSoul : CameraSoul {
    virtual ~RegionCameraSoul() {}                                 // scalar deleting 0x40b7c0
    virtual void Input();
};
struct PlaceCameraSoul : CameraSoul {
    virtual ~PlaceCameraSoul() {}                                  // scalar deleting 0x40b830
    virtual void Input();
};

MMap::~MMap() {}
Soul::~Soul() {}
SimulationSoul::~SimulationSoul() {}
CameraSoul::~CameraSoul() {}
void RegionCameraSoul::Input() {}
void PlaceCameraSoul::Input() {}
RegionCameraSoul *MakeRegionCam() { return new RegionCameraSoul; }
PlaceCameraSoul *MakePlaceCam() { return new PlaceCameraSoul; }

// PhysicalSoul constructor (0x446e30, vtable 0x49ebdc): the chain's inline constructors, then the one-time read of [Physics]
struct PhysicalSoul : SimulationSoul {
    PhysicalSoul();                                                // 0x446e30
    virtual ~PhysicalSoul() {}
};
extern int *gPhysicsRead;                                          // 0x4bd6ac
void ReadPhysicsSettings();                                        // 0x446ed0
PhysicalSoul::PhysicalSoul()
{
    if (!gPhysicsRead)
        ReadPhysicsSettings();
}
