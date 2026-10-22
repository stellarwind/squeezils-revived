#include "geometry.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    RECT content = sqz_fit(1280, 720, 640, 480);
    assert(content.left == 160 && content.right == 1120 && content.top == 0 && content.bottom == 720);
    POINT center = sqz_to_logical((POINT){640,360}, content, 640,480);
    assert(center.x == 320 && center.y == 240);
    POINT os = sqz_to_native(center, content, 640,480);
    assert(os.x == 640 && os.y == 360);
    content = sqz_fit(900,700,640,480);
    assert(content.left == 0 && content.top == 12 && content.right == 900 && content.bottom == 687);
    center = sqz_to_logical((POINT){450,350}, content,640,480);
    assert(center.x == 320 && center.y == 240);
    os = sqz_to_native((POINT){0,0}, content,640,480);
    assert(os.x == 0 && os.y == 12);
    content = sqz_fit(0,0,640,480);
    center = sqz_to_logical((POINT){0,0}, content,640,480);
    assert(center.x == 0 && center.y == 0);
    puts("PASS: aspect fit, letterbox origin, pointer/warp round trip, minimized client");
}
