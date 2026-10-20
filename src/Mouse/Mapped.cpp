// SQZ.EXE 0x43b120: mapped axis and charged-action input.
#include "AppInput.h"
#include <windows.h>
#include <mmsystem.h>
void SetPaused(int on);                            // 0x43ed80
extern const char gText[];                         // 0x4b9488

int App::Mapped(int code, int pressed)
{
    int ok = 1;
    switch (code) {
    case 0: case 0xc: {
        if (pressed) valueA = 1.0f;
        if (heldA && pressed) return 0;
        AppInputAxis *a = &axisA;
        heldA = pressed;
        a->direction = 1;
        if (pressed) a->down = timeGetTime();
        else a->up = timeGetTime();
        break;
    }
    case 1: case 0xd: {
        if (pressed) valueA = -1.0f;
        if (heldA && pressed) return 0;
        AppInputAxis *a = &axisA;
        heldA = pressed;
        a->direction = -1;
        if (pressed) a->down = timeGetTime();
        else a->up = timeGetTime();
        break;
    }
    case 2:
        if (pressed) FirstDown();
        else FirstUp();
        break;
    case 3: {
        if (firstDown && pressed) return 0;
        if (!sink) return 0;
        if (pressed) {
            firstDown = timeGetTime();
            sink->BeginFirst();
            firstHeld = 1;
        } else {
            firstUp = timeGetTime();
            float strength = (float)(firstUp - firstDown) * 0.001f;
            if (strength <= firstDuration) strength = strength / firstDuration;
            else strength = 1.0f;
            sink->EndFirst(strength);
            firstHeld = 0;
            firstDown = 0;
            firstUp = 0;
        }
        break;
    }
    case 4: {
        if (secondDown && pressed) return 0;
        if (!sink) return 0;
        if (pressed) {
            secondDown = timeGetTime();
            sink->BeginSecond();
            secondHeld = 1;
        } else {
            secondUp = timeGetTime();
            float strength = (float)(secondUp - secondDown) * 0.001f;
            if (strength <= secondDuration) strength = strength / secondDuration;
            else strength = 1.0f;
            sink->EndSecond(strength);
            secondHeld = 0;
            secondDown = 0;
            secondUp = 0;
        }
        break;
    }
    case 5: case 10: {
        if (pressed) valueB = -1.0f;
        if (heldB && pressed) return 0;
        AppInputAxis *a = &axisB;
        heldB = pressed;
        a->direction = -1;
        if (pressed) a->down = timeGetTime();
        else a->up = timeGetTime();
        break;
    }
    case 6: case 0xb: {
        if (pressed) valueB = 1.0f;
        if (heldB && pressed) return 0;
        AppInputAxis *a = &axisB;
        heldB = pressed;
        a->direction = 1;
        if (pressed) a->down = timeGetTime();
        else a->up = timeGetTime();
        break;
    }
    case 8:
        if (pressed) SecondDown();
        else SecondUp();
        break;
    case 0x16:
        if (!sink) return 0;
        if (pressed) sink->Command(code, gText);
        break;
    case 0x17:
        if (!pressed) return 0;
        modal = !modal;
        if (modal) SetPaused(1);
        else SetPaused(0);
        break;
    default:
        ok = 0;
        break;
    }
    return ok;
}
