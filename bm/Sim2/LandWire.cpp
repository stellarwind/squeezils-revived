// LandSimulation slot 2 (SQZ.EXE 0x41e8d0): write the two embedded sub-objects (when present) to a stream.
struct Streamable;
struct Stream { Stream &operator<<(Streamable *o); };    // 0x448c60
struct LandSimulation {
    int pad;
    char *first;                                         // 0x04
    char *second;                                        // 0x08
    Stream &Save(Stream &out);                           // slot 2
};

Stream &LandSimulation::Save(Stream &out)
{
    return out << (Streamable *)(first ? first + 0x2c : 0) << (Streamable *)(second ? second + 0x10 : 0);
}
