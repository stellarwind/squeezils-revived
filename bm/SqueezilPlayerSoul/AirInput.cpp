// Spare declarations preserve VC++ 4.1 register allocation (padscan 16).
void PlayerAirPad1();
void PlayerAirPad2();
void PlayerAirPad3();
void PlayerAirPad4();
void PlayerAirPad5();
void PlayerAirPad6();
void PlayerAirPad7();
void PlayerAirPad8();
void PlayerAirPad9();
void PlayerAirPad10();
void PlayerAirPad11();
void PlayerAirPad12();
void PlayerAirPad13();
void PlayerAirPad14();
void PlayerAirPad15();
void PlayerAirPad16();
// 0x41f3e0: read pitch, turn and dive input for an airborne player.
#include <math.h>
#pragma intrinsic(fabs)
struct PlayerFlight { float values[23]; };
struct PlayerAirControl { float pitch, turn; PlayerFlight flight; };
struct PlayerAirPhysics {
    char pad[0x84];
    PlayerFlight fly;
    char pade0[0x28c - 0xe0];
    float forwardTapFactor, turnTapFactor;
};
struct PlayerAirApplication {
    char pad[0x68];
    int attack;
    float ForwardAxis(); // 0x43bc70
    float TurnAxis(); // 0x43bd20
    float ForwardTap(); // 0x43bc10
    float TurnTap(); // 0x43bc40
};
PlayerAirApplication *GetPlayerAirApplication(); // 0x4206b0
extern const float k_playerAirZero; // 0x49d898, 0.0f
// The original compares the nonnegative pitch magnitude as IEEE-754 bits.
inline int PlayerAirBits(const float &value) { return *(const int *)&value; }
struct PlayerAirSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual int GetState();
    virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29();
    virtual void v30(); virtual void v31(); virtual void v32(); virtual void v33();
    virtual void v34(); virtual void v35(); virtual void v36(); virtual void v37();
    virtual void v38(); virtual void v39(); virtual void v40(); virtual void v41();
    virtual void GetAirMotion(int state, PlayerAirControl *control);
    char pad04[0x44 - 4];
    int frozen;
    PlayerAirPhysics *physics;
    char pad4c[0x84 - 0x4c];
    int attackHeld;
    void ReadAirInput(PlayerAirControl *control, int *next);
};
void PlayerAirSoul::ReadAirInput(PlayerAirControl *control, int *next)
{
    float push = GetPlayerAirApplication()->ForwardAxis();
    float sign = 1;
    if (push < k_playerAirZero)
        sign = -1;
    float pitchSize = push * sign;
    int attack = GetPlayerAirApplication()->attack;
    float turn = GetPlayerAirApplication()->TurnAxis();
    float forwardTap = GetPlayerAirApplication()->ForwardTap();
    float turnTap = GetPlayerAirApplication()->TurnTap();
    *next = 24;
    if (frozen) {
        control->turn = 0;
        control->pitch = 999;
        control->flight = physics->fly;
        return;
    }
    if ((PlayerAirBits(forwardTap) & 0x7fffffff) && PlayerAirBits(pitchSize) < 0x3dcccccd) {
        *(int *)&pitchSize = PlayerAirBits(physics->forwardTapFactor);
        push = forwardTap * pitchSize;
    }
    if ((PlayerAirBits(turnTap) & 0x7fffffff) && fabs(turn) <= (double)0.1f)
        turn = physics->turnTapFactor * turnTap;
    int current = GetState();
    GetAirMotion(current, control);
    if (attack && !attackHeld && current != 18)
        *next = 18;
    if (!attack) {
        attackHeld = 0;
        if (current == 18)
            *next = 17;
    }
    control->pitch = PlayerAirBits(pitchSize) > 0x3dcccccd ? push : 999;
    control->turn = fabs(turn) > (double)0.1f ? turn : 0;
}
