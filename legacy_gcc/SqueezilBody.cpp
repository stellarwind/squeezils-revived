// SqueezilBody: see SqueezilBody.h. Reconstructed from SQZ.EXE 0x457050-0x459046.
#include <stdio.h>
#include <string.h>
#include "SqueezilBody.h"
#include "BodyPart.h"
#include "Animation.h"
#include "SqgModel.h"
#include "IniReader.h"
#include "GameString.h"
#include "Scene.h"
#include "Simulation.h"
#include "Original.h"

static void *const kSqueezilBodyVtable = (void *)0x49fe4c;
static const float kRadiansPerDegree = 0.0174533f;     // 0x49fe38
static const float kDegreesPerRadian = 57.2958f;       // 0x49fe1c
static const int   kCameraId = 1;                      // the camera's creature id (the player is 0)

// The settings below are read from the ini by the first body made (see Init). They and the loaded tables stay at their
// original addresses while the class is hooked into the original game; built on their own they are ordinary statics.
static int   &gSettingsRead          = *(int *)0x4bf068;
static float &gDiveAngle             = *(float *)0x4c7b7c;   // [Physics] DiveAngle (default 45): the steepest dive the fly clips show
static float &gClimbAngle            = *(float *)0x4c7b80;   // [Physics] ClimbAngle (default -45)
static float &gMorphTimeScale        = *(float *)0x4c7b84;   // [Defaults] MorphTimeScale (default 2): clip seconds are this many game seconds
static float &gBlendSeconds          = *(float *)0x4c7b88;   // [Camera] CamInterpTime (default 0.5): length of the "up" blend
static float &gFlyBank               = *(float *)0x4c7b8c;   // [Physics] FlyBank (default 60): the steepest bank the fly clips show
static float &gNonFlyMorphRotation   = *(float *)0x4c7b90;   // [Defaults] NonFlyMorphRotation (default -90): the turn of a tilted clip
static float &gDefaultSqueezilMass   = *(float *)0x4c7ba8;   // [Physics] DefaultSqueezilMass (default 0.015625)
static float &gMorphDisableDistance  = *(float *)0x4c7bac;   // [Defaults] MorphDisableDistance (default 15)

static MorphCycle   **const gClips       = (MorphCycle **)0x4bf070;       // [MODE_COUNT + 1] by mode (the last stays NULL)
static AnimatedBody **const gProtoBodies = (AnimatedBody **)0x4bf138;     // [7] by body colour
static Array<SqgModel *> *const gBodyModels = (Array<SqgModel *> *)0x4c7b98;   // every part model the prototypes use

static const char *const kBodyColors[7] = { "BLUE1", "BLUE2", "GREEN1", "GREEN2", "ORANGE1", "ORANGE2", "RED1" };   // 0x49fdd8
static const int kRed1 = 6;

// @0049f860 The clips by mode, and an all-zero row for MODE_NONE.
static const SqueezilClipInfo kClips[MODE_COUNT + 1] = {
    { "stop",          0,   0, 1, 0, 1.0f,  0.15f }, { "scamperstop",   0,   0, 1, 1, 1.0f,  0.15f },
    { "walk",          0,   0, 0, 1, 1.0f, -1.0f },  { "run",           0,   0, 0, 1, 0.75f, -1.0f },
    { "scamper",       0,   0, 0, 1, 0.8f, -1.0f },  { "bump",          0,   0, 1, 1, 1.0f, -1.0f },
    { "skid",          0,   0, 1, 1, 1.0f, -1.0f },  { "idle1",         0,   0, 0, 0, 0.5f, -1.0f },
    { "idle2",         0,   0, 1, 0, 2.0f, -1.0f },  { "sleep",         0,   0, 1, 0, 2.0f, -1.0f },
    { "wave",          0,   0, 0, 0, 1.0f, -1.0f },  { "slide",         0,   0, 0, 1, 1.0f, -1.0f },
    { "spit",          0,   0, 0, 0, 1.0f, -1.0f },  { "hop",           0,   0, 0, 0, 1.0f, -1.0f },
    { "jump",          0,   0, 0, 0, 1.0f, -1.0f },  { "stumble",       0,   0, 0, 0, 1.0f, -1.0f },
    { "fly",           0,   0, 0, 0, 1.0f,  0.3f },  { "flyUp",         0,   0, 0, 0, 1.0f,  0.3f },
    { "flyLeft",       0,   0, 0, 0, 1.0f,  0.3f },  { "flyRight",      0,   0, 0, 0, 1.0f,  0.3f },
    { "flyDown",       0,   0, 0, 0, 1.0f,  0.3f },  { "tumble",        0,   0, 0, 0, 1.0f,  1.0f },
    { "cannon",        0,   0, 1, 0, 0.7f, -1.0f },  { "deadShudder",   0,   0, 0, 0, 1.5f,  1.0f },
    { "deadDrown",     0,   0, 1, 0, 1.0f,  1.2f },  { "deadFaint",     0,   0, 1, 0, 1.0f,  0.5f },
    // pieces: the clip they are cut from and its frames first..last
    { "sleep",         0,  23, 1, 0, 1.0f, -1.0f },  { "sleep",        23, 131, 1, 0, 1.0f, -1.0f },
    { "sleep",       131, 139, 1, 0, 0.8f, -1.0f },
    { "slide",         0,   6, 1, 1, 0.5f, -1.0f },  { "slide",         6,   7, 1, 1, 0.5f, -1.0f },
    { "slide",         8,   9, 1, 1, 0.7f, -1.0f },
    { "spit",          0,   6, 1, 0, 0.5f, -1.0f },  { "spit",          6,   7, 1, 0, 0.1f, -1.0f },
    { "spit",          8,  15, 1, 0, 0.8f, -1.0f },
    { "hop",           0,   3, 1, 0, 0.5f, -1.0f },  { "hop",           3,   4, 1, 0, 0.1f, -1.0f },
    { "hop",           5,   8, 1, 0, 0.9f, -1.0f },  { "hop",           8,  10, 0, 0, 0.2f, -1.0f },
    { "hop",          11,  14, 1, 0, 0.5f, -1.0f },
    { "jump",          0,   4, 1, 1, 0.5f, -1.0f },  { "jump",          4,   5, 1, 1, 0.1f, -1.0f },
    { "jump",          6,   9, 1, 1, 0.8f, -1.0f },
    { "stumble",       0,   5, 0, 1, 0.6f, -1.0f },  { "stumble",       5,   5, 1, 1, 1.0f, -1.0f },
    { "stumble",       5,  32, 1, 1, 0.5f, -1.0f },
    { "deadFaint",     0,  56, 1, 0, 1.0f, -1.0f },
    { "skid",          0,   1, 1, 1, 1.0f, -1.0f },  { "skid",          1,   3, 1, 1, 0.7f, -1.0f },
    { NULL,            0,   0, 0, 0, 0.0f,  0.0f },
};
static const int kWholeClipCount = MODE_SLEEP_1;      // 26 rows read from files, the rest are pieces

static const char *const kPartNames[7] = { "body", "foot_l", "foot_r", "hand_l", "hand_r", "head", "tail" };

// @0047dd20 a turn of `radians` about the y (up) axis
static void YRotationMatrix(float m[16], float radians)
{
    float c = cosf(radians), s = sinf(radians);
    m[0] = c;    m[1] = 0.0f; m[2] = -s;    m[3] = 0.0f;
    m[4] = 0.0f; m[5] = 1.0f; m[6] = 0.0f;  m[7] = 0.0f;
    m[8] = s;    m[9] = 0.0f; m[10] = c;    m[11] = 0.0f;
    m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}

static const float kIdentity[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };

// The seconds the last frame took (the game clock's milliseconds)
static float FrameSeconds()
{
    return (float)FrameMilliseconds() * 0.001f;
}

// ---- the body ----

// @00457050
SqueezilBody *SqueezilBody::Init(const float *matrix, int bodyType)
{
    Body::Init(matrix);
    vtbl = (void **)kSqueezilBodyVtable;
    IniReader ini;
    if (!gSettingsRead) {                     // the first three from [Defaults]
        gMorphTimeScale = ini.GetFloat("MorphTimeScale", 2.0f);
        gNonFlyMorphRotation = ini.GetFloat("NonFlyMorphRotation", -90.0f);
        gMorphDisableDistance = ini.GetFloat("MorphDisableDistance", 15.0f);
        ini.SetSection("Physics");
        gDefaultSqueezilMass = ini.GetFloat("DefaultSqueezilMass", 0.015625f);
        gFlyBank = ini.GetFloat("FlyBank", 60.0f);
        gClimbAngle = ini.GetFloat("ClimbAngle", -45.0f);
        gDiveAngle = ini.GetFloat("DiveAngle", 45.0f);
        ini.SetSection("Camera");
        gBlendSeconds = ini.GetFloat("CamInterpTime", 0.5f);
        gSettingsRead = 1;
    }
    ini.SetSection("Physics");
    collisionRadius = ini.GetFloat("CollisionRadius", 0.5f);
    collisionSpeedAmp = ini.GetFloat("CollisionSpeedAmp", 1.0f);
    extraValue = gDefaultSqueezilMass;
    memcpy(this->matrix, kIdentity, sizeof this->matrix);
    orientation.x = orientation.y = orientation.z = 0.0f;
    clipSpeed = 0.0f;

    frame = GetScene()->CreateSceneFrame();
    mode = MODE_STOP;
    previousMode = MODE_STOP;
    AnimatedBody *copy = (AnimatedBody *)Operator_New(sizeof(AnimatedBody));
    if (copy)
        copy = copy->InitCopy(gProtoBodies[bodyType]);
    character = copy;
    character->Build(frame);
    character->SetCycle(gClips[mode], 0);

    unknown228 = 1;
    blending = 0;
    blendTime = 0.0f;
    noBlend = 1;
    unknown20c = 0;
    return this;
}

// @00457380
void SqueezilBody::Destruct()
{
    vtbl = (void **)kSqueezilBodyVtable;
    if (mode != MODE_NONE)
        character->DestroyFrames(frame);
    if (character) {
        character->Destruct();
        Operator_Delete(character);
    }
    Scene *scene = GetScene();
    DeleteChildFromFrame(scene->GetSceneChild(), frame);
    Body::Destruct();
}

// @00457360
SqueezilBody *SqueezilBody::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @004574b0 The three rotation rows are mixed (blendFrom * (1 - t) + blendTo * t), the position is blendTo's.
float *SqueezilBody::BlendedMatrix(float *out)
{
    Vector3f position(blendTo[12], blendTo[13], blendTo[14]);
    float t = blendTime / gBlendSeconds;
    float s = 1.0f - t;
    Vector3f rows[3];
    for (int r = 0; r < 3; r++) {
        const float *from = &blendFrom[r * 4], *to = &blendTo[r * 4];
        rows[r] = Vector3f(from[0], from[1], from[2]) * s + Vector3f(to[0], to[1], to[2]) * t;
    }
    float m[16] = {
        rows[0].x, rows[0].y, rows[0].z, 0.0f,
        rows[1].x, rows[1].y, rows[1].z, 0.0f,
        rows[2].x, rows[2].y, rows[2].z, 0.0f,
        position.x, position.y, position.z, 1.0f
    };
    OrthonormalizeMatrix(m);
    memcpy(out, m, sizeof m);
    return out;
}

// @004577e0 The matrix is built from the placement's up and direction (right = up x direction) and squared up; the contact
// matrix is then recomputed for the body's new matrix against the face the placement names (the place's walkable surfaces).
void SqueezilBody::Place(BodyPlacement placement)
{
    Vector3f up(placement.up.x, placement.up.y, placement.up.z);
    Vector3f direction(placement.direction.x, placement.direction.y, placement.direction.z);
    Vector3f right = Cross(up, direction);
    Normalize(right);
    matrix[0] = right.x;     matrix[1] = right.y;     matrix[2] = right.z;      matrix[3] = 0.0f;
    matrix[4] = up.x;        matrix[5] = up.y;        matrix[6] = up.z;         matrix[7] = 0.0f;
    matrix[8] = direction.x; matrix[9] = direction.y; matrix[10] = direction.z; matrix[11] = 0.0f;
    matrix[12] = placement.position.x; matrix[13] = placement.position.y; matrix[14] = placement.position.z; matrix[15] = 1.0f;
    OrthonormalizeMatrix(matrix);

    contact.face = placement.contact.face;
    contact.second = placement.contact.second;
    memcpy(contact.matrix, placement.contact.matrix, sizeof contact.matrix);
    void *surfaces = GetPlaceSurfaces(DynamicCast(GetCurrentSimulation(), kLocationSimulationType, kPlaceSimulationType));
    float relative[16];
    FaceRelativeMatrix(NavSurfaceFaceOf(surfaces, &placement.contact), relative, matrix);
    memcpy(contact.matrix, relative, sizeof contact.matrix);
}

// @00457bc0 On the ground a change of "up" (the matrix's second row) starts a blend from the last matrix to the current one, so
// the character tips over smoothly; the frame is put at the blended matrix until the blend has run CamInterpTime seconds.
// Then the clip is played on: by speed on the ground, by climb and bank in the air.
void SqueezilBody::Update()
{
    if (CallVirtual<int>(this, SQB_ON_GROUND)) {
        float *m = CallVirtual<float *>(this, BODY_GET_MATRIX);
        if (!noBlend && (m[4] != lastMatrix[4] || m[5] != lastMatrix[5] || m[6] != lastMatrix[6])) {
            blendTime = 0.0f;
            memcpy(blendFrom, lastMatrix, sizeof blendFrom);
            blending = 1;
        } else {
            unknown20c = 0;
        }
    }

    if (blending) {
        blendTime += (float)FrameMilliseconds() * 0.001f;
        if (blendTime >= gBlendSeconds) {
            blending = 0;
            memcpy(shownMatrix, blendTo, sizeof shownMatrix);
        } else {
            memcpy(blendTo, CallVirtual<float *>(this, BODY_GET_MATRIX), sizeof blendTo);
            float blended[16];
            BlendedMatrix(blended);
            memcpy(shownMatrix, blended, sizeof shownMatrix);
        }
    } else {
        memcpy(shownMatrix, CallVirtual<float *>(this, BODY_GET_MATRIX), sizeof shownMatrix);
        if (noBlend == 1)
            noBlend = 0;
    }
    memcpy(lastMatrix, CallVirtual<float *>(this, BODY_GET_MATRIX), sizeof lastMatrix);

    if (mode == MODE_NONE)
        return;
    float seconds = FrameSeconds();
    if (CallVirtual<int>(this, SQB_ON_GROUND))
        AnimateOnGround(seconds);
    else
        AnimateInAir(seconds);
}

// @00458ec0 The pose is only applied near the camera (the fly pose was already mixed by AnimateInAir); the frame always follows.
void SqueezilBody::UpdateFrame()
{
    if (mode != MODE_NONE && IsNearCamera()) {
        if (mode == MODE_FLY)
            character->ApplyPose();
        else
            character->Advance(0.0f, 0);
    }
    GetScene()->SetFrameTransform(frame, shownMatrix);
}

// @00459000
int SqueezilBody::IsOnSurface()
{
    return CallVirtual<int>(this, SQB_ON_GROUND);
}

// @00457780
void SqueezilBody::SetOrientation(float pitch, float yaw, float roll)
{
    orientation.x = pitch;
    orientation.y = yaw;
    orientation.z = roll;
}

// @004577b0
void SqueezilBody::GetOrientation(float *pitch, float *yaw, float *roll)
{
    *pitch = orientation.x;
    *yaw = orientation.y;
    *roll = orientation.z;
}

// @004579f0 Pitch is kept in -90..270 and roll in -180..180 (one turn added or taken away); yaw is left as it comes.
void SqueezilBody::UpdateOrientation()
{
    float pitch, yaw, roll;
    MatrixToAngles(matrix, &pitch, &yaw, &roll);
    pitch *= kDegreesPerRadian;
    yaw *= kDegreesPerRadian;
    roll *= kDegreesPerRadian;
    if (pitch > 270.0f)
        pitch -= 360.0f;
    if (pitch < -90.0f)
        pitch += 360.0f;
    if (roll > 180.0f)
        roll -= 360.0f;
    if (roll < -180.0f)
        roll += 360.0f;
    CallVirtual<void>(this, SQB_SET_ORIENTATION, pitch, yaw, roll);
}

// @00458820 The character is put into the scene when it leaves MODE_NONE and taken out when it enters it. A tilted clip turns
// the character by NonFlyMorphRotation.
void SqueezilBody::SetMode(int newMode, int blend, float startTime)
{
    if (mode == newMode)
        return;
    float tilted[16];
    YRotationMatrix(tilted, gNonFlyMorphRotation * kRadiansPerDegree);
    float straight[16];
    memcpy(straight, kIdentity, sizeof straight);
    if (mode == MODE_NONE)
        character->Build(frame);
    if (newMode != MODE_NONE) {
        character->SetCycle(gClips[newMode], blend);
        character->SetTime(startTime);
        if (kClips[newMode].tilted)
            character->SetRootTransform(frame, tilted);
        else
            character->SetRootTransform(frame, straight);
    } else {
        character->DestroyFrames(frame);
    }
    previousMode = mode;
    mode = newMode;
}

// @00457ad0
int SqueezilBody::ClipEnded()
{
    float time = character->GetTime();
    return character->GetDuration() == time;
}

// @00457b10
Vector3f *SqueezilBody::HeadPosition(Vector3f *out)
{
    float m[16];
    character->pose->Find("head")->ToMatrix(m);
    out->x = m[12];
    out->y = m[13];
    out->z = m[14];
    return out;
}

// @004589a0 "Behind" is measured along the camera's direction: up to 2 units behind is still near.
int SqueezilBody::IsNearCamera()
{
    LocationSimulation *sim = (LocationSimulation *)GetCurrentSimulation();
    void *camera = sim->GetActor(kCameraId);
    float *cameraMatrix = ActorMatrix(camera);
    Vector3f offset = Vector3f(matrix[12], matrix[13], matrix[14]) - Vector3f(cameraMatrix[12], cameraMatrix[13], cameraMatrix[14]);
    if (offset.y * offset.y + offset.z * offset.z + offset.x * offset.x > gMorphDisableDistance * gMorphDisableDistance)
        return 0;
    float ahead = cameraMatrix[8] * offset.x + cameraMatrix[9] * offset.y + cameraMatrix[10] * offset.z;
    if (ahead < 0.0f && ahead * ahead > 4.0f)
        return 0;
    return 1;
}

// @00458aa0 Walking, running and scampering play faster the faster the body moves compared with clipSpeed (backwards when it
// moves against its facing). While a clip change blends, the tilt is turned gradually from the old clip's to the new one's.
void SqueezilBody::AnimateOnGround(float seconds)
{
    float timeScale = gMorphTimeScale;
    if (mode == MODE_WALK || mode == MODE_SCAMPER || mode == MODE_RUN) {
        if (CallVirtual<float>(this, SQB_GET_CLIP_SPEED) > 0.0f && CallVirtual<float>(this, BODY_GET_FACING_VALUE) > 0.01f) {
            float rate = CallVirtual<float>(this, BODY_GET_FACING_VALUE) / CallVirtual<float>(this, SQB_GET_CLIP_SPEED);
            if (facing.z * matrix[10] + facing.y * matrix[9] + facing.x * matrix[8] < 0.0f)
                rate = -rate;
            timeScale /= rate;
        }
    }
    character->Advance(seconds / timeScale, 1);

    float blendFraction = character->GetBlendFraction();
    if (blendFraction == -1.0f) {
        float m[16];
        YRotationMatrix(m, (kClips[mode].tilted ? gNonFlyMorphRotation : 0.0f) * kRadiansPerDegree);
        character->SetRootTransform(frame, m);
    } else if (kClips[mode].tilted != kClips[previousMode].tilted) {
        float before = kClips[previousMode].tilted ? gNonFlyMorphRotation : 0.0f;
        float after = kClips[mode].tilted ? gNonFlyMorphRotation : 0.0f;
        float m[16];
        YRotationMatrix(m, ((after - before) * blendFraction + before) * kRadiansPerDegree);
        character->SetRootTransform(frame, m);
    }
}

// @00458cd0 The fly pose is the "fly" clip's first frame, then flyUp or flyDown mixed in by the pitch (as a share of ClimbAngle
// or DiveAngle) and flyLeft (roll below 0) or flyRight by the roll (as a share of FlyBank). The tumbling, cannon and mid-jump clips just play.
// (The original tests the signs on the floats' bit patterns; -0 counts as positive either way.)
void SqueezilBody::AnimateInAir(float seconds)
{
    if (mode == MODE_TUMBLE || mode == MODE_CANNON || mode == MODE_HOP_4 || mode == MODE_JUMP_1 || mode == MODE_JUMP_2) {
        character->Advance(seconds / gMorphTimeScale, 1);
        return;
    }
    float pitch = orientation.x, roll = orientation.z;
    float up = 0.0f, down = 0.0f, left = 0.0f, right = 0.0f;
    if (pitch < gClimbAngle)
        pitch = gClimbAngle;
    if (pitch > gDiveAngle)
        pitch = gDiveAngle;
    if (pitch < 0.0f)
        up = pitch / gClimbAngle;
    else
        down = pitch / gDiveAngle;
    if (roll < -gFlyBank)
        roll = -gFlyBank;
    if (roll > gFlyBank)
        roll = gFlyBank;
    if (roll < 0.0f)
        left = -(roll / gFlyBank);
    else
        right = roll / gFlyBank;

    PoseTable *pose = character->pose;
    gClips[MODE_FLY]->SamplePose(pose, 0.0f, -1);
    if (up > 0.0f)
        gClips[MODE_FLY_UP]->BlendPose(pose, 0.0f, up, -1);
    if (down > 0.0f)
        gClips[MODE_FLY_DOWN]->BlendPose(pose, 0.0f, down, -1);
    if (left > 0.0f)
        gClips[MODE_FLY_LEFT]->BlendPose(pose, 0.0f, left, -1);
    if (right > 0.0f)
        gClips[MODE_FLY_RIGHT]->BlendPose(pose, 0.0f, right, -1);
}

// ---- prototypes and clips ----

// @00458050
int LoadProtoBodies()
{
    for (int i = 0; i < 7; i++) {
        GameLog("Loading %s proto body...\n", kBodyColors[i]);
        gProtoBodies[i] = LoadProtoBody(kBodyColors[i]);
        if (!gProtoBodies[i])
            return 0;
        GameLog("OK.\n");
    }
    return 1;
}

// @004580a0 (the clip pointers are not cleared)
void FreeProtoBodies()
{
    for (int i = 0; i < 7; i++) {
        AnimatedBody *body = gProtoBodies[i];
        if (body) {
            body->Destruct();
            Operator_Delete(body);
            gProtoBodies[i] = NULL;
        }
    }
    for (int i = 0; i < gBodyModels->count; i++) {
        SqgModel *model = gBodyModels->data[i];
        if (model) {
            SqgModelDestruct(model);
            Operator_Delete(model);
        }
    }
    for (int i = 0; i < MODE_COUNT + 1; i++) {
        MorphCycle *clip = gClips[i];
        if (clip)
            delete clip;
    }
}

static void DeleteProtoBody(AnimatedBody *body, SqgModel *model)
{
    if (body) {
        body->Destruct();
        Operator_Delete(body);
    }
    if (model) {
        SqgModelDestruct(model);
        Operator_Delete(model);
    }
}

// @00458130 A model that loaded is kept on the model list; on a failure the body and the model being loaded are deleted (the
// parts already added go with the body, their models stay on the list).
AnimatedBody *LoadProtoBody(const char *color)
{
    IniReader ini;
    ini.SetSection("Cycles");
    StringList parts;
    for (int i = 0; i < 7; i++)
        parts.Insert(String(kPartNames[i]));
    AnimatedBody *body = (AnimatedBody *)Operator_New(sizeof(AnimatedBody));
    body = body ? body->Init() : NULL;

    for (int i = 0; i < parts.count; i++) {
        void *memory = Operator_New(0x2c);
        SqgModel *model = memory ? SqgModelConstruct(memory) : NULL;
        char key[80];
        sprintf(key, "%s_%s", color, parts.Get(i).c_str());
        GameLog("\tLoading morph body part %s... ", key);
        const char *file = ini.GetString(key);
        if (strlen(file) == 0) {
            GameLog("ERROR: no filename entry!\n");
            DeleteProtoBody(body, model);
            return NULL;
        }
        if (!model->Load(file, 0)) {
            GameLog("ERROR: Failed to load %s from %s!\n", key, file);
            DeleteProtoBody(body, model);
            return NULL;
        }
        BodyPart *part = (BodyPart *)Operator_New(sizeof(BodyPart));
        if (part)
            part = part->Init(model);
        body->AddPart(parts.Get(i).c_str(), part);
        GameLog("OK.\n");
        gBodyModels->Insert(model);
    }
    return body;
}

// @00458700
int LoadCycle(IniReader *ini, const char *name, const SqueezilClipInfo *info, MorphCycle **out)
{
    GameLog("Loading cycle %s... ", name);
    *out = new MorphCycle(info->noLoop, info->speed, info->period);
    const char *file = ini->GetString(name);
    if (strlen(file) == 0) {
        GameLog("ERROR: no filename entry!\n");
    } else if (!MorphCycleLoad(*out, file)) {
        GameLog("ERROR: couldn't load from %s!\n", file);
    } else {
        GameLog("OK.\n");
        return 1;
    }
    if (*out)
        delete *out;
    *out = NULL;
    return 0;
}

// @00458590 A piece is cut from the first whole clip with its name (none found: the row after the whole clips, as the original).
int LoadCycles()
{
    IniReader ini;
    ini.SetSection("Cycles");
    for (int i = 0; i < kWholeClipCount; i++) {
        if (!LoadCycle(&ini, kClips[i].name, &kClips[i], &gClips[i]))
            return 0;
    }
    for (int i = kWholeClipCount; i < MODE_COUNT; i++) {
        int whole = 0;
        while (whole < kWholeClipCount && strcmp(kClips[whole].name, kClips[i].name) != 0)
            whole++;
        const SqueezilClipInfo &piece = kClips[i];
        gClips[i] = new MorphCycle(*gClips[whole], piece.first, piece.last, piece.noLoop, piece.speed, piece.period);
    }
    return 1;
}

// ---- body colours ----

// @00458fd0
const char *BodyColorName(int color)
{
    return kBodyColors[color];
}

// @00458f20
int FindBodyColor(const char *name)
{
    int i;
    for (i = 0; i < 7; i++) {
        if (_stricmp(name, BodyColorName(i)) == 0)
            break;
    }
    return i == 7 ? 8 : i;
}

// @00458f60
int ReadBodyColor(IniReader *ini)
{
    const char *name = ini->GetString("BodyColor");
    if (strlen(name) == 0) {
        name = kBodyColors[kRed1];
        GameLog("WARNING: no body color specified, using %s.\n", name);
    }
    int color = FindBodyColor(name);
    if (color == 8) {
        color = kRed1;
        GameLog("WARNING: unrecognized body color %s specified, using %s.\n", name, BodyColorName(color));
    }
    return color;
}
