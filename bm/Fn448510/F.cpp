// CreatureTemplate::Load (0x448510, ret 8): reads the stimulus function names, the three sense radii and the body colour from an INI section.
#include "CreatureTemplate/CreatureTemplate.h"
#include "SqueezilBody/BodyColor.h"
#include "IniReader/IniReader.h"

extern const char *kStimulusName[20];   // 0x49ec10

int CreatureTemplate::Load(String &section, IniReader *ini)
{
    String value;
    ini->SetSection(section.c_str());
    for (int i = 0; i < 20; i++) {
        value = String(ini->GetString(kStimulusName[i]));
        names.Replace(i, value);
    }
    senseRadius[2] = ini->GetFloat("SenseRadiusG", 0);
    senseRadius[1] = ini->GetFloat("SenseRadiusR", 0);
    senseRadius[0] = ini->GetFloat("SenseRadiusP", 0);
    bodyColor = ReadBodyColor(ini);
    return 1;
}
