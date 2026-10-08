// NavSurface, part three: moving over the surfaces. Reconstructed from SQZ.EXE 0x4108e0 (a flight) and 0x411120 (a walk), with
// the little helpers 0x412830, 0x4128a0 and 0x4110b0. The geometry of one surface (a nav mesh) is the original's: NavMeshTrace
// (0x47b180) and NavMeshMove (0x47b490) in Original.h. What is done here is working out, across all the surface copies of the
// place (some of which move), which copy is hit or walked onto.
//
// A surface copy has its own matrix. Every query is done in the space of the copy it is about: the squeezil's matrix and the
// direction are turned into the copy's space (MatrixToLocal), asked of the nav mesh there, and the answer turned back (the contact
// keeps the matrix in the copy's space; MatrixToWorld gives the world one).
//
// The original works on matrices of 16 floats whose fourth column is 0 0 0 1; MakeMatrix builds one from its four rows.
#include "NavSurface.h"
#include "NavMesh.h"
#include "SqueezilSoulInternal.h"
#include "FrameProp.h"
#include "Simulation.h"
#include "Script.h"
#include <string.h>

// @00412830 (also @00409f00, the same code) right, up, forward and the position as the four rows of a matrix
float *MakeMatrix(float *m, const float *right, const float *up, const float *forward, const float *pos)
{
    m[0] = right[0];   m[1] = right[1];   m[2] = right[2];   m[3] = 0.0f;
    m[4] = up[0];      m[5] = up[1];      m[6] = up[2];      m[7] = 0.0f;
    m[8] = forward[0]; m[9] = forward[1]; m[10] = forward[2]; m[11] = 0.0f;
    m[12] = pos[0];    m[13] = pos[1];    m[14] = pos[2];    m[15] = 1.0f;
    return m;
}

static Vector3f Vec(const float *p) { return Vector3f(p[0], p[1], p[2]); }
static void Put(float *to, const Vector3f &v) { to[0] = v.x; to[1] = v.y; to[2] = v.z; }

// ---- the helpers on a surface copy ----

// @004128a0
float *InstancedSurface::ToWorldBefore(float *out, const float *point) { return PointToWorld(out, point, previous, previousIdentity, NULL); }
// @004110b0
float *InstancedSurface::MatrixToLocal(float *out, const float *m) { return ::MatrixToLocal(out, m, matrix, identity); }

// What Original.h calls the face object is a surface copy
float *FaceRelativeMatrix(void *face, float *out, const float *matrix) { return ((InstancedSurface *)face)->MatrixToLocal(out, matrix); }

// ---- a flight ----

// @004108e0 The straight flight from pos along direction for `distance`. Every surface copy that is near the start or the end of the
// flight (now, or, if it moved, a frame ago) is asked in its own space, and the hit nearest to where the squeezil stands (by the
// distance in the copy's space) wins. Copies queued for removal are left out.
// For a copy that moved, the flight is first done the way the squeezil, standing still in the world, would have moved relative to
// the copy: from where the copy had it a frame ago to where it is now; only if that hits nothing is the real flight done.
int NavSurfaceList::FlightHitsSurface(const Float3 *pos, const Float3 *right, const Float3 *up, const Float3 *forward,
                                      const Float3 *direction, float distance, Float3 *hitPos, Float3 *newRight, Float3 *newUp,
                                      Float3 *newForward, Contact *newContact)
{
    float m[16];
    MakeMatrix(m, &right->x, &up->x, &forward->x, &pos->x);
    Vector3f end = Vec(&pos->x) + Vec(&direction->x) * distance;

    float bestDistance = 100000.0f;
    int bestInstance = -1;
    int bestFace = 0;
    float best[16];
    InstancedSurface *bestSurface = NULL;

    for (int i = 0; i < instances.count; i++) {
        InstancedSurface *inst = instances.At(i);
        if (!inst)
            continue;
        if (removed.Contains(i) || removedLater.Contains(i))
            continue;
        if (!(inst->IsNear(&pos->x) || inst->IsNear(&end.x) || inst->WasNear(&pos->x) || inst->WasNear(&end.x)))
            continue;

        float local[16];
        inst->MatrixToLocal(local, m);
        int hit = 0;
        float hitAt[3], hitRight[3], hitUp[3], hitForward[3];
        int face = 0;
        if (inst->Changed()) {
            float before[3];
            inst->ToLocalBefore(before, &pos->x);
            Vector3f now = Vec(&local[12]);
            float moved[16];
            memcpy(moved, local, sizeof moved);
            moved[12] = before[0];
            moved[13] = before[1];
            moved[14] = before[2];
            Vector3f delta = now - Vec(before);
            float length = Normalize(delta);
            hit = NavMeshTrace(inst->surface->mesh, &moved[12], &moved[0], &moved[4], &moved[8], &delta.x, length, hitAt, hitRight,
                               hitUp, hitForward, &face, inst->flags.data);
        }
        if (!hit) {
            float localEnd[3];
            inst->ToLocal(localEnd, &end.x);
            Vector3f d = Vec(localEnd) - Vec(&local[12]);
            float length = Length(d);
            Normalize(d);
            hit = NavMeshTrace(inst->surface->mesh, &local[12], &local[0], &local[4], &local[8], &d.x, length, hitAt, hitRight, hitUp,
                               hitForward, &face, inst->flags.data);
            if (!hit)
                continue;
        }

        Vector3f apart = Vec(&local[12]) - Vec(hitAt);
        float squared = (apart.y * apart.y + apart.z * apart.z) + apart.x * apart.x;
        if (squared < bestDistance || bestInstance < 0) {
            MakeMatrix(best, hitRight, hitUp, hitForward, hitAt);
            bestInstance = i;
            bestFace = face;
            bestSurface = inst;
            bestDistance = squared;
        }
    }
    if (bestInstance < 0)
        return 0;

    float world[16];
    bestSurface->MatrixToWorld(world, best);
    hitPos->x = world[12];  hitPos->y = world[13];  hitPos->z = world[14];
    newRight->x = world[0]; newRight->y = world[1]; newRight->z = world[2];
    newUp->x = world[4];    newUp->y = world[5];    newUp->z = world[6];
    newForward->x = world[8]; newForward->y = world[9]; newForward->z = world[10];
    newContact->face = bestInstance;
    newContact->second = bestFace;
    memcpy(newContact->matrix, best, sizeof best);
    return 1;
}

// ---- a walk ----

// @00411120 One step of `distance` along the surface the squeezil stands on (the contact: surface copy, face, and the matrix in
// the copy's space). First the nav mesh slides it along that copy (NavMeshMove). Then every other copy that is near is asked
// whether the squeezil, as moved, runs into it or runs off onto it; the first that says yes decides:
//   - a "flyThrough" type is no obstacle: its Activate script runs and the squeezil stays on its own copy;
//   - otherwise it ends up on that copy (and, if there was distance left over, goes on along it).
// Outputs: the new place and way (world), the new contact. Returns the nav mesh's answer (nonzero: it moved), 0 for nowhere.
// The locals keep the names of what they hold; `slid` is the answer of the first NavMeshMove, `found` the one of the copy that
// is being looked at. The original's choices between its several ways out are kept as they are, including that one search
// normalises the wrong vector (see `quirk`) and that two floats are told apart by the bits of their sign.
int NavSurfaceList::MoveAlongSurface(void *soul, const Float3 *pos, const Float3 *right, const Float3 *up, const Float3 *forward,
                                     const Float3 *direction, float distance, const Contact *contact, Float3 *newPos,
                                     Float3 *newRight, Float3 *newUp, Float3 *newForward, Float3 *newDirection, Contact *newContact)
{
    InstancedSurface *cur = Instance(contact->face);
    int curFace = contact->face;
    float m[16];
    MakeMatrix(m, &right->x, &up->x, &forward->x, &pos->x);
    Vector3f end = Vec(&pos->x) + Vec(&direction->x) * distance;
    float localEnd[3];
    cur->ToLocal(localEnd, &end.x);
    float c[16];
    memcpy(c, contact->matrix, sizeof c);
    Vector3f d = Vec(localEnd) - Vec(&c[12]);
    Normalize(d);

    float outPos[3], outRight[3], outUp[3], outForward[3], outDirection[3];
    int outFace;
    int slid = NavMeshMove(cur->surface->mesh, &c[12], &c[0], &c[4], &c[8], &d.x, distance, contact->second, outPos, outRight, outUp,
                           outForward, outDirection, &outFace, cur->flags.data);

    // what the slide along its own copy gave: the matrix in the copy's space, and in the world
    float stayLocal[16];
    MakeMatrix(stayLocal, outRight, outUp, outForward, outPos);
    float stayWorld[16];
    cur->MatrixToWorld(stayWorld, stayLocal);
    Vector3f reached = Vec(outPos) + Vec(outDirection) * distance;
    float stayDirection[3] = { outDirection[0], outDirection[1], outDirection[2] };
    float reachedWorld[3];
    cur->ToWorld(reachedWorld, &reached.x);
    int stayFace = outFace;

    int found = 0;
    int j = 0;

    // The squeezil goes back to what the slide along its own copy gave
    #define RESTORE_STAY() do { \
        Put(outPos, Vec(&stayLocal[12])); Put(outRight, Vec(&stayLocal[0])); Put(outUp, Vec(&stayLocal[4])); \
        Put(outForward, Vec(&stayLocal[8])); Put(outDirection, Vec(stayDirection)); \
        outFace = stayFace; curFace = contact->face; cur = Instance(contact->face); found = 0; } while (0)
    // Runs the Activate script of the type of (copy j, face oface) if it is a flyThrough type; true when it is one
    #define FLY_THROUGH(oface) ([&]() -> bool { \
        Contact info; info.face = j; info.second = (oface); \
        if (!FlyThrough(&info)) return false; \
        String script; ActivateScript(&script, &info); \
        if (script.c_str()[0]) { \
            ScriptCaller caller; \
            ScriptCallerInitSoul(&caller, soul, ((LocationSimulation *)GetCurrentSimulation())->FindIdBySoul(soul), NULL); \
            PostFunction(script.c_str(), &caller, 0, 0); } \
        return true; }())

    for (j = 0; j < instances.count; j++) {
        if (contact->face == j)
            continue;
        InstancedSurface *o = instances.At(j);
        if (!o)
            continue;
        if (!(o->IsNear(&pos->x) || o->IsNear(&end.x) || (o->Changed() && (o->WasNear(&pos->x) || o->WasNear(&end.x)))))
            continue;

        float O[16];                         // the squeezil in the space of the copy o
        o->MatrixToLocal(O, m);
        int oface = 0;
        float lookAt[3];

        // 1. If either copy moved: where the squeezil's start would be, had it stood still in the world while the copy o moved
        if (cur->Changed() || o->Changed()) {
            float startWorld[3], startLocal[3];
            cur->ToWorldBefore(startWorld, &c[12]);
            o->ToLocalBefore(startLocal, startWorld);
            Vector3f way = Vec(&O[12]) - Vec(startLocal);
            float length = Normalize(way);
            found = NavMeshTrace(o->surface->mesh, startLocal, &O[0], &O[4], &O[8], &way.x, length, outPos, outRight, outUp,
                                 outForward, &oface, o->flags.data);
        }
        if (found != 0) {
            if (FLY_THROUGH(oface)) {
                RESTORE_STAY();
            } else {
                cur = o;
                outFace = oface;
            }
            break;
        }

        // 2. Where the slide along the own copy ended (stayWorld) in the space of o: is it a way onto o?
        Put(lookAt, Vec(o->ToLocal(lookAt, &stayWorld[12])));
        Vector3f way = Vec(lookAt) - Vec(&O[12]);
        float length = Normalize(way);
        found = NavMeshTrace(o->surface->mesh, &O[12], &O[0], &O[4], &O[8], &way.x, length, outPos, outRight, outUp, outForward,
                             &oface, o->flags.data);
        if (found != 0) {
            if (FLY_THROUGH(oface)) {
                RESTORE_STAY();
                break;
            }
            // Onto o. Is there distance left? Then go on along o.
            cur = o;
            outFace = oface;
            float endLocal[3];
            o->ToLocal(endLocal, &end.x);
            Vector3f rest = Vec(endLocal) - Vec(&O[12]);
            d = rest;
            Normalize(d);
            float used = Length(Vec(&O[12]) - Vec(outPos));
            if (used < distance) {
                int goes = NavMeshMove(o->surface->mesh, outPos, outRight, outUp, outForward, &d.x, distance - used, outFace, outPos,
                                       outRight, outUp, outForward, outDirection, &outFace, o->flags.data);
                if (goes) {
                    if (slid == 0) {
                        slid = 1;
                        found = 0;
                        curFace = j;
                    } else {
                        // Both the own copy and o can be walked: o wins if the way the own copy took leaves o's top (its up
                        // vector) behind and the way o took leaves the own copy's: the two floats are compared as signed ints.
                        float onN[16];
                        MakeMatrix(onN, outRight, outUp, outForward, outPos);
                        float onNWorld[16];
                        o->MatrixToWorld(onNWorld, onN);
                        Vector3f apart = Vec(&onNWorld[12]) - Vec(&stayWorld[12]);
                        float dotA = (stayWorld[5] * apart.y + stayWorld[6] * apart.z) + stayWorld[4] * apart.x;
                        Vector3f back = apart * -1.0f;
                        float dotB = (back.y * onNWorld[5] + back.z * onNWorld[6]) + back.x * onNWorld[4];
                        int bitsA, bitsB;
                        memcpy(&bitsA, &dotA, 4);
                        memcpy(&bitsB, &dotB, 4);
                        if (bitsA <= 0 && bitsB > 0) {
                            RESTORE_STAY();
                        } else {
                            slid = 1;
                            curFace = j;
                            found = 0;
                        }
                    }
                } else if (slid == 0) {
                    found = 0;
                } else {
                    RESTORE_STAY();
                }
                break;
            }
            // no distance left: it ends on o where the first look put it (found stays nonzero)
            float stepLocal[3];
            o->ToLocal(stepLocal, &end.x);
            Vector3f toEnd = Vec(stepLocal) - Vec(&O[12]);
            Put(outDirection, toEnd);
            Vector3f unit = Vec(outDirection);
            Normalize(unit);
            Put(outDirection, unit);
            break;
        }

        // 3. The own copy could not be slid along: look at the point it would have reached, a little above o's surface
        if (slid == 0) {
            float t[16];
            o->MatrixToLocal(t, stayWorld);
            memcpy(O, t, sizeof O);
            float where[3];
            o->ToLocal(where, reachedWorld);
            Vector3f lift = Vec(&O[4]) * 0.05f;
            Vector3f raised = Vec(&O[12]) + lift;
            Put(&O[12], raised);
            Vector3f lift2 = Vec(&O[4]) * 0.05f;
            Vector3f lowered = Vec(where) - lift2;
            Vector3f toPoint = lowered - Vec(&O[12]);
            // The original normalises d here (it is already a unit vector) and takes the length from that, not toPoint's
            float quirk = Normalize(d);
            found = NavMeshTrace(o->surface->mesh, &O[12], &O[0], &O[4], &O[8], &toPoint.x, quirk, outPos, outRight, outUp,
                                 outForward, &oface, o->flags.data);
            if (found != 0) {
                cur = o;
                outFace = oface;
                break;
            }
        }
    }
    #undef RESTORE_STAY
    #undef FLY_THROUGH

    if (slid == 0 && found == 0)
        return 0;

    float result[16];
    MakeMatrix(result, outRight, outUp, outForward, outPos);
    float resultWorld[16];
    cur->MatrixToWorld(resultWorld, result);
    newPos->x = resultWorld[12];   newPos->y = resultWorld[13];   newPos->z = resultWorld[14];
    newRight->x = resultWorld[0];  newRight->y = resultWorld[1];  newRight->z = resultWorld[2];
    newUp->x = resultWorld[4];     newUp->y = resultWorld[5];     newUp->z = resultWorld[6];
    newForward->x = resultWorld[8]; newForward->y = resultWorld[9]; newForward->z = resultWorld[10];
    if (found != 0) {
        // onto another copy: the way is the one it had
        Float3 keep = *direction;
        *newDirection = keep;
        newContact->face = j;
        newContact->second = outFace;
        memcpy(newContact->matrix, result, sizeof result);
        return found;
    }
    Vector3f ahead = Vec(outPos) + Vec(outDirection);
    float aheadWorld[3];
    cur->ToWorld(aheadWorld, &ahead.x);
    Vector3f way = Vec(aheadWorld) - Vec(&newPos->x);
    Normalize(way);
    newDirection->x = way.x;
    newDirection->y = way.y;
    newDirection->z = way.z;
    newContact->face = curFace;
    newContact->second = outFace;
    memcpy(newContact->matrix, result, sizeof result);
    return slid;
}

// The calls the rest of the reconstruction makes (Original.h)
int MoveAlongSurface(void *surfaces, void *soul, Float3 *pos, Float3 *right, Float3 *up, Float3 *forward, Float3 *direction,
                     float distance, Contact *contact, Float3 *newPos, Float3 *newRight, Float3 *newUp, Float3 *newForward,
                     Float3 *newDirection, Contact *newContact)
{
    return ((NavSurfaceList *)surfaces)->MoveAlongSurface(soul, pos, right, up, forward, direction, distance, contact, newPos,
                                                           newRight, newUp, newForward, newDirection, newContact);
}
int FlightHitsSurface(void *surfaces, Float3 *pos, Float3 *right, Float3 *up, Float3 *forward, Float3 *direction, float distance,
                      Float3 *hitPos, Float3 *newRight, Float3 *newUp, Float3 *newForward, Contact *newContact)
{
    return ((NavSurfaceList *)surfaces)->FlightHitsSurface(pos, right, up, forward, direction, distance, hitPos, newRight, newUp,
                                                           newForward, newContact);
}
void NavSurfaceTypeChanged(void *surfaces, void *soul, const Contact *contact, const void *oldInfo, const void *newInfo)
{
    ((NavSurfaceList *)surfaces)->TypeChanged(soul, contact, (const SurfaceInfo *)oldInfo, (const SurfaceInfo *)newInfo);
}
