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
extern unsigned char f_4c778c;

void DestroyPhysicsGravity()
{
    gPhysicalSettings.gravity.~PhysicsVector();
}

// 0x446d60: destroy down, then gravity, preserving EH cleanup and the shared guard.
void G446d60()
{
    if (!(f_4c778c & 1)) {
        f_4c778c |= 1;
        gPhysicalSettings.~PhysicalSettings();
    }
}
