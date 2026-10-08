// SqueezilBody: the body of a squeezil (the player and the other creatures of its kind). Reconstructed from SQZ.EXE
// 0x457050-0x459046.
//
// It is a Body plus an animated character (AnimatedBody, BodyPart.h) hung on its own scene frame. The character is copied from
// one of seven prototype bodies, one per body colour, which are loaded once at start-up together with the 49 animation clips
// ("modes") every squeezil shares. The body's soul (SqueezilSoul, not reconstructed) picks the mode; the body plays it:
//   - on the ground the walking clips are sped up or slowed down to match how fast the squeezil really moves, and the frame
//     follows the matrix with a short blend whenever the "up" direction changes (walking over a bend in a surface),
//   - in the air the "fly" clip is mixed with flyUp / flyDown / flyLeft / flyRight by how steeply it climbs and how far it banks,
//   - far from the camera, or behind it, the pose is not worked out at all.
//
// "Collision": the per-body CollisionRadius and CollisionSpeedAmp settings ([Physics]) live in the first two fields. Nothing in
// this class uses them; the souls' physics code reads them straight from the object (CollisionSpeedAmp at 0x44727e and
// 0x4474aa). The other body classes have a word called "flags" in the same place (Body.h).
//
// Like the other bodies the object is made and used by original code through the vtable at +0 (see VirtualCall.h), so the
// virtual functions are plain members here; SqueezilBodySlot lists their slot numbers.
#pragma once
#include "Body.h"
#include "Vector3f.h"

struct IDirect3DRMFrame;
struct AnimatedBody;
struct MorphCycle;
class IniReader;

// The clips, by mode number (the order of the static table at 0x49f860). 0..25 are loaded from the files named in the [Cycles]
// section; 26..48 are pieces of one of those (the frame ranges are in the table in SqueezilBody.cpp; their meaning is guessed
// from the clip names only).
enum SqueezilMode {
    MODE_STOP, MODE_SCAMPER_STOP, MODE_WALK, MODE_RUN, MODE_SCAMPER, MODE_BUMP, MODE_SKID, MODE_IDLE1, MODE_IDLE2, MODE_SLEEP,
    MODE_WAVE, MODE_SLIDE, MODE_SPIT, MODE_HOP, MODE_JUMP, MODE_STUMBLE, MODE_FLY, MODE_FLY_UP, MODE_FLY_LEFT, MODE_FLY_RIGHT,
    MODE_FLY_DOWN, MODE_TUMBLE, MODE_CANNON, MODE_DEAD_SHUDDER, MODE_DEAD_DROWN, MODE_DEAD_FAINT,
    MODE_SLEEP_1, MODE_SLEEP_2, MODE_SLEEP_3,                     // 26 frames 0-23, 23-131, 131-139 of "sleep"
    MODE_SLIDE_1, MODE_SLIDE_2, MODE_SLIDE_3,                     // 29
    MODE_SPIT_1, MODE_SPIT_2, MODE_SPIT_3,                        // 32
    MODE_HOP_1, MODE_HOP_2, MODE_HOP_3, MODE_HOP_4, MODE_HOP_5,   // 35 (HOP_4 is the only looping piece)
    MODE_JUMP_1, MODE_JUMP_2, MODE_JUMP_3,                        // 40
    MODE_STUMBLE_1, MODE_STUMBLE_2, MODE_STUMBLE_3,               // 43
    MODE_DEAD_FAINT_1,                                            // 46
    MODE_SKID_1, MODE_SKID_2,                                     // 47
    MODE_COUNT,
    MODE_NONE = MODE_COUNT        // 49: no clip; the character is not in the scene
};

// One row of the static clip table (0x1c bytes)
struct SqueezilClipInfo {
    const char *name;     // 0x00 the [Cycles] key of the clip; for a piece, the clip it is cut from
    int   first, last;    // 0x04 a piece's frame range (unused for a whole clip)
    int   noLoop;         // 0x0c
    int   tilted;         // 0x10 the character is turned by NonFlyMorphRotation about its up axis while this clip plays
    float speed;          // 0x14
    float period;         // 0x18 seconds per frame (-1: the file's / the whole clip's)
};

// The placement handed to Place (0xb0 bytes, by value). Only the fields from 0x44 on are read.
struct BodyPlacement {
    int     unknown00;        // 0x00
    float   matrix[16];       // 0x04
    Contact contact;          // 0x44
    Float3  position;         // 0x8c
    Float3  up;               // 0x98
    Float3  direction;        // 0xa4
};
static_assert(sizeof(BodyPlacement) == 0xb0, "BodyPlacement layout");

enum SqueezilBodySlot {
    SQB_DESTROY = 0, SQB_UPDATE = 1, SQB_UPDATE_FRAME = 2, SQB_ON_SURFACE = 8, SQB_ON_GROUND = 15, SQB_SET_ON_GROUND = 16,
    SQB_SET_ORIENTATION = 17, SQB_GET_ORIENTATION = 18, SQB_UPDATE_ORIENTATION = 19, SQB_GET_MODE = 20, SQB_SET_MODE = 21,
    SQB_CLIP_ENDED = 22, SQB_HEAD_POSITION = 23, SQB_SET_CLIP_SPEED = 24, SQB_GET_CLIP_SPEED = 25, SQB_GET_PITCH = 26,
    SQB_GET_ROLL = 27
};

struct SqueezilBody : Body {
    float collisionRadius;          // 0xb8 [Physics] CollisionRadius (default 0.5)
    float collisionSpeedAmp;        // 0xbc [Physics] CollisionSpeedAmp (default 1)
    float lastMatrix[16];           // 0xc0 the body's matrix at the end of the last update
    float shownMatrix[16];          // 0x100 where the character's frame is put
    float blendFrom[16];            // 0x140 the matrix the blend starts from
    float unused180[16];            // 0x180 never used
    float blendTo[16];              // 0x1c0 the matrix the blend goes to (the body's matrix, refreshed every update)
    float blendTime;                // 0x200 seconds into the blend
    int   blending;                 // 0x204
    int   noBlend;                  // 0x208 set: the next change of "up" is taken at once (cleared by the next update that does not blend)
    int   unknown20c;               // 0x20c cleared by Update, set by nothing here
    Float3 orientation;             // 0x210 pitch, yaw, roll in degrees (UpdateOrientation)
    float clipSpeed;                // 0x21c the speed the walking clips are made for (set by the soul; meaning inferred)
    IDirect3DRMFrame *frame;        // 0x220 the character's frame, under the scene child
    int   onGround;                 // 0x224 set by the soul (the constructor leaves it as it was)
    int   unknown228;               // 0x228 set to 1 by the constructor, read by nothing here
    AnimatedBody *character;        // 0x22c
    int   mode;                     // 0x230 SqueezilMode
    int   previousMode;             // 0x234

    // @00457050 bodyType: the body colour (ReadBodyColor). The matrix only gives the starting facing; the body's own matrix
    // starts as the identity (the soul places it afterwards).
    SqueezilBody *Init(const float *matrix, int bodyType);
    void  Destruct();                               // @00457380
    SqueezilBody *DestroyAndFree(int flags);        // @00457360 slot 0 (bit 0 of flags: free the memory)
    float *BlendedMatrix(float *out);               // @004574b0 blendFrom -> blendTo by blendTime, squared up
    void  Place(BodyPlacement placement);           // @004577e0 (not virtual) matrix and contact from a placement

    void  Update();                                 // @00457bc0 slot 1
    void  UpdateFrame();                            // @00458ec0 slot 2
    int   IsOnSurface();                            // @00459000 slot 8: the same as slot 15
    int   IsOnGround()          { return onGround; }       // @00458fe0 slot 15
    void  SetOnGround(int on)   { onGround = on; }         // @00458ff0 slot 16
    void  SetOrientation(float pitch, float yaw, float roll);       // @00457780 slot 17
    void  GetOrientation(float *pitch, float *yaw, float *roll);    // @004577b0 slot 18
    void  UpdateOrientation();                      // @004579f0 slot 19: from the matrix, in degrees
    int   GetMode()             { return mode; }           // @00457ac0 slot 20
    void  SetMode(int newMode, int blend, float startTime);         // @00458820 slot 21
    int   ClipEnded();                              // @00457ad0 slot 22: the clip's time is exactly its duration
    Vector3f *HeadPosition(Vector3f *out);          // @00457b10 slot 23: the head part's position in the character's frame
    void  SetClipSpeed(float s) { clipSpeed = s; }         // @00459010 slot 24
    float GetClipSpeed()        { return clipSpeed; }      // @00459020 slot 25
    float GetPitch()            { return orientation.x; }  // @00459030 slot 26
    float GetRoll()             { return orientation.z; }  // @00459040 slot 27

    int   IsNearCamera();                           // @004589a0 within MorphDisableDistance of the camera and at most 2 behind it
    void  AnimateOnGround(float seconds);           // @00458aa0
    void  AnimateInAir(float seconds);              // @00458cd0
};
static_assert(sizeof(SqueezilBody) == 0x238, "SqueezilBody layout");

// The shared data, loaded once at start-up (called from 0x42041f / 0x42042f, freed from 0x42065d)
int   LoadProtoBodies();                            // @00458050 the seven colours; 0 when one fails
AnimatedBody *LoadProtoBody(const char *color);     // @00458130 the seven parts "<color>_<part>" from [Cycles]; NULL on failure
void  FreeProtoBodies();                            // @004580a0 the bodies, their models and the clips
int   LoadCycles();                                 // @00458590 the 26 clip files and the 23 pieces; 0 when one fails
int   LoadCycle(IniReader *ini, const char *name, const SqueezilClipInfo *info, MorphCycle **out);   // @00458700

// Body colours: BLUE1 BLUE2 GREEN1 GREEN2 ORANGE1 ORANGE2 RED1
const char *BodyColorName(int color);               // @00458fd0
int   FindBodyColor(const char *name);              // @00458f20 any case; 8 when unknown
int   ReadBodyColor(IniReader *ini);                // @00458f60 the BodyColor key of the current section (RED1 when missing or unknown)
