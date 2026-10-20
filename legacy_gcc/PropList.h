// PropList: the list of props that are on show and need a call every frame. Reconstructed from SQZ.EXE 0x42d010-0x42d2b0.
//
// A FrameProp (and so LODProp, CycleProp, NavProp) adds itself to this list when it is shown and takes itself out when it is
// hidden. The game then calls every prop on it once a frame: Update (virtual slot 7) with the time that passed, and UpdateLevel
// (slot 8, which only a LODProp does anything with) when the camera has moved.
//
// A prop can come off the list while the game is going through it, so taking one off only writes its place on the list down in
// a queue; the queue is worked through (FlushRemovedProps) before and after each pass. A prop can also be put on the "destroy
// later" list: when its place on the main list is cleared away, the prop itself is deleted too.
#pragma once
#include "Container.h"

struct Prop;

void AddUpdatingProp(Prop *prop);       // @0042d180 puts the prop on the list
void RemoveUpdatingProp(Prop *prop);    // @0042d1a0 queues every place the prop is on the list for removal
void DestroyPropLater(Prop *prop);      // @0042d1f0 the prop is deleted when its place on the list is cleared away
void UpdateProps(float seconds);        // @0042d210 slot 7 of every prop on the list
void UpdateLevels();                    // @0042d250 slot 8 of every prop on the list that is not about to come off it
void FlushRemovedProps();               // @0042d2b0 clears away the queued places (and deletes the props on the destroy list)
void ClearWorldObjects();               // @0042d010 empties the second and third sets of prop and path tables, then the lists
