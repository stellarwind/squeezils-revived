// GameLoop: the per-idle step of the application and a few small window helpers beside it (SQZ.EXE 0x43ed80-0x43ee9e).
#pragma once
#include <windows.h>

// the globals these use
inline int &OneSimPerRender() { return *(int *)0x4c76f4; }   // the [Defaults] "OneSimPerRender" switch
inline int &GamePaused()      { return *(int *)0x4c7700; }   // 1 while SetGamePaused(1) is in effect

// @0043ed80 (cdecl) Pauses (nonzero) or resumes the game: the application's input mode and the audio. Does nothing while the
// object in the global at 0x4c75e4 starts with a nonzero word.
void SetGamePaused(int paused);

// @0043edd0 (stdcall) The dialog procedure of the game's simple dialogs: accepts at once, and ends the dialog on the OK button.
INT_PTR CALLBACK SimpleDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// @0043ee10 (cdecl, returns 0) Called whenever the message queue is empty. Runs the simulation while the clock is behind and
// draws a frame when it is ahead; with OneSimPerRender it does one of each every time. Nothing while paused.
int GameIdle();

// @0043ee60 (cdecl, returns 1) Redraws (with their frames, now) every window above the game's window in the z order.
int RedrawWindowsAbove();
