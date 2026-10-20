// SQZ.EXE 0x407d40: the "Keys and buttons" dialog procedure (template 106, opened from the menu). The list shows one
// line per input action with its key and button; Enter or a double click asks for a new key (PressKey dialog), Change
// asks which binding to reset (KeyKind dialog), and the left/right keys or the two small scroll bars move a binding
// between its two slots. OK saves the bindings when something changed, Cancel reverts them.
#include <windows.h>

#define IDC_KEYS_LIST       0x404
#define IDC_KEYS_KEY_BAR    0x40c
#define IDC_KEYS_BUTTON_BAR 0x42c
#define IDC_KEYS_CHANGE     0x40f

struct KeysInput {
    virtual int AssignKey(char *line, const char *keyName);   // slot 0
    virtual int SetKeySlot(char *line, int slot);             // slot 1
    void BindingLabel(char *line, char *keyName);             // 0x43ad00
    void Revert(int which);                                   // 0x43ad30
    int SetButtonSlot(char *line, int slot);                  // 0x43ae70
    int Save(HWND owner);                                     // 0x43c4d0
    int ActionName(int index, char *line);                    // 0x43c7b0
};
KeysInput *GetInput();                                        // 0x4206b0
extern HINSTANCE g_hInstance;                                 // 0x4c73c8
extern char g_keyName[];                                      // 0x4c6468 the PressKey dialog's answer
extern int g_bindingsChanged;                                 // 0x4b9514
int CALLBACK KeyKindDlgProc(HWND, UINT, WPARAM, LPARAM);      // 0x407c90
int CALLBACK PressKeyDlgProc(HWND, UINT, WPARAM, LPARAM);     // 0x407b90

int CALLBACK KeysDlgProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int slot = -1;              // the slot a left/right key or scroll arrow picks
    int buttonBar = 0;          // the scroll arrow belongs to the button bar
    char line[100];
    int n, i, selected, index;

    switch (msg) {
    case WM_INITDIALOG:
        for (n = SendMessage(GetDlgItem(dlg, IDC_KEYS_LIST), LB_GETCOUNT, 0, 0); n > 0; n--)
            SendMessage(GetDlgItem(dlg, IDC_KEYS_LIST), LB_DELETESTRING, 0, 0);
        for (i = 0; i < 51; i++)
            if (GetInput()->ActionName(i, line) > 0)
                SendMessage(GetDlgItem(dlg, IDC_KEYS_LIST), LB_ADDSTRING, 0, (LPARAM)line);
        SetFocus(GetDlgItem(dlg, IDC_KEYS_LIST));
        SendMessage(GetDlgItem(dlg, IDC_KEYS_LIST), LB_SETCURSEL, 0, 0);
        g_bindingsChanged = 0;
        SetScrollRange(GetDlgItem(dlg, IDC_KEYS_KEY_BAR), SB_CTL, 0, 1, FALSE);
        SetScrollRange(GetDlgItem(dlg, IDC_KEYS_BUTTON_BAR), SB_CTL, 0, 1, FALSE);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || HIWORD(wParam) == LBN_DBLCLK) {
            if (GetDlgItem(dlg, IDC_KEYS_LIST) == GetFocus()) {
                HWND list = GetDlgItem(dlg, IDC_KEYS_LIST);
                selected = SendMessage(list, LB_GETCURSEL, 0, 0);
                SendMessage(list, LB_GETTEXT, selected, (LPARAM)line);
                GetInput()->BindingLabel(line, g_keyName);
                if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x6f), dlg, (DLGPROC)PressKeyDlgProc, 0)) {
                    if (IsCharAlphaNumeric(g_keyName[0]) || g_keyName[0] == ' ') {
                        if (GetInput()->AssignKey(line, g_keyName)) {
                            SendMessage(list, LB_DELETESTRING, selected, 0);
                            index = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)line);
                            SendMessage(list, LB_SETCURSEL, index, 0);
                            g_bindingsChanged = 1;
                        } else {
                            MessageBox(dlg, "Sorry, this key is not changeable!", "Reassign Key Error", MB_OK);
                        }
                    }
                }
                return 0;
            }
            if (LOWORD(wParam) == IDOK) {
                if (g_bindingsChanged) {
                    if (GetInput()->Save(dlg))
                        EndDialog(dlg, 1);
                } else {
                    EndDialog(dlg, 1);
                }
            }
            return 0;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            GetInput()->Revert(1);
            EndDialog(dlg, 1);
            return 0;
        }
        if (LOWORD(wParam) == IDC_KEYS_CHANGE) {
            HWND list = GetDlgItem(dlg, IDC_KEYS_LIST);
            selected = SendMessage(list, LB_GETCURSEL, 0, 0);
            SendMessage(list, LB_GETTEXT, selected, (LPARAM)line);
            int answer = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x70), dlg, (DLGPROC)KeyKindDlgProc, 0);
            switch (answer) {
            case 1:
            case 2:
                if (answer == 1) {
                    if (!GetInput()->SetKeySlot(line, 2))
                        MessageBox(dlg, "Error changing key value!", "Reassign Key Error", MB_OK);
                    if (!GetInput()->SetButtonSlot(line, 2))
                        MessageBox(dlg, "Error changing button value!", "Reassign Key Error", MB_OK);
                } else {
                    GetInput()->SetKeySlot(line, 3);
                    GetInput()->SetButtonSlot(line, 3);
                }
                SendMessage(list, LB_DELETESTRING, selected, 0);
                index = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)line);
                SendMessage(list, LB_SETCURSEL, index, 0);
                break;
            case 3:
                GetInput()->Revert(1);
                SendMessage(dlg, WM_INITDIALOG, 0, 0);       // fill the list again
                break;
            case 4:
                GetInput()->Revert(2);
                SendMessage(dlg, WM_INITDIALOG, 0, 0);
                break;
            }
            g_bindingsChanged = 1;
            return 1;
        }
        return 1;

    case WM_VSCROLL:
        // the arrows of the two small scroll bars stand in for the left/right keys
        if (GetDlgItem(dlg, IDC_KEYS_KEY_BAR) == (HWND)lParam || GetDlgItem(dlg, IDC_KEYS_BUTTON_BAR) == (HWND)lParam) {
            switch (LOWORD(wParam)) {
            case SB_LINEUP:
                slot = 0;
                buttonBar = GetDlgItem(dlg, IDC_KEYS_KEY_BAR) != (HWND)lParam;
                SetScrollPos((HWND)lParam, SB_CTL, 1, TRUE);
                break;
            case SB_LINEDOWN:
                slot = 1;
                buttonBar = GetDlgItem(dlg, IDC_KEYS_KEY_BAR) != (HWND)lParam;
                SetScrollPos((HWND)lParam, SB_CTL, 0, TRUE);
                break;
            }
        }
        // fall through
    case WM_VKEYTOITEM:
        if (msg == WM_VKEYTOITEM) {
            switch (LOWORD(wParam)) {
            case VK_SPACE:
                break;
            case VK_LEFT:
                slot = 1;
                break;
            case VK_RIGHT:
                slot = 0;
                break;
            default:
                return -1;
            }
        }
        {
            HWND list = GetDlgItem(dlg, IDC_KEYS_LIST);
            if (slot == -1 && list != (HWND)lParam)
                return 0;
            selected = SendMessage(list, LB_GETCURSEL, 0, 0);
            SendMessage(list, LB_GETTEXT, selected, (LPARAM)line);
            int changed;
            if (buttonBar == 1)
                changed = GetInput()->SetButtonSlot(line, slot);
            else
                changed = GetInput()->SetKeySlot(line, slot);
            if (changed) {
                SendMessage(list, LB_DELETESTRING, selected, 0);
                index = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)line);
                SendMessage(list, LB_SETCURSEL, index, 0);
                g_bindingsChanged = 1;
            } else {
                MessageBox(dlg, "Sorry, this key is not changeable!", "Reassign Key Error", MB_OK);
            }
            return list == (HWND)lParam ? -2 : 0;
        }
    }
    return 0;
}
