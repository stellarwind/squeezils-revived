// Dialogs: see Dialogs.h. Every procedure follows the Windows dialog contract: return nonzero when the message was handled
// (for WM_INITDIALOG: nonzero lets Windows pick the focus), and close the dialog with EndDialog.
#include "Dialogs.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <commctrl.h>
#include "Original.h"

namespace Dialogs {

// the repaint the launchers and the error paths ask the parent window for
static const UINT kRepaintAll = RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_FRAME;

// ---- control and template ids (the exe's dialog resources) ----
enum {
    TEMPLATE_QUIT = 128, TEMPLATE_OPTIONS = 129, TEMPLATE_JOYSTICK = 116, TEMPLATE_KEYS = 106, TEMPLATE_SENSITIVITY = 115,
    TEMPLATE_SAVE_NAME = 114, TEMPLATE_KEY_KIND = 112, TEMPLATE_PRESS_KEY = 111,

    // Options
    IDC_OPTIONS_KEYS = 1071, IDC_OPTIONS_JOYSTICK = 40042, IDC_OPTIONS_SENSITIVITY = 1072,
    // Joystick
    IDC_JOY_NONE = 1022, IDC_JOY_STICK = 1023, IDC_JOY_STICK_ALT = 1024, IDC_JOY_CALIBRATE = 1065,
    // Keys
    IDC_KEYS_LIST = 1028, IDC_KEYS_KEY_BAR = 1036, IDC_KEYS_BUTTON_BAR = 1068, IDC_KEYS_CHANGE = 1039,
    // Press a key
    IDC_PRESS_KEY_EDIT = 1041,
    // Pick one of four
    IDC_KIND_A = 1045, IDC_KIND_B = 1043, IDC_KIND_OK = 1047,
    // Load / save
    IDC_SAVES_LIST = 1050, IDC_SAVES_SAVE = 1048, IDC_SAVES_DELETE = 1049, IDC_SAVES_LOAD = 1051,
    // Name for a saved game
    IDC_NAME_OK = 1047, IDC_NAME_EDIT = 1053,
    // Sensitivity
    IDC_SENS_1 = 1056, IDC_SENS_2 = 1057, IDC_SENS_3 = 1058, IDC_SENS_OK = 1047,
};

// ---- the quit question and the options dialog ----

INT_PTR CALLBACK QuitQuestion(HWND dlg, UINT msg, WPARAM wParam, LPARAM)
{
    if (msg == WM_INITDIALOG)
        return 1;
    if (msg != WM_COMMAND)
        return 0;
    if (LOWORD(wParam) == IDOK) {
        EndDialog(dlg, 1);
        return 0;
    }
    if (LOWORD(wParam) == IDCANCEL) {
        EndDialog(dlg, 0);
        return 0;
    }
    return 0;
}

INT_PTR CALLBACK Options(HWND dlg, UINT msg, WPARAM wParam, LPARAM)
{
    if (msg == WM_INITDIALOG)
        return 1;
    if (msg != WM_COMMAND)
        return 0;
    switch (LOWORD(wParam)) {
    case IDOK:                     EndDialog(dlg, 1); return 0;
    case IDCANCEL:                 EndDialog(dlg, 0); return 0;
    case IDC_OPTIONS_JOYSTICK:     ShowJoystick(dlg); return 0;
    case IDC_OPTIONS_KEYS:         ShowKeys(dlg); return 0;
    case IDC_OPTIONS_SENSITIVITY:  ShowSensitivity(dlg); return 0;
    }
    return 0;
}

static int ShowDialog(HWND parent, int templateId, DLGPROC proc)
{
    DialogBoxParamA(GetInstance(), MAKEINTRESOURCE(templateId), parent, proc, 0);
    RedrawWindow(parent, NULL, NULL, kRepaintAll);
    return 0;
}

int __cdecl ShowJoystick(HWND parent)     { return ShowDialog(parent, TEMPLATE_JOYSTICK, (DLGPROC)Joystick); }
int __cdecl ShowKeys(HWND parent)         { return ShowDialog(parent, TEMPLATE_KEYS, (DLGPROC)Keys); }
int __cdecl ShowSensitivity(HWND parent)  { return ShowDialog(parent, TEMPLATE_SENSITIVITY, (DLGPROC)Sensitivity); }

// ---- joystick ----

static void NoJoystickMessage(HWND dlg)
{
    MessageBoxA(dlg, "No joystick detected. Please check your connection\n"
                     "and run the Joystick Calibration application using\n"
                     "the windows control panel.", "Error:No Joystick Detected", MB_ICONHAND);
    RedrawWindow(dlg, NULL, NULL, kRepaintAll);
}

INT_PTR CALLBACK Joystick(HWND dlg, UINT msg, WPARAM wParam, LPARAM)
{
    InputSystem *input = GetInputSystem();
    if (msg == WM_INITDIALOG) {
        int mode = InputJoystickMode(input);
        if (mode == 0)
            CheckRadioButton(dlg, IDC_JOY_NONE, IDC_JOY_STICK_ALT, IDC_JOY_NONE);
        else if (mode == 1)
            CheckRadioButton(dlg, IDC_JOY_NONE, IDC_JOY_STICK_ALT, IDC_JOY_STICK);
        else if (mode == 2)
            CheckRadioButton(dlg, IDC_JOY_NONE, IDC_JOY_STICK_ALT, IDC_JOY_STICK_ALT);
        return 1;
    }
    if (msg != WM_COMMAND)
        return 0;

    int id = LOWORD(wParam);
    if (id == IDOK) {
        int none = IsDlgButtonChecked(dlg, IDC_JOY_NONE);
        int stick = IsDlgButtonChecked(dlg, IDC_JOY_STICK);
        int stickAlt = IsDlgButtonChecked(dlg, IDC_JOY_STICK_ALT);
        if (stickAlt && !InputJoystickPresent(input)) {
            NoJoystickMessage(dlg);
            return 0;
        }
        if (none) {
            InputSetJoystickMode(input, 0);
        } else if (stick) {
            InputSetJoystickMode(input, 1);
        } else if (stickAlt) {
            if (!InputSetJoystickMode(input, 2)) {
                MessageBoxA(dlg, "Unable to connect with joystick", "Error", MB_ICONEXCLAMATION);
                EndDialog(dlg, 0);
                RedrawWindow(dlg, NULL, NULL, kRepaintAll);
                return 0;
            }
        }
        EndDialog(dlg, 1);
    } else {
        if (id == IDCANCEL)
            EndDialog(dlg, 0);
        if (id == IDC_JOY_CALIBRATE) {
            if (!InputJoystickPresent(input)) {
                NoJoystickMessage(dlg);
                return 0;
            }
            void *calibration = InputCalibration(input);
            CalibrationStart(calibration);
            if (!CalibrationCheck(calibration)) {
                if (!CalibrationRun(calibration, dlg))
                    MessageBoxA(dlg, "Unable to connect with joystick", "Error", MB_ICONEXCLAMATION);
                else
                    MessageBoxA(dlg, "Joystick Calibration Complete!", "Calibration", MB_ICONEXCLAMATION);
                RedrawWindow(dlg, NULL, NULL, kRepaintAll);
                return 0;
            }
            InputSetJoystickMode(input, 2);
            CheckRadioButton(dlg, IDC_JOY_NONE, IDC_JOY_STICK_ALT, IDC_JOY_STICK_ALT);
            EndDialog(dlg, 1);
            return 1;
        }
    }
    return 0;
}

// ---- press a key, and the choice of what to change ----

static int *const kWaitingForKey = (int *)0x4b9510;     // the first edit change is the dialog filling in the old key name; 0 until then

INT_PTR CALLBACK PressKey(HWND dlg, UINT msg, WPARAM wParam, LPARAM)
{
    if (msg == WM_INITDIALOG) {
        SetWindowTextA(GetDlgItem(dlg, IDC_PRESS_KEY_EDIT), kKeyName);
        return 1;
    }
    if (msg != WM_COMMAND)
        return 0;

    int id = LOWORD(wParam), notification = HIWORD(wParam);
    if (id == IDCANCEL) {
        *kWaitingForKey = 0;
        EndDialog(dlg, 0);
        return 0;
    }
    if (notification == EN_CHANGE) {
        if (id == IDC_PRESS_KEY_EDIT && *kWaitingForKey) {
            *(int *)kKeyName = 3;                       // the room EM_GETLINE is given
            SendMessageA(GetDlgItem(dlg, IDC_PRESS_KEY_EDIT), EM_GETLINE, 0, (LPARAM)kKeyName);
            *kWaitingForKey = 0;
            EndDialog(dlg, 1);
            return 0;
        }
        if (id == IDC_PRESS_KEY_EDIT && !*kWaitingForKey)
            *kWaitingForKey = 1;
    }
    return 0;
}

INT_PTR CALLBACK KeyKind(HWND dlg, UINT msg, WPARAM wParam, LPARAM)
{
    if (msg == WM_INITDIALOG) {
        CheckDlgButton(dlg, IDC_KIND_A, 1);
        CheckDlgButton(dlg, IDC_KIND_B, 1);
        return 1;
    }
    if (msg != WM_COMMAND)
        return 0;
    if (LOWORD(wParam) == IDCANCEL) {
        EndDialog(dlg, 0);
        return 0;
    }
    if (LOWORD(wParam) == IDC_KIND_OK) {
        // answers: 1 both boxes checked, 2 only B, 3 only A, 4 neither
        int a = IsDlgButtonChecked(dlg, IDC_KIND_A);
        int b = IsDlgButtonChecked(dlg, IDC_KIND_B);
        int answer;
        if (a)
            answer = b ? 1 : 3;
        else
            answer = b ? 2 : 4;
        EndDialog(dlg, answer);
        return 0;
    }
    return 0;
}

// ---- keys and buttons ----

static int *const kBindingsChanged = (int *)0x4b9514;    // something was reassigned; OK then saves it

static void MessageNotChangeable(HWND dlg)
{
    MessageBoxA(dlg, "Sorry, this key is not changeable!", "Reassign Key Error", MB_OK);
}

// replace the selected line of the list with its new text and select it again
static void ReplaceListLine(HWND list, int selected, const char *text)
{
    SendMessageA(list, LB_DELETESTRING, selected, 0);
    int index = (int)SendMessageA(list, LB_ADDSTRING, 0, (LPARAM)text);
    SendMessageA(list, LB_SETCURSEL, index, 0);
}

INT_PTR CALLBACK Keys(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    InputSystem *input = GetInputSystem();
    char line[100];
    int slot = -1;           // which of the two bindings an arrow key or scroll arrow picks (-1: the one chosen by the key kind)
    int buttonBar = 0;       // the scroll arrow belongs to the button bar, not the key bar

    if (msg == WM_INITDIALOG) {
        HWND list = GetDlgItem(dlg, IDC_KEYS_LIST);
        for (int n = (int)SendMessageA(list, LB_GETCOUNT, 0, 0); n > 0; n--)
            SendMessageA(list, LB_DELETESTRING, 0, 0);
        for (int i = 0; i < 51; i++)
            if (InputActionName(input, i, line) > 0)
                SendMessageA(list, LB_ADDSTRING, 0, (LPARAM)line);
        SetFocus(list);
        SendMessageA(list, LB_SETCURSEL, 0, 0);
        *kBindingsChanged = 0;
        SetScrollRange(GetDlgItem(dlg, IDC_KEYS_KEY_BAR), SB_CTL, 0, 1, FALSE);
        SetScrollRange(GetDlgItem(dlg, IDC_KEYS_BUTTON_BAR), SB_CTL, 0, 1, FALSE);
        return 0;
    }

    if (msg == WM_COMMAND) {
        int id = LOWORD(wParam);
        if (id == IDOK || HIWORD(wParam) == LBN_DBLCLK) {
            HWND list = GetDlgItem(dlg, IDC_KEYS_LIST);
            if (list == GetFocus()) {
                // double click or Enter on a line: ask for the new key
                int selected = (int)SendMessageA(list, LB_GETCURSEL, 0, 0);
                SendMessageA(list, LB_GETTEXT, selected, (LPARAM)line);
                InputBindingLabel(input, line, kKeyName);
                if (DialogBoxParamA(GetInstance(), MAKEINTRESOURCE(TEMPLATE_PRESS_KEY), dlg, (DLGPROC)PressKey, 0)) {
                    if (IsCharAlphaNumericA(kKeyName[0]) || kKeyName[0] == ' ') {
                        if (InputAssignKey(input, line, kKeyName)) {
                            ReplaceListLine(list, selected, line);
                            *kBindingsChanged = 1;
                            return 0;
                        }
                        MessageNotChangeable(dlg);
                    }
                }
                return 0;
            }
            if (id == IDOK) {
                if (!*kBindingsChanged || InputSaveBindings(input))
                    EndDialog(dlg, 1);
            }
            return 0;
        }
        if (id == IDCANCEL) {
            InputRevertBindings(input, 1);
            EndDialog(dlg, 1);
            return 0;
        }
        if (id == IDC_KEYS_CHANGE) {
            HWND list = GetDlgItem(dlg, IDC_KEYS_LIST);
            int selected = (int)SendMessageA(list, LB_GETCURSEL, 0, 0);
            SendMessageA(list, LB_GETTEXT, selected, (LPARAM)line);
            int answer = (int)DialogBoxParamA(GetInstance(), MAKEINTRESOURCE(TEMPLATE_KEY_KIND), dlg, (DLGPROC)KeyKind, 0);
            if (answer == 1 || answer == 2) {
                if (answer == 1) {
                    if (!InputSetKeySlot(input, line, 2))
                        MessageBoxA(dlg, "Error changing key value!", "Reassign Key Error", MB_OK);
                    if (!InputSetButtonSlot(input, line, 2))
                        MessageBoxA(dlg, "Error changing button value!", "Reassign Key Error", MB_OK);
                } else {
                    InputSetKeySlot(input, line, 3);
                    InputSetButtonSlot(input, line, 3);
                }
                SendMessageA(list, LB_DELETESTRING, selected, 0);
                int index = (int)SendMessageA(list, LB_ADDSTRING, 0, (LPARAM)line);
                SendMessageA(list, LB_SETCURSEL, index, 0);
            } else if (answer == 3 || answer == 4) {
                InputRevertBindings(input, answer == 3 ? 1 : 2);
                SendMessageA(dlg, WM_INITDIALOG, 0, 0);       // fill the list again
            }
            *kBindingsChanged = 1;
            return 1;
        }
        return 1;
    }

    if (msg == WM_VSCROLL || msg == WM_VKEYTOITEM) {
        if (msg == WM_VSCROLL) {
            // the arrows of the two small scroll bars stand in for the left/right keys
            HWND bar = (HWND)lParam;
            if (GetDlgItem(dlg, IDC_KEYS_KEY_BAR) == bar || GetDlgItem(dlg, IDC_KEYS_BUTTON_BAR) == bar) {
                int code = LOWORD(wParam);
                if (code == SB_LINEUP) {
                    slot = 0;
                    buttonBar = GetDlgItem(dlg, IDC_KEYS_KEY_BAR) != bar;
                    SetScrollPos(bar, SB_CTL, 1, TRUE);
                } else if (code == SB_LINEDOWN) {
                    slot = 1;
                    buttonBar = GetDlgItem(dlg, IDC_KEYS_KEY_BAR) != bar;
                    SetScrollPos(bar, SB_CTL, 0, TRUE);
                }
            }
        } else {
            switch (LOWORD(wParam)) {
            case VK_SPACE: break;
            case VK_LEFT:  slot = 1; break;
            case VK_RIGHT: slot = 0; break;
            default:       return -1;
            }
        }
        HWND list = GetDlgItem(dlg, IDC_KEYS_LIST);
        if (slot == -1 && list != (HWND)lParam)
            return 0;
        int selected = (int)SendMessageA(list, LB_GETCURSEL, 0, 0);
        SendMessageA(list, LB_GETTEXT, selected, (LPARAM)line);
        int changed = buttonBar ? InputSetButtonSlot(input, line, slot) : InputSetKeySlot(input, line, slot);
        if (changed) {
            ReplaceListLine(list, selected, line);
            *kBindingsChanged = 1;
        } else {
            MessageNotChangeable(dlg);
        }
        return list == (HWND)lParam ? -2 : 0;       // -2: the list does not act on the key itself
    }
    return 0;
}

// ---- movement sensitivity ----

INT_PTR CALLBACK Sensitivity(HWND dlg, UINT msg, WPARAM wParam, LPARAM)
{
    HWND slider1 = GetDlgItem(dlg, IDC_SENS_1);
    HWND slider2 = GetDlgItem(dlg, IDC_SENS_2);
    HWND slider3 = GetDlgItem(dlg, IDC_SENS_3);
    if (msg == WM_INITDIALOG) {
        // each slider runs 0..10 and shows the stored value (0.0 to 1.0, in steps of 0.1) times 10.1
        SendMessageA(slider1, TBM_SETRANGE, TRUE, MAKELONG(0, 10));
        SendMessageA(slider2, TBM_SETRANGE, TRUE, MAKELONG(0, 10));
        SendMessageA(slider3, TBM_SETRANGE, TRUE, MAKELONG(0, 10));
        SendMessageA(slider1, TBM_SETPOS, TRUE, (LONG)(*kSensitivity1 * 10.1f));
        SendMessageA(slider3, TBM_SETPOS, TRUE, (LONG)(*kSensitivity3 * 10.1f));
        SendMessageA(slider2, TBM_SETPOS, TRUE, (LONG)(*kSensitivity2 * 10.1f));
        return 1;
    }
    if (msg != WM_COMMAND)
        return 0;
    if (LOWORD(wParam) == IDC_SENS_OK) {
        int pos1 = (int)SendMessageA(slider1, TBM_GETPOS, 0, 0);
        SetSensitivity1((float)pos1 * 0.1f);
        int pos3 = (int)SendMessageA(slider3, TBM_GETPOS, 0, 0);
        float value3 = (float)pos3 * 0.1f;
        GameLog("settingpercent: %f pos:%d \n", (double)value3, pos3);
        SetSensitivity3(value3);
        int pos2 = (int)SendMessageA(slider2, TBM_GETPOS, 0, 0);
        SetSensitivity2((float)pos2 * 0.1f);
        EndDialog(dlg, 1);
        return 1;
    }
    if (LOWORD(wParam) == IDCANCEL) {
        EndDialog(dlg, 0);
        return 0;
    }
    return 0;
}

// ---- load and save games ----

// A line of the list reads "<name><padding to 14 characters><month>/<day>/<year>"; the name is what comes before the first space.
static void SavedNameOf(const char *line, char *name)
{
    int length = 0;
    if (line[0] != ' ') {
        length = 1;
        while (line[length] != ' ' && line[length] != 0)
            length++;
    }
    memcpy(name, line, length);
    name[length] = 0;
}

static void FillSaveList(HWND list)
{
    for (int n = (int)SendMessageA(list, LB_GETCOUNT, 0, 0); n > 0; n--)
        SendMessageA(list, LB_DELETESTRING, 0, 0);

    int count = 0;
    char pattern[200];
    strcpy(pattern, kSaveGameDirectory);
    strcat(pattern, "*.*");
    WIN32_FIND_DATAA found;
    HANDLE search = FindFirstFileA(pattern, &found);
    if (search != INVALID_HANDLE_VALUE) {
        do {                                                   // (the original never closes the search handle)
            if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            char line[100], number[16];
            strcpy(line, found.cFileName);
            for (int pad = 14 - (int)strlen(line); pad > 0; pad--)
                strcat(line, " ");
            SYSTEMTIME when;
            FileTimeToSystemTime(&found.ftLastAccessTime, &when);
            strcat(line, _itoa(when.wMonth, number, 10));
            strcat(line, "/");
            strcat(line, _itoa(when.wDay, number, 10));
            strcat(line, "/");
            strcat(line, _itoa(when.wYear, number, 10));
            SendMessageA(list, LB_ADDSTRING, 0, (LPARAM)line);
            count++;
        } while (FindNextFileA(search, &found));
    }
    if (count > 0) {
        SendMessageA(list, LB_SETCURSEL, 0, 0);
        SetFocus(list);
    }
}

INT_PTR CALLBACK SaveLoad(HWND dlg, UINT msg, WPARAM wParam, LPARAM)
{
    HWND list = GetDlgItem(dlg, IDC_SAVES_LIST);
    if (msg == WM_INITDIALOG) {
        FillSaveList(list);
        return 0;
    }
    if (msg != WM_COMMAND)
        return 0;

    char line[100], name[100], text[200], path[200];
    switch (LOWORD(wParam)) {
    case IDOK:
        EndDialog(dlg, 1);
        return 0;
    case IDCANCEL:
        EndDialog(dlg, 0);
        return 0;

    case IDC_SAVES_DELETE: {
        int selected = (int)SendMessageA(list, LB_GETCURSEL, 0, 0);
        if (selected < 0)
            return 0;
        SendMessageA(list, LB_GETTEXT, selected, (LPARAM)line);
        SavedNameOf(line, name);
        strcpy(text, "Delete ");
        strcat(text, name);
        strcat(text, " ?");
        if (MessageBoxA(dlg, text, "Delete Game", MB_YESNO) == IDYES) {
            strcpy(path, kSaveGameDirectory);
            strcat(path, name);
            if (DeleteFileA(path)) {
                SendMessageA(list, LB_DELETESTRING, selected, 0);
                SendMessageA(list, LB_SETCURSEL, 0, 0);
            } else {
                MessageBoxA(dlg, "Could not delete saved game!", "Error", MB_OK);
                SendMessageA(list, LB_SETCURSEL, selected, 0);
            }
        }
        return 0;
    }

    case IDC_SAVES_SAVE: {
        if (!DialogBoxParamA(GetInstance(), MAKEINTRESOURCE(TEMPLATE_SAVE_NAME), dlg, (DLGPROC)SaveName, 0)) {
            MessageBoxA(dlg, "Did not save game", "Save Game", MB_OK);
            return 0;
        }
        strcpy(path, kSaveGameDirectory);
        strcat(path, kSaveGameName);
        HANDLE file = CreateSaveFile(dlg, path);
        if (file) {
            CloseHandle(file);
            strcpy(text, "Saved game: ");
            strcat(text, kSaveGameName);
            MessageBoxA(dlg, text, "Save Game", MB_ICONEXCLAMATION);
            if (!SimulationCanSave(GetCurrentSimulation()))
                MessageBoxA(dlg, "When you load this save game, you will be\nin the region you started from.", "Save Game", MB_OK);
            if (SimulationSave(GetCurrentSimulation(), path)) {
                SendMessageA(dlg, WM_INITDIALOG, 0, 0);       // list the new file
                return 0;
            }
        }
        strcpy(text, "Could not save game: ");
        strcat(text, kSaveGameName);
        MessageBoxA(dlg, text, "Save Game", MB_OK);
        return 0;
    }

    case IDC_SAVES_LOAD: {
        int selected = (int)SendMessageA(list, LB_GETCURSEL, 0, 0);
        if (selected < 0)
            return 0;
        SendMessageA(list, LB_GETTEXT, selected, (LPARAM)line);
        SavedNameOf(line, name);
        strcpy(kLoadGameName, name);
        EndDialog(dlg, 1);
        return 1;
    }
    }
    return 0;
}

INT_PTR CALLBACK SaveName(HWND dlg, UINT msg, WPARAM wParam, LPARAM)
{
    if (msg == WM_INITDIALOG) {
        strcpy(kSaveGameName, "");
        SendMessageA(GetDlgItem(dlg, IDC_NAME_EDIT), EM_LIMITTEXT, 10, 0);
        return 1;
    }
    if (msg != WM_COMMAND)
        return 0;
    if (LOWORD(wParam) == IDC_NAME_OK) {
        *(int *)kSaveGameName = 10;                                  // the room EM_GETLINE is given
        int length = (int)SendMessageA(GetDlgItem(dlg, IDC_NAME_EDIT), EM_GETLINE, 0, (LPARAM)kSaveGameName);
        kSaveGameName[length] = 0;
        if (IsValidSaveName(kSaveGameName)) {
            EndDialog(dlg, 1);
            return 1;
        }
        MessageBoxA(dlg, "Please use only letters and Valid Filenames contain alphanumeric characters only", "Error", MB_ICONERROR);
        EndDialog(dlg, 0);
        return 1;
    }
    if (LOWORD(wParam) == IDCANCEL) {
        EndDialog(dlg, 0);
        return 0;
    }
    return 0;
}

int __cdecl IsValidSaveName(const char *name)
{
    if (strlen(name) == 0)
        return 0;
    for (size_t i = 0; i < strlen(name); i++)
        if (!isalnum((unsigned char)name[i]))
            return 0;
    return 1;
}

HANDLE __cdecl CreateSaveFile(HWND owner, const char *path)
{
    SECURITY_ATTRIBUTES security = { 12, NULL, FALSE };
    HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, &security, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file != INVALID_HANDLE_VALUE)
        return file;

    DWORD error = GetLastError();
    if (error != ERROR_FILE_EXISTS && error != ERROR_ALREADY_EXISTS) {
        MessageBoxA(owner, "Error Creating File!", "Error", MB_OK);
        return NULL;
    }
    if (MessageBoxA(owner, "File Already Exists! Overwrite?", "Error Creating File", MB_YESNO) == IDYES)
        file = CreateFileA(path, GENERIC_WRITE, 0, &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        MessageBoxA(owner, "Error Creating File!", "Error", MB_OK);
        return NULL;
    }
    return file;
}

}
