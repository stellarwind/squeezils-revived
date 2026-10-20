// FillBackground, SQZ.EXE 0x475380.
#include <windows.h>

void FillBackground(HDC dc, RECT *rect, int color)
{
    HBRUSH brush = CreateSolidBrush(PALETTEINDEX(color));
    FillRect(dc, rect, brush);
    DeleteObject(brush);
}
