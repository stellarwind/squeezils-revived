// SqueezilPlayerSoul's input-event slot (SQZ.EXE 0x41eff0, ret 8): this is the event-target part at soul + 0x80.
struct PlaceLevel;
struct Place { PlaceLevel *GetLevel(); };                // 0x439b50
struct Body { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
              virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
              virtual void v12(); virtual void PlaceOnFace(void *contact); };   // slot 13 (+0x34)
struct Soul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14();
    virtual void Freeze();                               // slot 15 (+0x3c)
    virtual void Unfreeze();                             // slot 16 (+0x40)
    virtual Body *GetBody();                             // slot 17 (+0x44)
    char pad[0x04];
    int finished;                                        // 0x08
    void Finish();                                       // 0x41f7f0
};
struct EventTarget { void OnEvent(int what, const char *text); };

void EventTarget::OnEvent(int what, const char *text)
{
    switch (what) {
    case 0x16:
        if (*(int *)((char *)this - 0x78))
            ((Soul *)((char *)this - 0x80))->Finish();
        break;
    case 0x1e: {
        ((Soul *)((char *)this - 0x80))->GetBody()->PlaceOnFace((char *)(*(Place **)((char *)this - 0x10))->GetLevel() + 0x23c);
        break;
    }
    case 0x1f:
        if (*(int *)((char *)this - 0x3c))
            ((Soul *)((char *)this - 0x80))->Unfreeze();
        else
            ((Soul *)((char *)this - 0x80))->Freeze();
        break;
    }
}
