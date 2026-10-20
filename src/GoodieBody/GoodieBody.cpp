// GoodieBody::HasContact (0x45a9f0): whether the body stands on a surface face.
#include "GoodieBody/GoodieBody.h"

int GoodieBody::HasContact()
{
    return contact.face != -1;
}
