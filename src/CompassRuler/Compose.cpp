// SQZ.EXE 0x4150b0: wrap the compass pictures across both edges of the strip.
#include "CompassC.h"
#include <math.h>
void GameLog(const char *format, ...);           // 0x419290
extern const char kCompassCopyFailed[];          // 0x4ba288

int CompassC::Compose(int a, int b, float fraction, float roll)
{
    RECT src, dst, nextSrc, nextDst;
    int width = ((StripBitmap *)sprite[a])->GetWidth();
    int left = (int)((1.0f - fraction) * width);
    int right = width - left;
    float rollSize = (float)fabs(roll);
    src.left = right;
    src.right = left + right;
    src.top = (int)(((StripBitmap *)sprite[a])->GetHeight() * rollSize);
    if (*(unsigned *)&roll > 0x80000000U)
        src.top = ((StripBitmap *)sprite[a])->GetHeight() - src.top;
    src.bottom = ((StripBitmap *)sprite[a])->GetHeight();
    dst.left = 0;
    dst.top = 0;
    dst.right = left;
    dst.bottom = src.bottom - src.top;
    if (left > 0 && !picture[11]->CopyRectFrom((StripBitmap *)sprite[a], src, dst)) {
        GameLog(kCompassCopyFailed);
        return 0;
    }
    if (right <= 0) return 1;

    nextSrc.left = 0;
    nextSrc.right = right;
    nextSrc.top = (int)(((StripBitmap *)sprite[a])->GetHeight() * rollSize);
    if (*(unsigned *)&roll > 0x80000000U)
        nextSrc.top = ((StripBitmap *)sprite[a])->GetHeight() - nextSrc.top;
    nextSrc.bottom = ((StripBitmap *)sprite[a])->GetHeight();
    nextDst.left = left;
    nextDst.top = 0;
    nextDst.right = left + right;
    nextDst.bottom = nextSrc.bottom - nextSrc.top;
    if (!picture[11]->CopyRectFrom((StripBitmap *)sprite[b], nextSrc, nextDst)) {
        GameLog(kCompassCopyFailed);
        return 0;
    }
    if ((*(unsigned *)&roll & 0x7fffffffU) == 0) return 1;

    int top = src.top;
    src.left = right;
    src.top = 0;
    src.right = left + right;
    src.bottom = top;
    dst.left = 0;
    dst.top = dst.bottom;
    dst.right = left;
    dst.bottom = ((StripBitmap *)sprite[a])->GetHeight();
    if (left > 0 && !picture[11]->CopyRectFrom((StripBitmap *)sprite[a], src, dst)) {
        GameLog(kCompassCopyFailed);
        return 0;
    }
    if (right <= 0) return 1;

    top = nextSrc.top;
    src.left = 0;
    src.top = 0;
    src.right = right;
    src.bottom = top;
    dst.left = left;
    dst.top = nextDst.bottom;
    dst.right = left + right;
    dst.bottom = ((StripBitmap *)sprite[b])->GetHeight();
    if (!picture[11]->CopyRectFrom((StripBitmap *)sprite[b], src, dst)) {
        GameLog(kCompassCopyFailed);
        return 0;
    }
    return 1;
}
