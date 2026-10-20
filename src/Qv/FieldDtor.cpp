#include "Qv/Field.h"
QvFieldA::~QvFieldA() {}
QvFieldB::~QvFieldB() {}
void Use2() { delete new QvFieldA; delete new QvFieldB; }
