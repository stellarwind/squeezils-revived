// Pointer-valued tables own String key storage and pointer-array storage only.
// Their enclosing PoseTable/MorphCycle/PlaceLevel owners delete the pointees.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

struct PartPose;
struct PartCycle;
struct RoboBrain;

template Table<String, PartPose *, Array<PartPose *>, 0>::Table();
template Table<String, PartPose *, Array<PartPose *>, 0>::~Table();
template Table<String, PartCycle *, Array<PartCycle *>, 0>::Table();
template Table<String, PartCycle *, Array<PartCycle *>, 0>::~Table();
template Table<String, RoboBrain *, Array<RoboBrain *>, 0>::Table();
template Table<String, RoboBrain *, Array<RoboBrain *>, 0>::~Table();
