// PossessionsTable::Get (0x465340).
#include "PossessionsTable/PossessionsTable.h"

void *PossessionsTable::Get(int id)
{
    return data[keys.IndexOf(id)];
}
