@echo off
rem Build sqzhook.dll. Needs 32-bit MinGW-w64 on PATH and the DirectX 7 SDK headers (set DX7_INCLUDE).
rem Usage: build.bat   then copy sqzhook.dll next to SQZ.EXE (together with the proxy ddraw.dll).
setlocal
if "%DX7_INCLUDE%"=="" (echo Set DX7_INCLUDE to the DirectX 7 SDK include folder & exit /b 1)
cd /d "%~dp0"
g++ -m32 -std=c++17 -O1 -shared -static -s -Wno-invalid-offsetof -Wno-deprecated-enum-enum-conversion ^
  -I..\src -I"%DX7_INCLUDE%" ^
  -DDirectDrawCreate=Hook_DirectDrawCreate -DDirectDrawCreateClipper=Hook_DirectDrawCreateClipper -DDirect3DRMCreate=Hook_Direct3DRMCreate -DD3DRMCreateColorRGB=Hook_D3DRMCreateColorRGB ^
  -o sqzhook.dll hook.cpp ..\src\Renderer.cpp ..\src\ListenerTable.cpp ..\src\Scene.cpp ..\src\Warp.cpp ..\src\SqgModel.cpp ..\src\Texture.cpp ..\src\BodyPart.cpp ..\src\Sky.cpp ..\src\Gif.cpp ..\src\Image.cpp ..\src\Bitmap.cpp ..\src\GameString.cpp ..\src\IniReader.cpp ..\src\IniFile.cpp ..\src\Splash.cpp ..\src\Menu.cpp ..\src\Dialogs.cpp ..\src\World.cpp ..\src\TextureDefaults.cpp ..\src\Resources.cpp ..\src\Prop.cpp ..\src\FrameProp.cpp ..\src\LODProp.cpp ..\src\CycleProp.cpp ..\src\NavProp.cpp ..\src\NavSurface.cpp ..\src\NavSurfaceRead.cpp ..\src\NavSurfaceMove.cpp ..\src\PropLoader.cpp ..\src\LoadProp.cpp ..\src\PropList.cpp ..\src\Simulation.cpp ..\src\Body.cpp ..\src\GoodieBody.cpp ..\src\CameraBody.cpp ..\src\SqueezilBody.cpp ..\src\SqueezilSoul.cpp ..\src\SqueezilSoulMove.cpp ..\src\Script.cpp ..\src\Animation.cpp ..\src\MoodFace.cpp ..\src\NutCounter.cpp ..\src\Panel.cpp ..\src\PanelText.cpp ..\src\PanelCompass.cpp ..\src\StatusBar.cpp ..\src\GoodieList.cpp ..\src\CompassRuler.cpp ..\src\QuestItems.cpp ..\src\QuestPanel.cpp -lwinmm -ldxguid -lgdi32 -luser32
