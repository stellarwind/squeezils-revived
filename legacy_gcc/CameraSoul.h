// CameraSoul, RegionCameraSoul and PlaceCameraSoul: the souls that drive the camera body (CameraBody.h). Reconstructed from SQZ.EXE
// 0x408600-0x40b840 (RTTI classes CameraSoul : SimulationSoul, RegionCameraSoul : CameraSoul and PlaceCameraSoul : CameraSoul; the
// vtables are 0x49d33c / 0x49d314 (CameraSoul), 0x49d3c4 / 0x49d39c (Region) and 0x49d414 / 0x49d3ec (Place)). The two derived classes
// add no fields; everything lives in CameraSoul.
//
// What it does. The camera follows the player's squeezil (the target). Every frame (UpdateRegion, UpdatePlace) the target's matrix
// is turned into the camera's by ComputeView: a point `povOffset` in the target's frame is the point the camera turns about; the
// camera stands `leash` behind it and is pitched and turned about it by angles that blend from the "out" values (the camera far
// from the squeezil: cameraOutPitch/Yaw) to the "pov" values (close behind it) as the squeezil slows down (the leash speeds), plus
// whatever the player looks around by (the mouse; it returns to rest after a while: ReturnLook), plus the extra tilt of a flight
// or a hop. The squeezil's matrix is used with its own up vector, so when the surface it walks on turns the camera turns smoothly:
// a change of up starts a blend (blending) from the previous matrix (fromMatrix) to the target's, over CamInterpTime seconds.
//
// A place's camera can also be put on a path (StartPath): an animation (AnimatedBody with one part, "part") is played and the camera
// follows the part, optionally relative to the player; the player is frozen meanwhile. StopPath gives the camera back, blending
// from where the path left it.
//
// Settings are read from the ini section of the class ("RegionCamera", "PlaceCamera"); the first camera of each kind reads them.
#pragma once
#include "Vector3f.h"
#include "Float3.h"

struct CameraBody;
struct SqueezilBody;
struct SqueezilSoul;
struct AnimatedBody;
struct MorphCycle;
struct Matrix16;

// @00408600 One [RegionCamera] / [PlaceCamera] section (0x60 bytes; the two static copies are at 0x4c64e8 and 0x4c6488)
struct CameraSettings {
    Float3 povOffset;               // 0x00 PovOffsetX/Y/Z: the point the camera turns about, in the target's frame
    int    autoLeashMode;           // 0x0c AutoLeashMode (1): the leash shortens with the squeezil's speed
    int    lookAtReturnMode;        // 0x10 LookAtReturnMode (1): the looking-around is scaled by the same blend
    float  leashLength;             // 0x14 LeashLength (1)
    float  outLeashSpeed;           // 0x18 OutLeashSpeed: below this speed the camera is fully "out"
    float  povLeashSpeed;           // 0x1c PovLeashSpeed (2): at this speed it is "pov"
    float  cameraOutPitch;          // 0x20 CameraOutPitch
    float  cameraPovPitch;          // 0x24 CameraPovPitch
    float  cameraOutYaw;            // 0x28 CameraOutYaw
    float  cameraPovYaw;            // 0x2c CameraPovYaw
    float  lookAtAttenPitch;        // 0x30 LookAtAttenPitch
    float  lookAtAttenYaw;          // 0x34 LookAtAttenYaw
    float  lookAtPitchRange;        // 0x38 LookAtPitchRange: how far up the player may look
    float  lookAtYawRange;          // 0x3c LookAtYawRange (not used)
    float  flyPitchRange;           // 0x40 FlyPitchRange
    float  flyYawRange;             // 0x44 FlyYawRange
    float  maxJumpHeight;           // 0x48 MaxJumpHeight
    float  maxFlyPitchAdjust;       // 0x4c MaxFlyPitchAdjust
    float  maxFlyYawAdjust;         // 0x50 MaxFlyYawAdjust
    float  maxJumpPitchAdjust;      // 0x54 MaxJumpPitchAdjust
    float  lookAtTurnValue;         // 0x58 LookAtTurnValue (4): degrees of looking per unit of mouse movement
    float  returnToBaseTime;        // 0x5c ReturnToBaseTime (0.4): seconds the looking-around takes to come back to rest
};
static_assert(sizeof(CameraSettings) == 0x60, "CameraSettings layout");

// @00408600 (thiscall, ret 8; `this` is not used) reads the section into `out`; also reads [Camera] CamInterpTime (global 0x4c6480)
void ReadCameraSettings(const char *section, CameraSettings *out);

struct CameraSoul {
    void          **vtbl;               // 0x00
    void          **inputVtbl;          // 0x04 the InputEventTarget part (the application is given this address)
    CameraBody     *camera;             // 0x08
    SqueezilBody   *target;             // 0x0c the body followed
    SqueezilSoul   *targetSoul;         // 0x10
    int             enabled;            // 0x14 PlaceCameraSoul: 1 normally, 0 while a path is played (RegionCameraSoul does not set it)
    Vector3f        povOffset;          // 0x18 a copy of the settings' first six words, 0x18 to 0x2c
    int             autoLeashMode;      // 0x24
    int             lookAtReturnMode;   // 0x28
    float           leashLength;        // 0x2c
    float           leashScale;         // 0x30 1
    float           leashBlend;         // 0x34 1 at rest, falling to 0 as the squeezil goes faster than PovLeashSpeed (ComputeView)
    float           lookPitch;          // 0x38 how far the player has looked up (degrees, 0 .. LookAtPitchRange)
    float           lookYaw;            // 0x3c and sideways
    float           returnPitchRate;    // 0x40 degrees per second the looking returns by (set when lookMode becomes 2)
    float           returnYawRate;      // 0x44
    float           unused48[16];       // 0x48 (a Matrix, never used)
    float           fromMatrix[16];     // 0x88 where a blend starts
    float           unusedC8[16];       // 0xc8 (a Matrix, never used)
    float           toMatrix[16];       // 0x108 where it goes (the target's matrix, refreshed every frame of the blend)
    int             lookMode;           // 0x148 0 the player may look about; 1 just let go (the return rates are made); 2 returning
    float           blendTime;          // 0x14c seconds into the blend
    float           blendDuration;      // 0x150
    int             blending;           // 0x154
    int             blendPosition;      // 0x158 1: the camera's position is blended too (paths); 0: it takes the target's at once
    int             justStarted;        // 0x15c 1 until the first frame has been made
    AnimatedBody   *path;               // 0x160 the path being played (never freed)
    int             pathMode;           // 0x164 1: the path is in the world; 2: relative to the player's place; otherwise relative to its matrix
    // total 0x168

    // @00408850 (thiscall, ret 0x48: the result, the settings, the target's matrix by value) The camera's matrix for a target matrix
    Matrix16 ComputeView(const CameraSettings *settings, Matrix16 targetMatrix);
    // @00409620 (ret 4) The matrix between fromMatrix and toMatrix at blendTime / blendDuration
    Matrix16 BlendMatrices();
    // @00409a50 (ret 0x10) Play an animation as the camera's path. The player is frozen; the camera blends to the path over `duration`
    void  StartPath(MorphCycle *clip, int loopMode, int pathMode, float duration);
    // @00409c40 (ret 4) Give the camera back to the player, blending from where it is over `duration`
    void  StopPath(float duration);

    // not in the original as functions of their own (the same code is inlined in both Updates)
    void  CopySettingsHead(const CameraSettings &settings);   // povOffset, the two modes and the leash length
    void  StartBlend();                         // the target's up changed: blend from its previous matrix
    void  ReturnLook(float returnToBaseTime);   // lookMode 1 and 2
};
static_assert(sizeof(CameraSoul) == 0x168, "CameraSoul layout");

struct RegionCameraSoul : CameraSoul {
    RegionCameraSoul *Init(CameraBody *camera, SqueezilBody *target, SqueezilSoul *targetSoul);   // @00409d10 ret 0xc
    RegionCameraSoul *DestroyAndFree(int flags);        // @0040b7c0 slot 0
    void  UpdateRegion();                               // @00409f70 slot 5
    void  OnEvent(int what, const char *text);          // @0040a5e0 InputEventTarget slot 1 (this = soul + 4), ret 8
};

struct PlaceCameraSoul : CameraSoul {
    PlaceCameraSoul *Init(CameraBody *camera, SqueezilBody *target, SqueezilSoul *targetSoul);    // @0040a630 ret 0xc
    PlaceCameraSoul *DestroyAndFree(int flags);         // @0040b830 slot 0
    void  UpdatePlace();                                // @0040a820 slot 5
    void  OnEvent(int what, const char *text);          // @0040b2f0 InputEventTarget slot 1 (this = soul + 4), ret 8
};
// Not here: slots 1 to 4, 6 and 7 do nothing (0x40b350-0x40b630), slot 8 @0040b490 is 5; the InputEventTarget slots 0 and 2 to 8 do
// nothing (0x40b640-0x40b6c0, 0x40a620, 0x40b330)
