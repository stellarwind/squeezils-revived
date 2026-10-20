// A spatial-tree node (SQZ.EXE 0x47ade0-0x47ae90): eight children and a list of item indices.
struct TreeNode {
    int a0, a1, a2, a3, a4, a5;      // 0x00..0x14
    int count;                       // 0x18 items in the list
    int *items;                      // 0x1c
    TreeNode *child[8];              // 0x20
    TreeNode();                      // 0x47ade0
    void Clear();                    // 0x47ae10
    int Allocate(int n);             // 0x47ae50
    void FreeItems();                // 0x47ae90
};

TreeNode::TreeNode()
{
    a1 = 0; a0 = 0; a3 = 0; a2 = 0;
    a5 = 0; a4 = 0; count = 0; items = 0;
    for (int i = 0; i < 8; i++)
        child[i] = 0;
}

void TreeNode::Clear()
{
    FreeItems();
    for (int i = 0; i < 8; i++) {
        TreeNode *c = child[i];
        if (c) {
            c->Clear();
            delete c;
            child[i] = 0;
        }
    }
}

int TreeNode::Allocate(int n)
{
    FreeItems();
    if (n) {
        items = new int[n];
        if (!items)
            return 0;
        count = n;
    }
    return 1;
}

void TreeNode::FreeItems()
{
    if (items)
        delete items;
    count = 0;
    items = 0;
}
