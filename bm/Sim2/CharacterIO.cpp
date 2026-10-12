// Character (BufferObject, 8 bytes, vtable 0x4bd948-ish): Write (0x4489c0, ret 4) and the factory Create (0x448a00).
struct BufferObject { virtual void v0(); };
struct Stream { Stream &Write(BufferObject *o); };       // 0x448c60 (ret 4)
struct Character {
    virtual void v0();
    BufferObject *field4;
    Character();                                         // 0x4488d0
    void Write(Stream &s);                               // 0x4489c0
};
Character *CreateCharacter();                            // 0x448a00

void Character::Write(Stream &s)
{
    s.Write(field4);
}

Character *CreateCharacter()
{
    return new Character;
}
