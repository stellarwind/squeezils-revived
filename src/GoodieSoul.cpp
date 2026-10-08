// GoodieSoul: see GoodieSoul.h. Reconstructed from SQZ.EXE 0x459590-0x45a840.
#include "GoodieSoul.h"
#include "SqueezilSoulInternal.h"
#include "GoodieBody.h"
#include "PlaceSimulation.h"
#include "NavSurface.h"
#include "NavMesh.h"
#include "World.h"
#include "IniReader.h"
#include "Original.h"
#include "VirtualCall.h"
#include <string.h>
#include <new>
#include <math.h>

static_assert(offsetof(GoodieSoul, name) == 0x0c, "GoodieSoul layout");
static_assert(offsetof(GoodieSoul, direction) == 0x20, "GoodieSoul layout");
static_assert(offsetof(GoodieSoul, rebudTimer) == 0x2c, "GoodieSoul layout");

static void **const kGoodieSoulVtable   = (void **)0x49ff44;
static void **const kGoodieGrantVtable  = (void **)0x49ff24;
static void **const kMessageMapVtable   = (void **)0x49d2b4;
static void **const kGrantMessageMapVtable = (void **)0x49f844;

static GoodieSettings *&gSettings = *(GoodieSettings **)0x4c7bb4;   // points to kSettingsBlock once the first goodie is made
static GoodieSettings *const kSettingsBlock = (GoodieSettings *)0x4c7bc0;
static int   &gSettingsRead = *(int *)0x4bf354;
static float &gMass         = *(float *)0x4c7bb8;       // GoodieMass (0.0625): the body's +0x6c
static float &gRebudPlane   = *(float *)0x4c7bb0;       // GoodieRebudPlane (-10000): below this height a goodie is removed
static int   &gSprayCounter = *(int *)0x4bf358;         // which of the 16 sprayed courses comes next

static const float kStill = 0.1f;                       // 0x49ff14: a goodie whose direction is shorter than this lies still
static const float kRestSpeed = 0.01f;                  // 0x49fee0: a hitter whose direction is shorter than this is not moving

// Reaches the body through its own table (the body is an original object, see VirtualCall.h)
namespace GoodieBodyCall {
inline float   *Matrix(GoodieBody *b)               { return CallVirtual<float *>(b, BODY_GET_MATRIX); }
inline Float3  *Facing(GoodieBody *b)               { return CallVirtual<Float3 *>(b, BODY_GET_FACING); }
inline float    Speed(GoodieBody *b)                { return CallVirtual<float>(b, BODY_GET_FACING_VALUE); }
inline void     SetMatrix(GoodieBody *b, const float *m)               { CallVirtual<void>(b, BODY_SET_MATRIX, m); }
inline void     SetFacing(GoodieBody *b, const Float3 *d, float speed) { CallVirtual<void>(b, BODY_SET_FACING, d, speed); }
inline void     SetContact(GoodieBody *b, const Contact *c)            { CallVirtual<void>(b, BODY_SET_CONTACT, c); }
}

// @004597b0
void ReadGoodieSettings()
{
    IniReader ini;
    ini.SetSection("GoodiePhysics");
    gSettingsRead = 1;
    GoodieSettings *s = gSettings;
    s->physical = *(PhysicalSettings *)0x4c7768;
    s->gravityFactor = ini.GetFloat("GoodieGravityFactor", 1.0f);
    s->bounceCount = ini.GetInt("BounceCount", 3);
    s->bounceDamping = ini.GetFloat("BounceDamping", 0.5f);
    s->collisionSpeedAmp = ini.GetFloat("CollisionSpeedAmp", 1.0f);
    s->sprayAngle = ini.GetFloat("AngleOfSpray", 45.0f);
    s->spraySpeed = ini.GetFloat("InitialSpraySpeed", 5.0f);
    s->sprayOffset = ini.GetFloat("InitialSprayPosOffset", 1.0f);
    gMass = ini.GetFloat("GoodieMass", 0.0625f);
    gRebudPlane = ini.GetFloat("GoodieRebudPlane", -10000.0f);
}

// @00459590
GoodieSoul *GoodieSoul::Init(GoodieBody *theBody, const char *theName, int theKind)
{
    PhysicalSoulInit(this);
    grantVtbl = kMessageMapVtable;
    grantVtbl = kGrantMessageMapVtable;
    new (&name) String();
    vtbl = kGoodieSoulVtable;
    body = theBody;
    grantVtbl = kGoodieGrantVtable;
    name = String(theName);

    if (!gSettingsRead) {
        gSettings = kSettingsBlock;
        ReadGoodieSettings();
    }
    theBody->extraValue = gMass;                                    // the body's mass
    memcpy(&theBody->unused_bc, &gSettings->collisionSpeedAmp, sizeof(int));   // (the collision code reads the field as a float)
    direction.x = direction.y = direction.z = 0.0f;
    thrown = 0;
    bounces = 0;
    rebudDelay = 0.0f;
    rebudTimer = -1.0f;
    kind = theKind;
    return this;
}

// @004596e0 slot 0
GoodieSoul *GoodieSoul::DestroyAndFree(int flags)
{
    name.~String();
    grantVtbl = kMessageMapVtable;      // GrantMessageMap @00450980
    vtbl = kMessageMapVtable;           // PhysicalSoul @00450e40, down to MessageMap
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00459960 slot 1
void *GoodieSoul::GetInterface(int which)
{
    if (which == 1)
        return this ? &grantVtbl : NULL;
    return NULL;
}

// @00459980 slot 3
int GoodieSoul::IsLoose() const { return kind == 0; }

// @00459bf0 slot 8
int GoodieSoul::GetKind() const { return kind == 0 ? 2 : 4; }

// @0045a840 slot 10
GoodieSettings *GoodieSoul::GetSettings() { return gSettings; }

// @0045a830 slot 11
int GoodieSoul::IsAttacking() const { return thrown != 0; }

// @00459990 (GrantMessageMap slot 3, this = soul + 4)
int GoodieSoul::GrantSlot3(int arg) const
{
    if (kind != 0)
        return arg == 0;
    return thrown == 0;
}

// @004599b0 slot 4 A creature touched the goodie. One that takes goodies (the interface answers slot 2 for the goodie's kind) is
// given it, unless the goodie was thrown (then it only bounces off); the goodie is removed once given.
void GoodieSoul::Touch(int otherId)
{
    LocationSimulation *sim = (LocationSimulation *)GetCurrentSimulation();
    void *other = sim->GetSoul(otherId);
    void *grantPart = CallVirtual<void *>(other, 1, 1);                                 // GetInterface(1)
    void *taker = DynamicCast(grantPart, kMessageMapType, kGrantMessageMapType);
    if (!taker)
        return;
    if (CallVirtual<int>(taker, 2, kind) && !thrown) {
        if (CallVirtual<int>(taker, 4, name.c_str(), sim->FindIdBySoul(this))) {        // Grant(goodie, value)
            sim->AddId(sim->FindIdBySoul(this));
            return;
        }
        PhysicalCollision(sim->FindIdByActor(body), otherId);
        return;
    }
    PhysicalCollision(sim->FindIdByActor(body), otherId);
    thrown = 0;
}

// @00459b80 (GrantMessageMap slot 5, this = soul + 4) Give the goodie to the creature
int GoodieSoul::GrantTo(int otherId)
{
    LocationSimulation *sim = (LocationSimulation *)GetCurrentSimulation();
    void *other = sim->GetSoul(otherId);
    void *taker = CallVirtual<void *>(other, 1, 1);                                     // GetInterface(1)
    if (CallVirtual<int>(taker, 4, name.c_str(), sim->FindIdBySoul(this))) {
        sim->AddId(sim->FindIdBySoul(this));
        return 1;
    }
    return 0;
}

// @00459a80 slot 9 The goodie was hit (by creature byId): a loose goodie at rest flies off the way the hitter was going, at its speed.
// Either way it is no longer thrown.
void GoodieSoul::Hit(int knock, int, int byId)
{
    if (knock && kind == 0) {
        if (Length(ToVector(*GoodieBodyCall::Facing(body))) < kRestSpeed) {
            LocationSimulation *sim = (LocationSimulation *)GetCurrentSimulation();
            void *hitter = sim->GetActor(byId);
            float speed = CallVirtual<float>(hitter, BODY_GET_FACING_VALUE);
            Vector3f away = ToVector(*CallVirtual<Float3 *>(hitter, BODY_GET_FACING)) * -1.0f;
            Float3 awayDirection = ToFloat3(away);
            GoodieBodyCall::SetFacing(body, &awayDirection, speed);
        }
        Float3 *facing = GoodieBodyCall::Facing(body);
        direction = ToVector(*facing);
        flightTime = 0.0f;
    }
    thrown = 0;
}

// @00459c00 The sprayed goodies fly off on 16 courses in turn (8 headings, then the same turned by 22.5 degrees): the squeezil's
// matrix, turned by the heading about the vertical and up by AngleOfSpray, with the start moved sprayOffset ahead.
void GoodieSoul::Spray(Matrix16 from)
{
    GoodieSettings *s = gSettings;
    float offset = s->sprayOffset;

    float shift[16];
    memset(shift, 0, sizeof shift);
    shift[0] = shift[5] = shift[10] = shift[15] = 1.0f;
    shift[14] = offset;                         // moved ahead along z

    static const float kHeadings[8] = { 0.0f, 180.0f, 90.0f, 270.0f, 45.0f, 225.0f, 135.0f, 315.0f };   // jump table @00459eac
    float angle = kHeadings[gSprayCounter % 8];
    if (gSprayCounter > 7)
        angle += 22.5f;                         // 0x49fefc

    float turnY[16], turnX[16], step1[16], step2[16], placed[16];
    RotationY(turnY, angle * kRadiansPerDegree);
    RotationX(turnX, (360.0f - s->sprayAngle) * kRadiansPerDegree);
    MultiplyMatrix(step1, shift, turnX);
    MultiplyMatrix(step2, step1, turnY);
    MultiplyMatrix(placed, step2, from.m);

    GoodieBodyCall::SetMatrix(body, placed);
    direction.x = placed[8];
    direction.y = placed[9];
    direction.z = placed[10];
    Normalize(direction);

    gSprayCounter++;
    if (gSprayCounter == 16)
        gSprayCounter = 0;
    Float3 course = ToFloat3(direction);
    GoodieBodyCall::SetFacing(body, &course, s->spraySpeed);
    flightTime = 0.0f;
}

// @00459ed0 Thrown from a matrix in a direction at a speed
void GoodieSoul::Launch(Matrix16 from, Float3 course, float speed)
{
    GoodieBodyCall::SetMatrix(body, from.m);
    direction.x = course.x;
    direction.y = course.y;
    direction.z = course.z;
    Normalize(direction);
    Float3 unit = ToFloat3(direction);
    GoodieBodyCall::SetFacing(body, &unit, speed);
    flightTime = 0.0f;
    thrown = 1;
}

// @00459f80 slot 5 One frame of flight
void GoodieSoul::UpdateGoodie()
{
    LocationSimulation *sim = (LocationSimulation *)GetCurrentSimulation();
    PlaceSimulation *place = (PlaceSimulation *)DynamicCast(sim, kLocationSimulationType, kPlaceSimulationType);
    float seconds = FrameSeconds();

    if (kind) {
        Float3 *facing = GoodieBodyCall::Facing(body);
        GoodieBodyCall::SetFacing(body, facing, 0.0f);          // held still
    }

    if (rebudTimer != -1.0f) {
        rebudTimer += seconds;
        if (!(rebudTimer < rebudDelay)) {
            // it has stayed on a rebud face long enough: gone (the level may put it back)
            rebudTimer = -1.0f;
            rebudDelay = 0.0f;
            PlaceLevelForgetCreature(place->GetLevel(), sim->FindIdBySoul(this));
            sim->AddId(sim->FindIdBySoul(this));
            return;
        }
    }

    if (!(Length(direction) > kStill))
        return;                                                 // lying still
    flightTime += seconds;
    if (flightTime == 0.0f)
        return;

    GoodieSettings *s = gSettings;
    float speed = GoodieBodyCall::Speed(body);
    Vector3f step = (direction * speed) * seconds;
    float t = flightTime;
    step.y = ((t * t - (t - seconds) * (t - seconds)) * (s->gravityFactor * s->physical.gravity.y)) * 0.5f + step.y;

    Vector3f course = step;
    float distance = Length(course);
    Normalize(course);
    Float3 courseF = ToFloat3(course);
    GoodieBodyCall::SetFacing(body, &courseF, GoodieBodyCall::Speed(body));

    Contact newContact;
    memset(&newContact, 0, sizeof newContact);
    newContact.face = -1;
    newContact.second = 0;
    Float3 hitPos, newRight, newUp, newForward;
    memset(&hitPos, 0, sizeof hitPos);
    memset(&newRight, 0, sizeof newRight);
    memset(&newUp, 0, sizeof newUp);
    memset(&newForward, 0, sizeof newForward);
    float *m = GoodieBodyCall::Matrix(body);
    int hit = place->GetSurfaces()->FlightHitsSurface((Float3 *)(m + 12), (Float3 *)(m + 0), (Float3 *)(m + 4), (Float3 *)(m + 8),
                                                      GoodieBodyCall::Facing(body), distance, &hitPos, &newRight, &newUp, &newForward,
                                                      &newContact);
    if (hit && place->GetSurfaces()->FlyThrough(&newContact))
        hit = 0;

    if (hit) {
        bounces++;
        if (!(s->bounceCount > bounces)) {
            // the bounces are used up: it lands for good
            Float3 zero = { 0.0f, 0.0f, 0.0f };
            GoodieBodyCall::SetFacing(body, &zero, 0.0f);
            direction.x = direction.y = direction.z = 0.0f;
            GoodieBodyCall::SetContact(body, &newContact);
            thrown = 0;
            bounces = 0;
            float delay = place->GetSurfaces()->RebudGoodies(&newContact);
            if (delay != 0.0f) {
                rebudTimer = 0.0f;
                rebudDelay = delay;
            }
        } else {
            // bounce: the direction is reflected in the face's normal, the speed damped
            float newSpeed = GoodieBodyCall::Speed(body) * s->bounceDamping;
            Vector3f d = ToVector(*GoodieBodyCall::Facing(body));
            NavMesh *mesh = (NavMesh *)WorldSurface(GetWorld(), newContact.face);
            Vector3f n = mesh->faces[newContact.second].normal;
            Vector3f reflected = d - n * ((d.y * n.y + d.z * n.z + d.x * n.x) * 2.0f);
            Normalize(reflected);
            Float3 reflectedF = ToFloat3(reflected);
            GoodieBodyCall::SetFacing(body, &reflectedF, newSpeed);
            direction = reflected;
            flightTime = 0.0f;
        }
        float placed[16];
        SetRows(placed, ToVector(newRight), ToVector(newUp), ToVector(newForward), ToVector(hitPos));
        GoodieBodyCall::SetMatrix(body, placed);
        PlayEffect("bounceEffect", ToFloat3(Row(GoodieBodyCall::Matrix(body), 3)));
        return;
    }

    // nothing in the way: on along the arc, unless it has fallen out of the world
    Vector3f newPos = Row(GoodieBodyCall::Matrix(body), 3) + step;
    if (!(gRebudPlane <= newPos.y)) {
        PlaceLevelForgetCreature(place->GetLevel(), sim->FindIdBySoul(this));
        sim->AddId(sim->FindIdBySoul(this));
        return;
    }
    float moved[16];
    memcpy(moved, GoodieBodyCall::Matrix(body), sizeof moved);
    SetTranslation(moved, newPos);
    GoodieBodyCall::SetMatrix(body, moved);
}
