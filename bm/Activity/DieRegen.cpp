// DieActivity (0x401e60) and RegenActivity (0x401f50) constructors, in address order.
#include "Activity/Activity.h"
DieActivity::DieActivity(ABody *b, ASoul *s) : WalkActivity(b, s), done(0) {}
RegenActivity::RegenActivity(ABody *b, ASoul *s) : WalkActivity(b, s) {}
