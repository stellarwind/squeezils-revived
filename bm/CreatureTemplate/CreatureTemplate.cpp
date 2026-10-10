// FindBehaviourMode (0x448850) and FindStimulus (0x448890): case-insensitive name -> index loops
// over the STIM_* / BEH_* name tables. The tables are read-only global arrays of string pointers.
#include <string.h>

extern const char *kStimulusName[20];   // 0x49ec10
extern const char *kBehaviourName[8];   // 0x49ec60

int FindBehaviourMode(const char *name)
{
    int i;
    for (i = 0; i < 8; i++)
        if (_stricmp(name, kBehaviourName[i]) == 0)
            break;
    return i == 8 ? -1 : i;
}

int FindStimulus(const char *name)
{
    int i;
    for (i = 0; i < 20; i++)
        if (_stricmp(name, kStimulusName[i]) == 0)
            break;
    return i == 20 ? -1 : i;
}
