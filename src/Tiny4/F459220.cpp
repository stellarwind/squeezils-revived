// 0x459220 (ret 4): clamp a short (x,y) pair into [0,w]x[0,h] and, if it changed, hand it to the setter.
struct ShortPt { short x, y; };
struct Clamp459220 {
    char pad[0x10];
    int w, h;           // 0x10 0x14
    void Set(int x, int y);     // 0x459150
    void Clamp(const ShortPt *p);
};
void Clamp459220::Clamp(const ShortPt *p)
{
    short sx = p->x;
    short sy = p->y;
    struct { int x, y; } q;
    q.x = sx;
    q.y = sy;
    if (sx < 0)
        q.x = 0;
    if (w < sx)
        q.x = (short)w;
    if (sy < 0)
        q.y = 0;
    if (sy > h)
        q.y = (short)h;
    if (sx != q.x || sy != q.y)
        Set(q.x, q.y);
}
