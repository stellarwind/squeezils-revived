// Panel trivial setters.
#include "Panel/Panel.h"

void Panel::Invalidate() { dirty = 1; }
void Panel::ClearEvent() { event = 0; }
