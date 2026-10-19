// @00446d40 Cleanup of the first vector in the shared PhysicalSettings block.
// The original compiler emits this helper after the initializer's EH handler.
struct PhysicsVector {
    float x, y, z;
    ~PhysicsVector();                                   // 0x401660
};
struct PhysicalSettings {
    PhysicsVector gravity, down;
    float gravityStrength, kineticFriction, staticFriction;
};
extern PhysicalSettings gPhysicalSettings;              // 0x4c7768

void DestroyPhysicsGravity()
{
    gPhysicalSettings.gravity.~PhysicsVector();
}
