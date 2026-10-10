// AddUpdatingProp (0x42d180) and DestroyPropLater (0x42d1f0): a table object at a fixed address.
// The object address is a masked immediate (the game embeds the table at 0x4c71f8/0x4c72e8), so
// one source matches both.
struct PropTable {
    int Insert(void *const &v, int hint);   // out-of-line, target masked
};

void AddUpdatingProp(void *prop)
{
    ((PropTable *)0x4c71f8)->Insert(prop, -1);
}
