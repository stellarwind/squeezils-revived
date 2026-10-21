// SQZ.EXE 0x4108e0: NavSurfaceList::FlightHits - cast a flight segment (position + direction * distance) against
// every live surface near either end, in each surface's local frame, and return the nearest hit in world space.
#include "compat41.h"
#include "NavMesh/RaycastBridge.h"
#include "NavSurface/Nearness.h"

struct FlightVector {
    float x, y, z;
    FlightVector() {}
    ~FlightVector();                                            // 0x401660
    FlightVector operator*(float scale) const;                  // 0x401b60
    FlightVector operator+(const FlightVector &v) const;        // 0x403070
    FlightVector operator-(const FlightVector &v) const;        // 0x401ae0
    float Length() const;                                       // 0x47d5f0
    float Normalize();                                          // 0x47d630
};

// SIZE 0x40
struct FlightMatrix {
    float m[16];
    FlightMatrix() {}
    FlightMatrix(const FlightVector &right, const FlightVector &up, const FlightVector &forward,
                 const FlightVector &position) {
        m[0] = right.x; m[1] = right.y; m[2] = right.z; m[3] = 0;
        m[4] = up.x; m[5] = up.y; m[6] = up.z; m[7] = 0;
        m[8] = forward.x; m[9] = forward.y; m[10] = forward.z; m[11] = 0;
        m[12] = position.x; m[13] = position.y; m[14] = position.z; m[15] = 1.0f;
    }
    ~FlightMatrix();                                            // 0x402310
    FlightMatrix &operator=(const FlightMatrix &other);         // 0x402340
    FlightMatrix &SetTranslation(const FlightVector &v);        // 0x47e330
    void Set(const FlightVector &right, const FlightVector &up, const FlightVector &forward,
             const FlightVector &position) {
        m[0] = right.x; m[1] = right.y; m[2] = right.z;
        m[4] = up.x; m[5] = up.y; m[6] = up.z;
        m[8] = forward.x; m[9] = forward.y; m[10] = forward.z;
        m[12] = position.x; m[13] = position.y; m[14] = position.z;
        m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1.0f;
    }
    const FlightVector &Right() const { return *(const FlightVector *)(m + 0); }
    const FlightVector &Up() const { return *(const FlightVector *)(m + 4); }
    const FlightVector &Forward() const { return *(const FlightVector *)(m + 8); }
    const FlightVector &Position() const { return *(const FlightVector *)(m + 12); }
};

// SIZE 0x48
struct FlightContact {
    int face;                                                   // 0x00 surface number, -1 for none
    int second;                                                 // 0x04 face of that surface's mesh
    FlightMatrix matrix;                                        // 0x08
    FlightContact() : face(-1), second(0) {}
    ~FlightContact();                                           // 0x4031e0
};


struct FlightSurfaceData {
    NavMesh *mesh;                                           // 0x00
};

struct FlightSurface {
    FlightSurfaceData *GetSurface();                            // 0x40c6b0
    const int *TypeFilter();                                    // 0x40c6d0
    int WasNear(const FlightVector &point);                     // 0x40c7b0
    int Changed();                                              // 0x40c860
    FlightVector ToLocal(const FlightVector &point);            // 0x40f6f0
    FlightMatrix ToWorld(const FlightMatrix &matrix);           // 0x4105f0
    FlightVector ToLocalBefore(const FlightVector &point);      // 0x410660
    FlightMatrix Relative(const FlightMatrix &matrix);          // 0x4110b0
};

// SIZE 0x10
struct FlightSurfaces {
    virtual FlightSurface *Get(int index);                      // vtable+0x00
    int count;                                                  // 0x04
    int capacity;                                               // 0x08
    FlightSurface **data;                                       // 0x0c
};

// SIZE 0x14
struct FlightIds {
    virtual int Get(int index);                                 // vtable+0x00
    virtual int Insert(const int &id, int index);               // vtable+0x04
    virtual void Remove(int index);                             // vtable+0x08
    virtual void Replace(int index, const int &id);             // vtable+0x0c
    virtual int *Data();                                        // vtable+0x10
    virtual int Grow(int extra);                                // vtable+0x14
    virtual int Contains(const int &id);                        // vtable+0x18
    int count;                                                  // 0x04
    int capacity;                                               // 0x08
    int *data;                                                  // 0x0c
    void *bufferObject;                                         // 0x10
};

struct NavFlight {
    char pad[0x134];
    FlightSurfaces instances;                                   // 0x134
    FlightIds removed;                                          // 0x144
    FlightIds removedLater;                                     // 0x158
    int FlightHits(const FlightVector *position, const FlightVector *right, const FlightVector *up,
        const FlightVector *forward, const FlightVector *direction, float distance, FlightVector *hitPosition,
        FlightVector *newRight, FlightVector *newUp, FlightVector *newForward, FlightContact *contact);
};

int NavFlight::FlightHits(const FlightVector *position, const FlightVector *right, const FlightVector *up,
    const FlightVector *forward, const FlightVector *direction, float distance, FlightVector *hitPosition,
    FlightVector *newRight, FlightVector *newUp, FlightVector *newForward, FlightContact *contact)
{
    FlightMatrix frame(*right, *up, *forward, *position);
    FlightVector end = *position + *direction * distance;
    float bestDistance = 100000.0f;
    FlightContact best;
    FlightSurface *bestSurface;
    best.face = -1;

    for (int i = 0; i < instances.count; i++) {
        FlightSurface *surface = instances.Get(i);
        if (surface == 0 || removed.Contains(i) || removedLater.Contains(i))
            continue;
        if (!IsNavigationSurfaceNear((void *)surface, *position) && !IsNavigationSurfaceNear((void *)surface, end)
            && !surface->WasNear(*position) && !surface->WasNear(end))
            continue;

        FlightMatrix local = surface->Relative(frame);
        int hit = 0;
        FlightVector point, hitRight, hitUp, hitForward;
        int face;

        if (surface->Changed()) {
            // The surface moved this frame: cast from where the body was relative to it before the move.
            FlightVector before = surface->ToLocalBefore(*position);
            FlightVector now = local.Position();
            FlightMatrix start = local;
            start.SetTranslation(before);
            FlightVector move = now - before;
            float length = move.Normalize();
            hit = CastNavigationRay((NavMesh *)surface->GetSurface()->mesh, start.Position(), start.Right(), start.Up(), start.Forward(),
                move, length, &point, &hitRight, &hitUp, &hitForward, &face, surface->TypeFilter());
        }
        if (hit == 0) {
            FlightVector localEnd = surface->ToLocal(end);
            FlightVector move = localEnd - local.Position();
            float length = move.Length();
            move.Normalize();
            hit = CastNavigationRay((NavMesh *)surface->GetSurface()->mesh, local.Position(), local.Right(), local.Up(), local.Forward(),
                move, length, &point, &hitRight, &hitUp, &hitForward, &face, surface->TypeFilter());
        }
        if (hit) {
            FlightVector delta = point - local.Position();
            float squared = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            if (squared < bestDistance || best.face < 0) {
                best.matrix.Set(hitRight, hitUp, hitForward, point);
                best.face = i;
                best.second = face;
                bestSurface = surface;
                bestDistance = squared;
            }
        }
    }

    if (best.face >= 0) {
        FlightMatrix world = bestSurface->ToWorld(best.matrix);
        *hitPosition = world.Position();
        *newRight = world.Right();
        *newUp = world.Up();
        *newForward = world.Forward();
        contact->face = best.face;
        contact->second = best.second;
        contact->matrix = best.matrix;
        return 1;
    }
    return 0;
}
