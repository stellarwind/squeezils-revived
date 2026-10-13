// 15 spare declarations (symbol count, see padscan.sh; 14-17 all give 0)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
// Table<String,IniFileCache*,Array<IniFileCache*>,51> (SQZ.EXE 0x46df50-0x46ebd0): file name -> parsed file.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerTable.h"
#include "String/String.h"

class IniFileCache;
template class Table<String, IniFileCache *, Array<IniFileCache *>, 51>;
