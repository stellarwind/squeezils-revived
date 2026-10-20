// 0x47ac90: dispatch an unknown VRML node to the RLDecal callback.
#include <stdio.h>
#include <string.h>

struct Node;
struct Callback {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual void Notify(Node *node, void *arg); // +0x0c
    int enabled;                              // +0x04
};
extern Callback *gCallback;                    // 0x4c3c44
void WrlLog(const char *format, ...);          // 0x47a7b0
static const char kName[] = "RLDecal";          // 0x4c3db8
static const char kFmt[] = "WARNING: %s is unrecognized\n"; // 0x4c3d98
static const char kMissing[] = "No traversal object defined!\nSkipping UnknownNode.\n"; // 0x4c3dc0

struct Node {
    char pad[0x18];
    const char *name;                          // +0x18
    void Handle(void *arg);
};

void Node::Handle(void *arg)
{
    if (gCallback) {
        if (gCallback->enabled) {
            if (strcmp(name, kName) == 0)
                gCallback->Notify(this, arg);
            else
                fprintf(stderr, kFmt, name);
        }
    } else
        WrlLog(kMissing);
}
