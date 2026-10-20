// 0x47a990: load and traverse a VRML scene with a temporary node table.
struct QvNodeT;
struct QvNodeTable {
    int count;
    QvNodeT **buckets;
    QvNodeTable();                         // 0x481c90
    ~QvNodeTable();                        // 0x481cc0
};
class QvRc {
public:
    virtual ~QvRc();
    int pad4, pad8, padc;
    void *name;
    int refCount;
    void Ref();                           // 0x47e750
    void Unref();                         // 0x47e760
    void *GetName();
};
struct SceneNode : QvRc {
    virtual void v1();
    virtual void Traverse(QvNodeTable *table); // +0x08
};
struct QvNode;
QvNode *ReadWrlFile(const char *path);      // 0x47a800
void PrepareWrlNames();                    // 0x481c20
extern int gWrlCacheReady;                 // 0x4c3c40
struct SceneTraversal {
    int vptr, enabled;
    char pad[0x308 - 8];
    SceneNode *root;                       // +0x308
};
extern SceneTraversal *gSceneTraversal;    // 0x4c3c44

void Restore(const char *path)
{
    if (gWrlCacheReady) {
        PrepareWrlNames();
        gWrlCacheReady = 0;
    }
    SceneNode *root = (SceneNode *)ReadWrlFile(path);
    if (!root) {
        gSceneTraversal->enabled = 0;
    } else {
        root->Ref();
        gSceneTraversal->root = root;
        QvNodeTable table;
        root->Traverse(&table);
        root->Unref();
    }
}
