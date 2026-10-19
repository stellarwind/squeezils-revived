// Array<Vector3f>::Array(int n, const Vector3f *src) (0x45e6c0): allocates n (default 2) vectors and copies them from src.
// (written out because the container template's '*src++' loop schedules differently)
struct V3 { float x, y, z; V3(); ~V3(); V3 &operator=(const V3 &o) { x = o.x; y = o.y; z = o.z; return *this; } };
struct Arr3 {
    virtual int v0();
    int count, capacity;
    V3 *data;
    Arr3(int n, const V3 *src);
};
Arr3::Arr3(int n, const V3 *src)
{
    count = n;
    capacity = n == 0 ? 2 : n;
    data = new V3[capacity];
    for (int i = 0; i < count; i++) data[i] = src[i];
}
