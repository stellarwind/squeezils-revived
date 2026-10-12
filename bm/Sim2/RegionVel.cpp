// RegionSimulation slot 12 (SQZ.EXE 0x4189b0): a region has no moving ground; the velocity is always zero.
struct Float3 { float x, y, z; Float3() : x(0), y(0), z(0) {} Float3(const Float3 &o) : x(o.x), y(o.y), z(o.z) {} ~Float3(); };   // dtor 0x401660
struct Body;
struct RegionSimulation {
    Float3 Velocity(Body *body);                         // slot 12 (ret 8)
};

Float3 RegionSimulation::Velocity(Body *body)
{
    Float3 zero;
    return zero;
}
