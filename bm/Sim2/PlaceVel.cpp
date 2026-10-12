// PlaceSimulation slot 12 (SQZ.EXE 0x439e90): how fast the surface under a body moves; zero for a body in the air.
struct Float3 { float x, y, z; Float3() : x(0), y(0), z(0) {} Float3(const Float3 &o) : x(o.x), y(o.y), z(o.z) {} ~Float3(); };   // dtor 0x401660
struct Face { Float3 Carried(float *pos); };  // 0x40d370
struct Surfaces { Face *FaceOf(void *contact); };        // 0x412bf0
struct Body {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual float *GetMatrix();                          // slot 3 (+0xc)
    virtual void v4(); virtual void v5(); virtual void v6();
    virtual void *GetContact();                          // slot 7 (+0x1c)
    virtual int IsOnSurface();                           // slot 8 (+0x20)
};
struct PlaceSimulation {
    char pad[0x3c];
    Surfaces *surfaces;                                  // 0x3c
    Surfaces *GetSurfaces();                             // 0x439b60
    Float3 Velocity(Body *body);                        // slot 12 (ret 8)
};

Float3 PlaceSimulation::Velocity(Body *body)
{
    if (!body->IsOnSurface())
        {
            Float3 zero;
            return zero;
        }
    Face *face = GetSurfaces()->FaceOf(body->GetContact());
    return face->Carried(body->GetMatrix() + 12);
}
