// Compile-time checks that the reconstructed structs have the original offsets and sizes (Renderer, Scene, Warp, BodyPart, SqgModel).
#include "Renderer.h"
#include "Scene.h"
#include <stddef.h>
#define CHECK(f, off) static_assert(offsetof(Renderer, f) == off, "Renderer::" #f " offset")
CHECK(savedWidth, 0x00); CHECK(fullscreen, 0x0c); CHECK(displayState, 0x10); CHECK(windowObj, 0x14);
CHECK(hwnd, 0x18); CHECK(initializing, 0x1c); CHECK(doubleSize, 0x20); CHECK(ddraw, 0x24); CHECK(d3d, 0x28);
CHECK(d3drm, 0x2c); CHECK(clipper, 0x30); CHECK(depthBitMask, 0x34); CHECK(fmt, 0x38); CHECK(requestedWidth, 0x50);
CHECK(screenRect, 0x58); CHECK(originX, 0x68); CHECK(width, 0x70); CHECK(height, 0x74); CHECK(palette, 0x7c);
CHECK(deviceFound, 0x47c); CHECK(deviceGuids, 0x480); CHECK(deviceNames, 0x580); CHECK(chosenDevice, 0x980);
CHECK(hardware, 0x984); CHECK(monoColorModel, 0x988); CHECK(sceneFrame, 0x98c); CHECK(cameraFrame, 0x990);
CHECK(primary, 0x994); CHECK(backBuffer, 0x998); CHECK(depth, 0x99c); CHECK(frontPalette, 0x9a0);
CHECK(backPalette, 0x9a4); CHECK(device, 0x9a8); CHECK(textureColors, 0x9ac); CHECK(viewport, 0x9b8);
CHECK(nearPlane, 0x9bc); CHECK(farPlane, 0x9c0); CHECK(fovDegrees, 0x9c4); CHECK(ownsDDObjects, 0x9c8);
static_assert(sizeof(Renderer) == 0x9cc, "sizeof(Renderer)");
static_assert(D3DRMRENDER_GOURAUD == 0x89, "gouraud constant");
static_assert(sizeof(DDSURFACEDESC) == 0x6c && sizeof(DDCAPS) == 0x13c, "DD struct sizes");
static_assert(offsetof(D3DDEVICEDESC, dwDeviceRenderBitDepth) == 0x9c, "device desc render depth offset");
static_assert(offsetof(D3DDEVICEDESC, dcmColorModel) == 8, "device desc color model offset");

// Scene
static_assert(offsetof(Scene, renderer) == 0x00 && offsetof(Scene, warpSurface) == 0x04 &&
              offsetof(Scene, lightsFrame) == 0x08 && offsetof(Scene, sceneChild) == 0x0c &&
              offsetof(Scene, warp) == 0x10 && offsetof(Scene, warpWidth) == 0x14 &&
              offsetof(Scene, warpHeight) == 0x18, "Scene field offsets");
static_assert(sizeof(Scene) == 0x1c, "Scene size");

// Warp
#include "Warp.h"
static_assert(sizeof(WarpFace) == 0x48, "WarpFace size");
static_assert(offsetof(Warp, height) == 0x00 && offsetof(Warp, width) == 0x04 && offsetof(Warp, fovX) == 0x08 &&
              offsetof(Warp, fovY) == 0x0c && offsetof(Warp, current) == 0x10 && offsetof(Warp, faceCount) == 0x14 &&
              offsetof(Warp, outputDepth) == 0x18 && offsetof(Warp, faces) == 0x1c && offsetof(Warp, faceData) == 0x1cc,
              "Warp field offsets");
static_assert(sizeof(Warp) == 0x1e4, "Warp size");

// BodyPart
#include "BodyPart.h"
static_assert(offsetof(BodyPart, model) == 0x04 && offsetof(BodyPart, morph) == 0x08 && offsetof(BodyPart, frame) == 0x0c, "BodyPart fields");
static_assert(offsetof(AnimatedBody, partTable) == 0x04 && offsetof(AnimatedBody, cycle) == 0x2c && offsetof(AnimatedBody, time) == 0x30 &&
              offsetof(AnimatedBody, pose) == 0x40 && offsetof(AnimatedBody, blendLeft) == 0x48 && offsetof(AnimatedBody, rootFrame) == 0x4c, "AnimatedBody fields");
static_assert(offsetof(PartPose, direction) == 0x0c && offsetof(PartPose, up) == 0x18 && offsetof(PartPose, weights) == 0x24, "PartPose fields");
static_assert(offsetof(PartCycle, orientations) == 0x14 && offsetof(PartCycle, morph) == 0x24 && offsetof(PartCycle, cycle) == 0x34 && offsetof(PartCycle, weightCount) == 0x38, "PartCycle fields");
static_assert(offsetof(MorphCycle, noLoop) == 0x10 && offsetof(MorphCycle, speed) == 0x18 && offsetof(MorphCycle, times) == 0x20 && offsetof(MorphCycle, parts) == 0x24, "MorphCycle fields");
static_assert(sizeof(DisplayFormat) == 0x18, "DisplayFormat size");

// WarpFace field offsets
static_assert(offsetof(WarpFace, extentB) == 0x04 && offsetof(WarpFace, zero08) == 0x08 && offsetof(WarpFace, one) == 0x10 &&
              offsetof(WarpFace, halfA) == 0x14 && offsetof(WarpFace, halfB) == 0x18 && offsetof(WarpFace, halfA2) == 0x1c &&
              offsetof(WarpFace, k20) == 0x20 && offsetof(WarpFace, zero24) == 0x24 && offsetof(WarpFace, k28) == 0x28 &&
              offsetof(WarpFace, minusOne) == 0x2c && offsetof(WarpFace, flag) == 0x44, "WarpFace field offsets");

// SqgModel
#include "SqgModel.h"
static_assert(sizeof(SqgGroup) == 0x24 && sizeof(SqgDelta) == 0x10 && sizeof(SqgDeltaList) == 8 && sizeof(PZMorphMesh) == 0x0c,
              "Sqg struct sizes");
static_assert(sizeof(SqgModel) == 0x2c, "SqgModel size");

// Bitmap
#include "Bitmap.h"
static_assert(offsetof(Bitmap, desc) == 0x04 && offsetof(Bitmap, renderer) == 0x70 && offsetof(Bitmap, surface) == 0x74 &&
              offsetof(Bitmap, width) == 0x78 && offsetof(Bitmap, height) == 0x7c, "Bitmap fields");
static_assert(sizeof(Bitmap) == 0x80, "Bitmap size");

// String
#include "GameString.h"
static_assert(offsetof(StringRep, cap) == 0x4 && offsetof(StringRep, refs) == 0x8 && sizeof(StringRep) == 0xc, "StringRep layout");
static_assert(sizeof(String) == 4, "String size");

// Containers and the parsed ini files
#include "IniFile.h"
static_assert(offsetof(Array<String>, count) == 0x4 && offsetof(Array<String>, capacity) == 0x8 && offsetof(Array<String>, data) == 0xc &&
              sizeof(Array<String>) == 0x10 && sizeof(Array<IniFileSection *>) == 0x10, "Array layout");
static_assert(offsetof(StringList, vtbl) == 0x10 && sizeof(StringList) == 0x14 &&
              sizeof(IntList) == 0x14 && sizeof(Set<String, 23>) == 0x14, "PrimitiveArray / Set layout");
static_assert(offsetof(IniValueTable, keys) == 0x04 && offsetof(IniValueTable, values) == 0x18 && offsetof(IniValueTable, vtbl) == 0x2c &&
              sizeof(IniValueTable) == 0x30, "BufferObjectTable layout");
static_assert(offsetof(IniFileTable, keys) == 0x04 && offsetof(IniFileTable, values) == 0x18 && sizeof(IniFileTable) == 0x28, "Table layout");
static_assert(offsetof(IniFileSection, order) == 0x30 && sizeof(IniFileSection) == 0x44 && sizeof(IniFileCache) == 0x28, "ini file layout");

// Splash
#include "Splash.h"
static_assert(offsetof(Splash, picture) == 0x04 && offsetof(Splash, bar) == 0x08 && offsetof(Splash, step) == 0x0c && offsetof(Splash, steps) == 0x10 &&
              offsetof(Splash, barX) == 0x14 && offsetof(Splash, shown) == 0x1c && offsetof(Splash, file) == 0x20 &&
              offsetof(Splash, which) == 0x5c && offsetof(Splash, target) == 0x60, "Splash fields");

// Menu
#include "Menu.h"
static_assert(offsetof(Menu, itemCount) == 0x04 && offsetof(Menu, left) == 0x08 && offsetof(Menu, itemHeight) == 0x10 &&
              offsetof(Menu, items) == 0x18 && offsetof(Menu, anchorX) == 0x180 && offsetof(Menu, arg1) == 0x18c, "Menu fields");
static_assert(offsetof(Menu::Item, action) == 0x18 && offsetof(Menu::Item, highlighted) == 0x20, "Menu::Item fields");

// MoodFace
#include "MoodFace.h"
static_assert(offsetof(MoodFace, x) == 0x284 && offsetof(MoodFace, canvas) == 0x2a0 && offsetof(MoodFace, width) == 0x2a8 &&
              offsetof(MoodFace, lastMood) == 0x2b0 && offsetof(MoodFace, animationCount) == 0x2b4 && offsetof(MoodFace, frame) == 0x2b8 &&
              offsetof(MoodFace, lastTick) == 0x2c0 && offsetof(MoodFace, frameMs) == 0x2c4, "MoodFace fields");

// NutCounter
#include "NutCounter.h"
static_assert(offsetof(NutCounter, x) == 0x08 && offsetof(NutCounter, file) == 0x10 && offsetof(NutCounter, lastValue) == 0x24 && offsetof(NutCounter, mode) == 0x28, "NutCounter fields");

// Panel (partial)
#include "Panel.h"
static_assert(offsetof(Panel, digits) == 0x428 && offsetof(Panel, background) == 0x460 && offsetof(Panel, backgroundFile) == 0x464 && offsetof(Panel, compass) == 0x480, "Panel fields");

// StatusBar
#include "StatusBar.h"
static_assert(offsetof(StatusBar, food) == 0x34 && offsetof(StatusBar, foodShown) == 0x68 && offsetof(StatusBar, goodies) == 0x6c && offsetof(StatusBar, face) == 0x9c &&
              offsetof(StatusBar, deadMessage) == 0x37c && offsetof(StatusBar, messageY) == 0x388 && offsetof(StatusBar, goodieStep) == 0x364 && offsetof(StatusBar, faceState) == 0x378, "StatusBar fields");

// QuestItems
#include "QuestItems.h"
static_assert(offsetof(QuestItems, combinedFile) == 0x04 && offsetof(QuestItems, left) == 0x18 && offsetof(QuestItems, leftFile) == 0x1c && offsetof(QuestItems, middle) == 0x30 &&
              offsetof(QuestItems, right) == 0x48 && offsetof(QuestItems, rightFile) == 0x4c, "QuestItems fields");

// Panel (whole object)
static_assert(offsetof(Panel, status) == 0x08 && offsetof(Panel, quest) == 0x394 && offsetof(Panel, verticalChange) == 0x5c0 && offsetof(Panel, prevRow2) == 0x604 &&
              offsetof(Panel, fullTurn) == 0x610 && offsetof(Panel, font) == 0x618 && offsetof(Panel, message) == 0x630 && offsetof(Panel, note) == 0x694, "Panel fields 2");
#include "QuestPanel.h"
static_assert(offsetof(QuestPanel, places) == 0x68, "QuestPanel fields");
static_assert(offsetof(Panel, mode) == 0x450 && offsetof(Panel, event) == 0x614 && offsetof(Panel, fontHeight) == 0x61c &&
              offsetof(Panel, messageLines) == 0x62c && offsetof(Panel, prevRow0) == 0x5ec && offsetof(Panel, prevRow2) == 0x604, "Panel fields 3");
#include "GoodieList.h"
static_assert(offsetof(GoodieList, entries) == 0x1c && offsetof(GoodieList, canvas) == 0x24 && offsetof(GoodieList, height) == 0x2c &&
              offsetof(GoodieEntry, id) == 0x14 && offsetof(GoodieEntry, section) == 0x18 && offsetof(GoodieEntry, frame1) == 0x30, "GoodieList fields");
#include "CompassRuler.h"
static_assert(offsetof(CompassRuler, x) == 0x30 && offsetof(CompassRuler, sprite) == 0x38 && offsetof(CompassRuler, sector) == 0x58 &&
              offsetof(CompassRuler, pineRegion) == 0x64 && offsetof(CompassRuler, quarter) == 0x74 && offsetof(CompassRuler, name) == 0x7c &&
              offsetof(CompassRuler, path) == 0xf4, "CompassRuler fields");
#include "FrameProp.h"
static_assert(offsetof(FrameProp, childNames) == 0x40 && offsetof(FrameProp, children) == 0x54 && offsetof(FrameProp, messages) == 0x64 &&
              offsetof(FrameProp, body) == 0x94 && offsetof(FrameProp, cycle) == 0x98 && offsetof(FrameProp, actorIndex) == 0x9c &&
              offsetof(FrameProp, followMode) == 0xa0 && offsetof(FrameProp, startTime) == 0xa4 && offsetof(FrameProp, hideAtEnd) == 0xa8,
              "FrameProp fields");
static_assert(sizeof(CycleMessageTable) == 0x30 && offsetof(CycleMessageTable, values) == 0x18, "CycleMessageTable layout");
static_assert(offsetof(ScriptCaller, soul) == 0x08 && offsetof(ScriptCaller, actorIndex) == 0x10 && offsetof(ScriptCaller, prop) == 0x14,
              "ScriptCaller fields");
