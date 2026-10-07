// MoodFace: the Squeezil's animated face in the in-game panel. A set of named animations (Happy, Anger, Sleeping ...), each a
// few bitmaps, chosen from the player's mood and stepped on a timer; each frame is composed over the panel picture it sits on
// and drawn to the primary surface. Reconstructed from SQZ.EXE 0x405430-0x405e2c (a sub object of the panel, at +8).
// The animations come from the [Interface] section: Animations (count), FrameRate (seconds per frame), and per animation
// Animation<n> (frame count), Animation<n>Name (its name) and Animation<n>_<m> (the m-th frame's file under ui\).
#pragma once
#include "Bitmap.h"

class MoodFace {
public:
    enum { MAX_ANIMATIONS = 10, MAX_FRAMES = 10 };

    MoodFace();                                   // @405430
    ~MoodFace();                                  // @405490
    BOOL Load(int x, int y);                      // @405520 read the animations and their pictures; (x, y) is where the face sits in the panel
    int  Draw(int force, Bitmap *panel);          // @4059e0 draw the face if its frame is due (or force); 1 when the player is asleep
    int  FindAnimation(int mood);                 // @405d10 the animation for a mood: the row whose name matches, or 0

    struct Animation {                            // 0x40 bytes
        Bitmap *frames[MAX_FRAMES];               // 0x00
        int     frameCount;                       // 0x28
        char    name[20];                         // 0x2c
    };

    Animation animation[MAX_ANIMATIONS];          // 0x000
    int      unused280;                           // 0x280
    int      x;                                   // 0x284 where the face sits in the panel
    int      y;                                   // 0x288
    char     pad28c[0x14];                        // 0x28c
    Bitmap  *canvas;                              // 0x2a0 the face composed over the panel picture
    Bitmap  *background;                          // 0x2a4 the panel picture under the face, captured once
    int      width;                               // 0x2a8 largest frame
    int      height;                              // 0x2ac
    int      lastMood;                            // 0x2b0 -1 until drawn
    int      animationCount;                      // 0x2b4
    int      frame;                               // 0x2b8 frame of the current animation, -1 before the first
    int      current;                             // 0x2bc current animation, -1 before the first
    unsigned lastTick;                            // 0x2c0 timeGetTime when the frame was drawn
    float    frameMs;                             // 0x2c4 milliseconds per frame
};
static_assert(sizeof(MoodFace::Animation) == 0x40, "MoodFace::Animation layout");
static_assert(sizeof(MoodFace) == 0x2c8, "MoodFace layout");

// @0042da30 the screen position the panel is placed from: left and bottom of the renderer's screen rectangle
void GetScreenOrigin(int *x, int *y);
