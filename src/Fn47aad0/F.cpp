// 0x47aad0: gather the Coordinate/Normal/Material/Texture nodes of an IndexedFaceSet and hand them to the traversal object.
struct Val { char pad[0x24]; int mode; };               // 0x24
struct Fld { char pad[0xc]; Val *value; };              // 0xc
struct Node {
    char pad[4];
    Fld *coords;            // 0x04
    char pad1[0x10 - 8];
    Fld *materialBinding;   // 0x10
    Fld *materialIndex;     // 0x14
    Fld *normalBinding;     // 0x18
    Fld *normalIndex;       // 0x1c
    Fld *texCoordIndex;     // 0x20
    Fld *coordIndex;        // 0x24
    Fld *a28;               // 0x28
    Fld *extra;             // 0x2c
};
struct Item { char pad[4]; Node *node; };
struct Args {
    void *self;
    Val *coords;
    Val *normalIndex;
    Val *normalBinding;
    Val *materialIndex;
    Val *materialBinding;
    Val *coordIndex;
    Val *extra;
    Val *texIndex;
    int flag;
};
struct Trav {
    virtual void Run(Args a);
    int enabled;            // 0x04
    char pad[0x208 - 8];
    char where[1];          // 0x208
};
extern Trav *gTrav;                                     // 0x4c3c44
void Log(const char *format, ...);                      // 0x47a7b0
struct Walker { void Visit(Item *item); };
void Walker::Visit(Item *item)
{
    Args a;
    Node *n;
    if (gTrav == 0) {
        Log("No traversal object defined. Skipping..");
        return;
    }
    if (gTrav->enabled == 0)
        return;
    a.self = this;
    a.texIndex = 0;
    a.normalBinding = 0;
    a.materialBinding = 0;
    a.coords = 0;
    a.normalIndex = 0;
    n = item->node;
    a.extra = 0;
    a.materialIndex = 0;
    a.coordIndex = 0;
    if (n->coords)
        a.coords = n->coords->value;
    if (a.coords == 0) {
        Log("Can't find Coordinate node for IndexedFaceSet", gTrav->where);
        gTrav->enabled = 0;
        return;
    }
    if (n->normalIndex)
        a.normalIndex = n->normalIndex->value;
    if (n->normalBinding)
        a.normalBinding = n->normalBinding->value;
    if (a.normalBinding && a.normalBinding->mode != 7) {
        Log("NormalBinding value is not PER_VERTEX_INDEXED", gTrav->where);
        gTrav->enabled = 0;
        return;
    }
    if (n->materialIndex)
        a.materialIndex = n->materialIndex->value;
    if (n->materialBinding)
        a.materialBinding = n->materialBinding->value;
    if (a.materialBinding && a.materialBinding->mode != 0 && a.materialBinding->mode != 1 && a.materialBinding->mode != 5) {
        Log("MaterialBinding value isn't DEFAULT, OVERALL or PER_PART", gTrav->where);
        gTrav->enabled = 0;
        return;
    }
    if (n->coordIndex)
        a.coordIndex = n->coordIndex->value;
    if (n->extra)
        a.extra = n->extra->value;
    if (n->texCoordIndex)
        a.texIndex = n->texCoordIndex->value;
    a.flag = 1;
    if (a.texIndex && a.texIndex->mode == 1)
        a.flag = 0;
    gTrav->Run(a);
}
