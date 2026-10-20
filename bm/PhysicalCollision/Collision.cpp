// 0x447590: resolve a pair of creatures and report which one was hit.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "String/String.h"
struct CollisionMatrix { Vector3f *Position(); }; // 0x41faf0
struct CollisionBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual CollisionMatrix *GetMatrix();
};
struct CollisionSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void Hit(int knock, int spray, int otherId);
    virtual void v10(); virtual int IsAttacking();
};
struct CollisionSimulation {
    CollisionBody *GetBody(int id); // 0x414050
    void *GetSoul(int id); // 0x414080
};
CollisionSimulation *GetCollisionSimulation(); // 0x420690
void *CollisionDynamicCast(void *, int, const void *, const void *, int);
extern const char kCollisionSoulType[], kCollisionPhysicalType[]; // 0x4b93e8, 0x4b9420
void AttackImpulse(CollisionBody *attacker, CollisionBody *defender); // 0x446fb0
void BumpImpulse(CollisionBody *first, CollisionBody *second); // 0x4472e0
void CollisionSound(String name, Vector3f position); // 0x44a250
void PhysicalCollision(int id, int otherId)
{
    int knockFirst, sprayFirst, knockSecond, spraySecond;
    CollisionSimulation *simulation = GetCollisionSimulation();
    CollisionBody *first = simulation->GetBody(id);
    CollisionBody *second = simulation->GetBody(otherId);
    CollisionSoul *firstSoul = (CollisionSoul *)CollisionDynamicCast(simulation->GetSoul(id),
        0, kCollisionSoulType, kCollisionPhysicalType, 0);
    CollisionSoul *secondSoul = (CollisionSoul *)CollisionDynamicCast(simulation->GetSoul(otherId),
        0, kCollisionSoulType, kCollisionPhysicalType, 0);
    sprayFirst = 0; int firstAttack = firstSoul->IsAttacking();
    if (firstAttack != secondSoul->IsAttacking()) {
        CollisionBody *attacker;
        CollisionBody *defender;
        if (firstSoul->IsAttacking()) {
            attacker = first;
            defender = second;
            knockFirst = 0;
            knockSecond = 1; spraySecond = 1;
        } else {
            attacker = second;
            defender = first;
            knockFirst = 1; sprayFirst = 1;
            knockSecond = 0; spraySecond = 0;
        }
        AttackImpulse(attacker, defender);
        CollisionSound(String("attackEffect"), *attacker->GetMatrix()->Position());
    } else {
        knockFirst = 1;
        knockSecond = 1; spraySecond = 0;
        BumpImpulse(first, second);
        CollisionSound(String("bumpEffect"), *first->GetMatrix()->Position());
    }
    firstSoul->Hit(knockFirst, sprayFirst, otherId);
    secondSoul->Hit(knockSecond, spraySecond, id);
}
