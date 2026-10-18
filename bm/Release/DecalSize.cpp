// SQZ.EXE 0x426c00 (stdcall, ret 0x10): set a frame's decal size from a by-value vector (x = width, y = height) through the world.
struct V3 {
    float x, y, z;
    V3() {}
    V3(const V3 &o) : x(o.x), y(o.y), z(o.z) {}
    ~V3();
};
struct WorldD { void *SetDecalSize(void *frame, float width, float height); };   // 0x435c70
WorldD *GetWorld();                                                               // 0x4206a0

void __stdcall DecalSizeV(void *frame, V3 size)
{
    GetWorld()->SetDecalSize(frame, size.x, size.y);
}
