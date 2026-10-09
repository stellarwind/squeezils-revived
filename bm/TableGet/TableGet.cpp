// Table::Get.
#include "TableGet/TableGet.h"

void *TableGet::Get(int id)
{
    return data[keys.IndexOf(id)];
}
