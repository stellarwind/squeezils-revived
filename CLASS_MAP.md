# Squeezils class map

Generated from the RTTI table (`tools/rtti.tsv`). 149 plain classes plus about 150 template instances. Names come from the
binary's RTTI. The hierarchy shows the direct base, and the hex value is the primary (offset 0) vtable address in `SQZ.EXE` (base 0x400000); classes with several bases also have a second vtable at object+4.
Skeleton files for each class are in `Src\` (not compilable). The current reconstructed, compiling code is in `Repo\src` (the older copy in `Src_cpp\` is superseded).

Progress as of 2026-10-08: the renderer, scene, sky, UI, resources, props, bodies and the SqueezilSoul are reconstructed. Each is hooked into the unmodified game with `hook\` and checked by play tests. Commits are local to `Repo` (not pushed).

## Status of reconstruction

Rows are grouped by area. "Play-tested" means the hook ran in the real game with 0 mismatches. "Partly" means some paths were never run in play; see `TODO.md` for the details.

### Rendering and graphics
| Class | Source | State |
|---|---|---|
| `Renderer` | `Renderer.h/.cpp`, global `g_renderer` | done, play-tested. `CheckDesktopDepth` not run. `FadePalette` hooked |
| `ListenerTable` | `ListenerTable.h/.cpp` | done, play-tested |
| `Scene` (our name, no RTTI) | `Scene.h/.cpp`, global `DAT_004c75e0` | done. `SetQuality` and `EnsureWarpSurface` never ran |
| `Warp` (our name, no RTTI) | `Warp.h/.cpp`, `Scene +0x10` | done. `Resize` never ran |
| `SkyRenderer` | `Sky.h/.cpp` | done for sky mode 1 only. Modes 0 and 2 not reconstructed. `Resize` never ran |
| `Bitmap`, `Image`, `Gif`, `Texture` | `Bitmap`, `Image`, `Gif`, `Texture` `.h/.cpp` | done |
| `SqgModel` (meshes, morphing) | `SqgModel.h/.cpp` | done |
| `BodyPart`, `AnimatedBody` (D3DRM side) | `BodyPart.h/.cpp` | done |
| `Splash` (load screen) | `Splash.h/.cpp` | done. Windowed path (`Pump`, `Reset`) never ran |

### Resources, containers and files
| Class | Source | State |
|---|---|---|
| `GameString` (`SqzString`), `Container` templates | `GameString.*`, `Container.h` | done |
| `IniFile`, `IniReader` | `IniFile.*`, `IniReader.*` | done |
| `Resources` (textures and geometry) | `Resources.h/.cpp` | done, play-tested |
| `PropLoader` (prop files, `ReadPaths`, `FindPropTemplate`) | `PropLoader.*`, `LoadProp.cpp` | done |
| `Script` (`PostFunction`) | `Script.h/.cpp` | `PostFunction` done |

### World, props and animation
| Class | Source | State |
|---|---|---|
| `Prop`, `MeshProp`, `SpriteProp` | `Prop.h/.cpp` | done (uses `VirtualCall.h`) |
| `FrameProp`, `LODProp`, `CycleProp`, `NavProp` | `FrameProp.*`, `LODProp.*`, `CycleProp.*`, `NavProp.*` | done |
| `PropList` (updating-props list) | `PropList.h/.cpp` | done, play-tested |
| `Animation` (`MorphCycle`, `PartCycle`, `PartPose`, `PoseTable`, `AnimatedBody` player) | `Animation.h/.cpp` | done |
| `Simulation` (creature registry, lifecycle, `CollideCreatures`) | `Simulation.h/.cpp` | base class done. Derived simulations (`LandSimulation`, `RegionSimulation`, `PlaceSimulation`) not listed here |
| `World` | `World.h/.cpp` | done, never ran in play |

### Bodies and characters
| Class | Source | State |
|---|---|---|
| `Body` (base, `PlaceOnFace`) | `Body.h/.cpp` | done |
| `CameraBody`, `GoodieBody` | `CameraBody.*`, `GoodieBody.*` | done |
| `SqueezilBody` (animated body, clips, prototypes) | `SqueezilBody.*` | done |
| `SqueezilSoul` (state machine, ground and air movement) | `SqueezilSoul.h/.cpp`, `SqueezilSoulMove.cpp`, `SqueezilSoulInternal.h` | done, play-tested (66 hooks, 0 mismatches). Dying on the ground, `LieDown`, `GetUp`, `SkihopAir` never run. One crash seen once, not reproduced |

### Interface
| Class | Source | State |
|---|---|---|
| `Panel`, `PanelText`, `PanelCompass` | `Panel.*`, `PanelText.cpp`, `PanelCompass.cpp` | done. `Invalidate`/`DrawBackground` after a mode change untested |
| `StatusBar` | `StatusBar.*` | done. Some mood and goal paths not checked one by one |
| `CompassRuler` | `CompassRuler.*` | done. Looking up/down untested |
| `GoodieList`, `QuestItems`, `QuestPanel`, `MoodFace`, `NutCounter` | matching `.h/.cpp` | done |
| `Menu` | `Menu.h/.cpp` | done. Mouse and the six actions were not in the last log |
| `Dialogs` (native dialogs) | `Dialogs.h/.cpp` | done, confirmed by the user |

### Still skeletons only
Everything else in the RTTI list in the sections below is still a skeleton in `Src\`. The most likely next candidates are the derived simulations and the remaining soul and camera classes.

### Scene layout (0x1c bytes)
| Offset | Field |
|---|---|
| 0x00 | `Renderer *renderer` |
| 0x04 | warp surface (`IDirectDrawSurface`, offscreen plain, size `renderer.width/2` x `height/2`, doubled in doubleSize mode) |
| 0x08 | lights frame (child of `renderer.sceneFrame`) |
| 0x0c | scene frame (the child currently attached for rendering) |
| 0x10 | `Warp *` (optional backdrop warp effect) |
| 0x14, 0x18 | warp width, height |

Functions: ctor `0x476720`, dtor `0x476740`, `EnsureWarpSurface` `0x4767a0`, `Init` `0x4768f0`, `LoadWarp` `0x4769a0`,
`BeginFrame` `0x476af0` (clear z-buffer, then clear back buffer or blit the warp), `Render` `0x476cd0`
(`Viewport::Render(sceneFrame)`), `Present` `0x476d20` (`Blt` back to front), `SetQuality` `0x476ea0`, `CreateFrame` `0x476f40/0x476f90`,
`SetFrameTransform` `0x477040/0x4770d0`, `GetFrameMatrix` `0x477120`, `SetSceneChild` `0x4772d0`, `DeleteSceneChild` `0x477340`.

## Rendering-related classes: what is left

The props, bodies, screens and resource caches listed here are now reconstructed (see the tables above). What is still open:

- **Props**: all seven prop classes have code now (`MeshProp` and `SpriteProp` are in `Prop.cpp`).
- **Camera**: `CameraSoul`, `PlaceCameraSoul`, `RegionCameraSoul` (`CameraBody` is done).
- **Screens**: `FadeScreenDownFunction` and `PlayMovieFunction` are script actions and are not reconstructed.
- The hot D3DRM calls (about 217k frame `SetPosition` / `SetOrientation`) come from the per-frame update of bodies and props. That update is now in the reconstructed `Body`/`Prop` code.

## Inheritance tree (RTTI classes only)
The tree below comes from the binary's RTTI, so it lists only the classes the game itself names. Our own reconstructed
classes (renderer, scene, sky, UI panels and so on) have no RTTI entry and are in the next section instead.
- `Activity` (`0049d004`)
  - `FlyActivity` (`0049d12c`)
    - `FlyTargetActivity` (`0049d11c`)
  - `WalkActivity` (`0049d08c`)
    - `DieActivity` (`0049d09c`)
    - `MoveActivity` (`0049d07c`)
      - `TurnTargetActivity` (`0049d06c`)
    - `RegenActivity` (`0049d0ac`)
    - `StopActivity` (`0049d0dc`)
- `AnimatedBody` (`0049d7d4`)
- `Behavior` (`0049d0bc`)
  - `ChaseBehavior` (`0049d13c`)
  - `DeathBehavior` (`0049d0fc`)
  - `FleeBehavior` (`0049d15c`)
  - `PaceBehavior` (`0049d0ec`)
  - `SimpleFlyBehavior` (`0049d10c`)
  - `SpitBehavior` (`0049d14c`)
  - `WanderBehavior` (`0049d0cc`)
- `Body` (`0049d484`)
  - `CameraBody` (`0049d444`)
  - `GoodieBody` (`0049ff7c`)
  - `SqueezilBody` (`0049fe4c`)
- `BodyPart` (`0049d7c4`)
- `BodySimSoulTable` (`0049d5f4`)
- `BufferObject` (`0049d52c`)
  - `Character` (`0049ec8c`)
  - `Creature` (`0049d5c4`)
  - `Inventory` (`0049d6c4`)
- `Function` (`0049dabc`)
  - `AbortFunction` (`0049e8cc`)
  - `AddSpitNutsFunction` (`0049db14`)
  - `AddToFunction` (`0049e8b4`)
  - `AnimateCameraPathFunction` (`0049ecfc`)
  - `AnimateCameraPathOffFunction` (`0049ed14`)
  - `ChangeRoboResponseFunction` (`0049dafc`)
  - `ClearMessageFunction` (`0049e71c`)
  - `ClearSenseTargetsFunction` (`0049db2c`)
  - `ClearTimerMessageFunction` (`0049e734`)
  - `CompoundFunction` (`0049daa4`)
  - `CreateGoodieAtPositionFunction` (`0049e764`)
    - `CreateGoodieAtPositionWithVelocityFunction` (`0049e794`)
  - `CreatePropFunction` (`004a0e3c`)
    - `CreatePropAtPositionFunction` (`004a0fc4`)
    - `CreatePropAtSqueezilPositionFunction` (`004a0f14`)
      - `CreatePropTrackingSqueezilPositionFunction` (`004a0ec4`)
      - `CreatePropTrackingSqueezilXformFunction` (`004a1014`)
  - `DelayedExecuteFunction` (`0049e914`)
    - `RenderDelayedExecuteFunction` (`0049e92c`)
  - `ExecuteFunction` (`0049e8e4`)
  - `ExecuteWithPropFunction` (`0049e8fc`)
  - `FadeScreenDownFunction` (`0049e7ec`)
  - `FlashGoodieFunction` (`0049e7bc`)
  - `GoToRegionFunction` (`0049e834`)
  - `HidePropFunction` (`004a0e8c`)
  - `IfFunction` (`0049e874`)
    - `IfCompletedPlaceFunction` (`0049ea8c`)
    - `IfDyingOrDeadFunction` (`0049eb2c`)
    - `IfHasAllKeysFunction` (`0049eab4`)
    - `IfOnSurfaceTypeFunction` (`0049e84c`)
      - `IfOnFaceSetFunction` (`0049eadc`)
      - `IfOnOrAboveSurfaceTypeFunction` (`0049eb7c`)
    - `IfOnTreeFunction` (`0049e944`)
    - `IfPlayerDeadFunction` (`0049eb54`)
    - `IfPlayerFunction` (`0049eb04`)
    - `IfVariableFunction` (`0049ea5c`)
      - `IfVariableNonZeroFunction` (`0049ea2c`)
      - `IfVariableValueFunction` (`0049e9fc`)
        - `IfVariableEqualFunction` (`0049e96c`)
        - `IfVariableGreaterFunction` (`0049e9cc`)
        - `IfVariableLesserFunction` (`0049e99c`)
  - `KillSqueezilFunction` (`0049e81c`)
  - `MessageFunction` (`0049e6d4`)
  - `PlayAudioEffectFunction` (`0049e2a4`)
  - `PlayMovieFunction` (`0049e7d4`)
  - `RemoveCurrentGoodieFunction` (`0049e804`)
  - `SetCyclePropFunction` (`004a0f64`)
    - `SetCyclePropFrameDirFunction` (`004a0ff4`)
    - `SetCyclePropFrameFunction` (`004a0f84`)
    - `SetCyclePropFrameWrapFunction` (`004a0fa4`)
    - `SetCyclePropPathDirFunction` (`004a0f44`)
    - `SetCyclePropPathTimeFunction` (`004a0ea4`)
    - `SetCyclePropPathWrapFunction` (`004a0ef4`)
  - `SetRoboBehaviorFunction` (`0049dae4`)
  - `SetSurfaceTypeFunction` (`0049e74c`)
  - `SetToFunction` (`0049e89c`)
  - `ShowPropFunction` (`004a0e74`)
  - `StartAmbientLoopFunction` (`0049e2c4`)
  - `TimerMessageFunction` (`0049e6ec`)
  - `TimerTimeMessageFunction` (`0049e704`)
  - `TweakFloatFunction` (`0049de44`)
    - `AccelerateFunction` (`0049deb4`)
    - `BounceBackwardsFunction` (`0049df14`)
    - `LaunchFromFaceNormalFunction` (`0049df54`)
    - `ResetClawsFunction` (`0049de74`)
    - `ResetFrictionFunction` (`0049ded4`)
    - `ResetHopFunction` (`0049e014`)
    - `ResetJumpFunction` (`0049dfd4`)
    - `ResetJumpableFunction` (`0049df34`)
    - `ResetWalkSpeedsFunction` (`0049dff4`)
    - `SetClawsFunction` (`0049de94`)
    - `SetClawsZeroFunction` (`0049e074`)
    - `SetFrictionFunction` (`0049def4`)
    - `SetHopFunction` (`0049e054`)
    - `SetJumpFunction` (`0049df94`)
    - `SetJumpableFunction` (`0049df74`)
    - `SetMotionSpeedFunction` (`0049dfb4`)
    - `SetWalkSpeedsFunction` (`0049e034`)
- `FunctionArgument` (`0049da74`)
  - `BehaviorTypeFunctionArgument` (`0049dad4`)
  - `FloatFunctionArgument` (`0049da84`)
  - `IntFunctionArgument` (`0049da7c`)
  - `StimulusTypeFunctionArgument` (`0049dadc`)
  - `StringFunctionArgument` (`0049da9c`)
  - `Vector3fFunctionArgument` (`0049da94`)
- `InputEventTarget` (`0049d2c4`)
- `Keylist` (`0049e314`)
- `Menu` (`0049d28c`)
- `MessageMap` (`0049d2b4`)
  - `GrantMessageMap` (`0049f844`)
  - `Soul` (`0049d2a4`)
    - `SimulationSoul` (`0049d2ec`)
      - `CameraSoul` (`0049d33c`)
        - `PlaceCameraSoul` (`0049d414`)
        - `RegionCameraSoul` (`0049d3c4`)
      - `PhysicalSoul` (`0049ebdc`)
        - `GoodieSoul` (`0049ff44`)
        - `SqueezilSoul` (`0049f74c`)
          - `SqueezilPlayerSoul` (`0049d91c`)
          - `SqueezilRegionSoul` (`0049e12c`)
          - `SqueezilRoboSoul` (`0049d18c`)
- `Prop` (`0049db4c`)
  - `FrameProp` (`0049dc24`)
    - `CycleProp` (`0049dd44`)
      - `NavProp` (`0049ddac`)
    - `LODProp` (`0049dcb4`)
  - `MeshProp` (`0049db8c`)
  - `SpriteProp` (`0049dbe4`)
- `Simulation` (`0049d644`)
  - `LandSimulation` (`0049d82c`)
  - `LocationSimulation` (`0049d60c`)
    - `PlaceSimulation` (`0049e224`)
    - `RegionSimulation` (`0049d70c`)
- `Splash` (`0049ed34`)
- `SqzInputManager` (`0049e304`)
- `exception` (`004a19e4`)
  - `bad_cast` (`004a186c`)
  - `bad_typeid` (`004a17e4`)
    - `__non_rtti_object` (`004a17d4`)
- `type_info` (`004a177c`)


## Our own classes (no RTTI name)
These are reconstructed in `Repo\src` but are not in the inheritance tree. The name is ours, chosen from the code. The
header is the place to look.

| Class | Header | Role |
|---|---|---|
| `Renderer` | `Renderer.h` | DirectDraw and Direct3D Retained Mode setup: display modes, windowing, device, surfaces, palettes, viewport. Global `g_renderer` |
| `ListenerTable` | `ListenerTable.h` | the window object's registry of 16 callback/context pairs (offset 0 of the window object) |
| `Scene` | `Scene.h` | per-frame draw path on top of the renderer: z-buffer and back-buffer clear, `Viewport::Render`, present blit, scene frames. Global `DAT_004c75e0` |
| `Warp` | `Warp.h` | the sky object (`Scene +0x10`). Loads the six-view cube map and picks the face the camera looks at each frame |
| `SkyRenderer` | `Sky.h` | software renderer behind `Warp`: per-row rays into the fisheye faces, fixed-point parabola walk. Mode 1 only |
| `Bitmap` | `Bitmap.h` | DirectDraw offscreen surface for 2D art (HUD, digits, load bar), with the 8 bit `.bmp` loader. Seven virtuals used through the original vtable |
| `String` (`SqzString`) | `GameString.h` | reference counted, copy-on-write string with case-insensitive compares |
| `IniFileSection`, `IniFileCache`, `IniReader` | `IniFile.h`, `IniReader.h` | the parsed INI files, the global cache by path, and the settings cursor the game reads through |
| `Array`, `PrimitiveArray`, `Set`, `Table`, `BufferObjectTable` | `Container.h` | the game's container templates (one template for many instantiations) |
| `SqgModel` (+ `SqgGroup`, `SqgDelta`, `PZMorphMesh`) | `SqgModel.h` | the `.sqg` model format: loader, mesh builder, morph update |
| `Panel`, `StatusBar`, `CompassRuler`, `MoodFace`, `NutCounter`, `QuestItems`, `QuestPanel`, `GoodieList` | matching `.h` | the in-game HUD parts |
| `World` | `World.h` | world-level code: `CreateScene`, lights, nav mesh loading. Hook id `world`, never ran in play |
| `Splash`, `Menu` | `Splash.h`, `Menu.h` | load screen and main menu. These two do have RTTI entries (see the tree) |
| `Dialogs`, `Resources`, `Gif`, `Image`, `Texture`, `Script` | matching `.h` | native dialogs, texture and geometry resources, GIF reader, image conversion, texture upload, script action posting |

## Template families (instances, not listed individually)

| Family | Instances | Notes |
|---|---|---|
| `Array<T>` | 37 | pointer arrays such as `Array<IDirect3DRMFrame*>`, `Array<Character*>` |
| `PrimitiveArray<T,N>` | 38 | int / float arrays with fixed size |
| `Set<T,N>` | 30 | built on `PrimitiveArray` |
| `Table<SqzString, T, Array<T>, N>` | 23 | string-keyed lookup, for example frame, texture, `AudioSample` and `Function` tables |
| `BufferObjectArray<T,N>` | 20 | arrays of `BufferObject` subclasses, such as `Character` |
