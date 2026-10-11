// QvLib node getTypeId slot: returns the class-wide field table pointer (statics 0x4c8864-0x4c8884); one shape for all classes.
struct QvFieldData;
QvFieldData *g_fieldData;
QvFieldData *GetTypeId() { return g_fieldData; }
