// QvLib identifier character classes (0x4825d0 name char, 0x482580 / 0x482620 name start).
#include <ctype.h>
int QvIsNameChar(char c);
int QvIsNameChar(char c) { return isalnum(c) || c == '_'; }
int QvIsNameStart(char c)
{
    if (isdigit(c))
        return 0;
    return QvIsNameChar(c);
}
