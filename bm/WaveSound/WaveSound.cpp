// WaveSound ctor/dtor.
#include "WaveSound/WaveSound.h"

WaveSound::WaveSound()
{
    id = -1;
    format = 0;
    data = 0;
    file = 0;
    buffer = 0;
    useCount = 0;
    unknown10 = 0;
    unknown14 = 0;
}

WaveSound::~WaveSound()
{
}
