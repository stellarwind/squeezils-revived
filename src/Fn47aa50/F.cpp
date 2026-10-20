// 0x47aa50: group traversal (ret 4): notifies the global traverser, pushes the state, traverses all children, pops.
struct Action { void Push(); void Pop(); };           // 0x481d20, 0x481d30
struct Node { virtual void v0(); virtual void v1(); virtual void Traverse(Action *a); };
struct Group;
struct Traverser { virtual void v0(); virtual void Begin(Group *g, Action *a); virtual void End(Action *a); };
extern Traverser *g_traverser;                        // 0x4c3c44
extern char g_default[];                              // 0x4c3c68
void Fallback(void *p);                               // 0x47a7b0
struct Group {
    int Count();                                      // 0x481fb0
    Node *Get(int i);                                 // 0x481fa0
    void Traverse(Action *a);
};
void Group::Traverse(Action *a)
{
    if (*(int *)&g_traverser == 0) {
        Fallback(g_default);
        return;
    }
    g_traverser->Begin(this, a);
    a->Push();
    for (int i = 0; i < Count(); i++)
        Get(i)->Traverse(a);
    a->Pop();
    g_traverser->End(a);
}
