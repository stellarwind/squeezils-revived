// MoodFace::FindAnimation (SQZ.EXE 0x405d10, ret 4): the animation for a mood: the row whose name matches, or 0.
#include <string.h>
struct AnimRow { char name[0x40]; };
struct MoodFaceF {
    char pad[0x2c];
    AnimRow animation[10];          // 0x2c
    char pad2[0x2b4 - 0x2c - 0x280];
    int animationCount;             // 0x2b4
    int FindAnimation(int mood);
};

int MoodFaceF::FindAnimation(int mood)
{
    char want[20];
    switch (mood) {
    case 0: strcpy(want, "Ambivalent"); break;
    case 1: strcpy(want, "Sleeping"); break;
    case 2: strcpy(want, "Alert"); break;
    case 3: strcpy(want, "Alert"); break;
    case 4: strcpy(want, "Happy"); break;
    case 6: strcpy(want, "Surprise"); break;
    case 7: strcpy(want, "Anger"); break;
    case 8: strcpy(want, "Surprise"); break;
    case 9: strcpy(want, "Happy"); break;
    case 10: strcpy(want, "Ambivalent"); break;
    case 11: strcpy(want, "Surprise"); break;
    case 12: strcpy(want, "Jumping"); break;
    case 13: strcpy(want, "Jumping"); break;
    case 14: strcpy(want, "Jumping"); break;
    case 16: strcpy(want, "Spitting"); break;
    case 17: strcpy(want, "Happy"); break;
    case 19: strcpy(want, "Surprise"); break;
    case 20: strcpy(want, "Anger"); break;
    case 21: strcpy(want, "Anger"); break;
    case 22: strcpy(want, "Sleeping"); break;
    default: strcpy(want, "undefined"); break;
    }
    for (int i = 0; i < animationCount; i++)
        if (strcmp(animation[i].name, want) == 0)
            return i;
    return 0;
}
