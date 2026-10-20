// SqueezilPlayerSoul::Hit (slot 9, SQZ.EXE 0x41fab0): stop the looping sound, then the base class's Hit.
extern int g_loopingSound;                               // 0x4bd960
void StopLoopingSounds();                                // 0x44a8f0 (cdecl)
struct SqueezilSoul { void Hit(int knock, int sprayNuts, int unused); };   // 0x4506d0 (ret 0xc)
struct SqueezilPlayerSoul : SqueezilSoul { void Hit(int knock, int sprayNuts, int unused); };

void SqueezilPlayerSoul::Hit(int knock, int sprayNuts, int unused)
{
    if (g_loopingSound)
        StopLoopingSounds();
    SqueezilSoul::Hit(knock, sprayNuts, unused);
}
