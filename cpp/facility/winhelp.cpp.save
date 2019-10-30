#include "facility.h"
#include "winfunc.h"

ATOM CreateClass(HINSTANCE hInst, string strName, WNDPROC proc)
{
    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.cbWndExtra = DLGWINDOWEXTRA;
    wcex.lpfnWndProc = proc;
    wcex.hInstance = hInst;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszClassName = strName.data();
    return RegisterClassEx(&wcex);
}

/* Точка входа в поток, создающий окно */
UINT CALLBACK fMain_thCreateWindow(void* DLG_ID)
{
    MSG messages;
    CreateDialog(hInst, MAKEINTRESOURCE(*(int*)DLG_ID), 0, 0);
    while(GetMessage(&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
    return 0;
}

void thCreateWindow(HANDLE& thMain, int DLG_ID)
{
    DWORD status = 0;
    if(thMain != nullptr)
        GetExitCodeThread(thMain, &status);
    if(thMain == nullptr || status != STILL_ACTIVE)
        thMain = (HANDLE)_beginthreadex(nullptr, 0, fMain_thCreateWindow, &DLG_ID, 0, nullptr);
}


stringstream& rewrite(stringstream& buff)
{
    buff.str("");
    return buff;
}

UINT GetDlgItemTextMod(HWND hwnd, int nIDItem, stringstream& str_buf)
{
    char buffer[30];
    UINT result = GetDlgItemText(hwnd, nIDItem, buffer, 30);
    rewrite(str_buf) << buffer;
    return result;
}

double ApplySettingEditBox(HWND hwnd, int nIDEditBox, int prec)
{
    double result = 0.0;
    stringstream buff;
    buff << setprecision(prec) << fixed;
    GetDlgItemTextMod(hwnd, nIDEditBox, buff);
    result = atof(buff.str().data());
    rewrite(buff) << result;
    SetDlgItemText(hwnd, nIDEditBox, buff.str().data());
    return result;
}

string ApplySettingEditBoxString(HWND hwnd, int nIDEditBox)
{
    string result;
    stringstream buff;
    GetDlgItemTextMod(hwnd, nIDEditBox, buff);
    result = buff.str();
    SetDlgItemText(hwnd, nIDEditBox, result.data());
    return result;
}

bool EmptyEditBox(HWND hwnd, int nIDEditBox)
{
    stringstream buff;
    GetDlgItemTextMod(hwnd, nIDEditBox, buff);
    if(buff.str().empty())
        return true;
    return false;
}
