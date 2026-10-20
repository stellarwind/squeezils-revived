// Table<String,IniFileSection*,Array<IniFileSection*>,50> (SQZ.EXE 0x46d2c0-0x46df50): section name -> section.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerTable.h"
#include "String/String.h"

class IniFileSection;
template class Table<String, IniFileSection *, Array<IniFileSection *>, 50>;
