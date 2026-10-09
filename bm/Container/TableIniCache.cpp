// Table<String,IniFileCache*,Array<IniFileCache*>,51> (SQZ.EXE 0x46df50-0x46ebd0): file name -> parsed file.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerTable.h"
#include "String/String.h"

class IniFileCache;
template class Table<String, IniFileCache *, Array<IniFileCache *>, 51>;
