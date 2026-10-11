// Name / Make of the CreateProp subclasses (SQZ.EXE 0x470bd0-0x471010).
#include "FunctionProp/PropClasses.h"

const char *CreatePropAtPositionFunction::Name() { return "CreatePropAtPosition"; }
CreatePropFunction *CreatePropAtPositionFunction::Make() { return new CreatePropAtPositionFunction; }

const char *CreatePropAtSqueezilPositionFunction::Name() { return "CreatePropAtSqueezilPosition"; }
CreatePropFunction *CreatePropAtSqueezilPositionFunction::Make() { return new CreatePropAtSqueezilPositionFunction; }

const char *CreatePropTrackingSqueezilPositionFunction::Name() { return "CreatePropTrackingSqueezilPosition"; }
CreatePropFunction *CreatePropTrackingSqueezilPositionFunction::Make() { return new CreatePropTrackingSqueezilPositionFunction; }

const char *CreatePropTrackingSqueezilXformFunction::Name() { return "CreatePropTrackingSqueezilXform"; }
CreatePropFunction *CreatePropTrackingSqueezilXformFunction::Make() { return new CreatePropTrackingSqueezilXformFunction; }
