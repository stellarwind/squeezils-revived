// LandSimulation slot 1 (SQZ.EXE 0x41ea60): allocate the 0x18-byte part and return its embedded interface (+4).
struct LandPart {
    int pad[6];
    LandPart();                                          // 0x41e310
};
struct LandSimulation {
    void *Make();                                        // slot 1
};

void *LandSimulation::Make()
{
    LandPart *p = new LandPart;
    return p ? (char *)p + 4 : 0;
}
