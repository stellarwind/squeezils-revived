// MoodFace ctor (0x405430): zero the frames, set frame/current/lastMood to -1, zero the bitmaps/size/tick.
#include "MoodFace/MoodFace.h"

MoodFace::MoodFace()
{
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            animation[i].frames[j] = 0;
    frame = -1;
    current = -1;
    lastMood = -1;
    canvas = 0;
    background = 0;
    width = 0;
    height = 0;
    lastTick = 0;
}
