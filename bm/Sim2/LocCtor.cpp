// LocationSimulation constructor (SQZ.EXE 0x412e80): the base vtable, the id list, then the derived vtable and active = 1.
struct IdList { IdList(); ~IdList(); };                  // ctor 0x45eef0, dtor 0x45efa0
struct SimBase { virtual ~SimBase() {} };                // trivial inline: the vtable store (0x49d644) is inlined
struct LocationSimulation : SimBase {
    char pad[8];                                         // 0x04 creatures, 0x08 lastId
    IdList ids;                                          // 0x0c
    char pad2[0x10];
    int active;                                          // 0x20
    LocationSimulation();
    virtual ~LocationSimulation();
};

LocationSimulation::LocationSimulation()
{
    active = 1;
}
