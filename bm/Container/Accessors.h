// The non-virtual accessors of the container templates (Array::At 0x40f310, Table::At 0x40f2a0, Table::Get 0x462cc0 ...). They are
// separate from the main definitions because instantiating them in the same unit as the virtual methods changes the code generated
// for those (see BYTEMATCH_GUIDE.md, "Working method"). Include this after Container.h in the unit that instantiates them.
#pragma once

template <class T>
T Array<T>::At(int i)
{
    return data[i];
}
