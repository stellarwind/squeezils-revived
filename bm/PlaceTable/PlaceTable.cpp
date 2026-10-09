// PlaceTable::Get (0x469a70).
#include "PlaceTable/PlaceTable.h"

void *PlaceTable::Get(int id)
{
    return data[keys.IndexOf(id)];
}
