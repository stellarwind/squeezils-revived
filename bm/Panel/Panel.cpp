// Panel trivial setters.
#include <string.h>
#include "Panel/Panel.h"

void Panel::Invalidate() { dirty = 1; }
void Panel::ClearEvent() { event = 0; }
void Panel::ClearMessage() { strcpy(message, ""); }
void Panel::ClearNote() { strcpy(note, ""); }
