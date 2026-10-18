// Navigation hit comparison callback, SQZ.EXE 0x47c630.
struct NavHit {
    int face;
    float t;
};

int CompareHits(const void *first, const void *second)
{
    const NavHit *a = (const NavHit *)first;
    const NavHit *b = (const NavHit *)second;
    if (b->t > a->t)
        return -1;
    if (b->t < a->t)
        return 1;
    return 0;
}
