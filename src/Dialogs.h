// Dialogs: the window procedures of the game's native Windows dialogs (the options dialog and the ones it opens), reconstructed
// from SQZ.EXE 0x4066b0-0x408600. Menu (Menu.cpp) starts the first level; the options dialog starts the others. The dialog
// templates (resource ids 106..129) live in the exe's resources and are not part of this file. What the dialogs change lives
// in the game's input object (Original.h, "the input object"), which stays original code.
//
//   template 128  Quit question          QuitQuestion       @406d60  (Yes / No)
//   template 129  Options                Options            @406cb0  (buttons open the three below)
//   template 116  Joystick               Joystick           @4078e0  opened by ShowJoystick      @4066b0
//   template 106  Keys and buttons       Keys               @407d40  opened by ShowKeys          @4066f0
//   template 115  Movement sensitivity   Sensitivity        @408410  opened by ShowSensitivity   @4068e0
//   template 113  Load / save game       SaveLoad           @406eb0  (opened by Menu::ActionLoadGame, and by the game for saving)
//   template 114  Name for a saved game  SaveName           @4077c0
//   template 112  Pick one of four       KeyKind            @407c90  (key, button, or both; opened by Keys)
//   template 111  Press a key            PressKey           @407b90
#pragma once
#include <windows.h>

namespace Dialogs {

INT_PTR CALLBACK QuitQuestion(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);       // @406d60
INT_PTR CALLBACK Options(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);            // @406cb0
INT_PTR CALLBACK Joystick(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);           // @4078e0
INT_PTR CALLBACK Keys(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);               // @407d40
INT_PTR CALLBACK Sensitivity(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);        // @408410
INT_PTR CALLBACK SaveLoad(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);           // @406eb0
INT_PTR CALLBACK SaveName(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);           // @4077c0
INT_PTR CALLBACK KeyKind(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);            // @407c90
INT_PTR CALLBACK PressKey(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);           // @407b90

// the launchers: open a dialog on top of 'parent', then repaint the parent; return 0
int __cdecl ShowJoystick(HWND parent);        // @4066b0
int __cdecl ShowKeys(HWND parent);            // @4066f0
int __cdecl ShowSensitivity(HWND parent);     // @4068e0

HANDLE __cdecl CreateSaveFile(HWND owner, const char *path);    // @406dc0 create the file (asks before overwriting); NULL after an error message
int    __cdecl IsValidSaveName(const char *name);               // @407730 non empty, letters and digits only

}
