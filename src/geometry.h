#ifndef SQZ_GEOMETRY_H
#define SQZ_GEOMETRY_H
#include <windows.h>
static inline RECT sqz_fit(int width, int height, int logical_width, int logical_height)
{
    int fit_width = width, fit_height = MulDiv(width, logical_height, logical_width);
    if (fit_height > height) {
        fit_height = height;
        fit_width = MulDiv(height, logical_width, logical_height);
    }
    RECT result = {(width-fit_width)/2, (height-fit_height)/2, 0, 0};
    result.right = result.left + fit_width;
    result.bottom = result.top + fit_height;
    return result;
}
static inline POINT sqz_to_logical(POINT p, RECT content, int width, int height)
{
    if (content.right > content.left && content.bottom > content.top) {
        p.x = MulDiv(p.x-content.left, width, content.right-content.left);
        p.y = MulDiv(p.y-content.top, height, content.bottom-content.top);
    }
    return p;
}
static inline POINT sqz_to_native(POINT p, RECT content, int width, int height)
{
    p.x = content.left + MulDiv(p.x, content.right-content.left, width);
    p.y = content.top + MulDiv(p.y, content.bottom-content.top, height);
    return p;
}
#endif
