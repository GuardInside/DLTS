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

BOOL BuildMenu(HWND hWnd)
{
    HMENU hMenu = GetMenu(hWnd);
    HMENU hFileMenu = GetSubMenu(hMenu, 0);
    HMENU hAnalysisMenu = GetSubMenu(hMenu, 1);

    LONG metric = GetMenuCheckMarkDimensions();
    int height = HIWORD(metric);
    int width = LOWORD(metric);
    /* File */
    HBITMAP hbmpOpen = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(ID_BITMAP_OPEN), IMAGE_BITMAP, width, height, LR_SHARED);
    HBITMAP hbmpSave = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(ID_BITMAP_SAVE), IMAGE_BITMAP, width, height, LR_SHARED);
    HBITMAP hbmpNext = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(ID_BITMAP_NEXT), IMAGE_BITMAP, width, height, LR_SHARED);
    HBITMAP hbmpReply = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(ID_BITMAP_REPLY), IMAGE_BITMAP, width, height, LR_SHARED);
    HBITMAP hbmpExit = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(ID_BITMAP_EXIT), IMAGE_BITMAP, width, height, LR_SHARED);

    SetMenuItemBitmaps(hFileMenu, ID_MENU_OPEN, MF_BYCOMMAND, hbmpOpen, hbmpOpen);
    SetMenuItemBitmaps(hFileMenu, 1, MF_BYPOSITION, hbmpSave, hbmpSave);
        SetMenuItemBitmaps(hFileMenu, ID_MENU_SAVE_SETTINGS, MF_BYCOMMAND, hbmpNext, hbmpNext);
        SetMenuItemBitmaps(hFileMenu, ID_MENU_SAVE_RELAXATIONS, MF_BYCOMMAND, hbmpNext, hbmpNext);
        SetMenuItemBitmaps(hFileMenu, ID_MENU_SAVE_DLTS, MF_BYCOMMAND, hbmpNext, hbmpNext);
        SetMenuItemBitmaps(hFileMenu, ID_MENU_SAVE_ARRHENIUS, MF_BYCOMMAND, hbmpNext, hbmpNext);
        SetMenuItemBitmaps(hFileMenu, ID_MENU_SAVE_CT, MF_BYCOMMAND, hbmpNext, hbmpNext);
    SetMenuItemBitmaps(hFileMenu, ID_MENU_CLEAR_MEMORY, MF_BYCOMMAND, hbmpReply, hbmpReply);
    SetMenuItemBitmaps(hFileMenu, ID_BUTTON_EXIT, MF_BYCOMMAND, hbmpExit, hbmpExit);
    /* Analysis */
    HBITMAP hbmpFitting = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(ID_BITMAP_PLOT), IMAGE_BITMAP, width, height, LR_SHARED);
    HBITMAP hbmpExp = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(ID_BITMAP_PLOT_EXP), IMAGE_BITMAP, width, height, LR_SHARED);
    HBITMAP hbmpDLTS = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(ID_BITMAP_PLOT_DLTS), IMAGE_BITMAP, width, height, LR_SHARED);

    SetMenuItemBitmaps(hAnalysisMenu, 0, MF_BYPOSITION, hbmpFitting, hbmpFitting);
        SetMenuItemBitmaps(hAnalysisMenu, ID_MENU_FITTING_BSPLINE, MF_BYCOMMAND, hbmpExp, hbmpExp);
    SetMenuItemBitmaps(hAnalysisMenu, 1, MF_BYPOSITION, hbmpDLTS, hbmpDLTS);
    CheckMenuItem(hAnalysisMenu, ID_MENU_AUTO_PEAK_DETECTING, menu::automatic ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hAnalysisMenu, ID_MENU_DIVIDE_S_ON_TC, menu::divide ? MF_CHECKED : MF_UNCHECKED);
    return EXIT_SUCCESS;
}

stringstream& rewrite(stringstream& buff)
{
    buff.str("");
    return buff;
}

UINT GetDlgItemTextMod(HWND hwnd, int nIDItem, stringstream& str_buf)
{
    char buffer[BUFF_SIZE];
    UINT result = GetDlgItemText(hwnd, nIDItem, buffer, BUFF_SIZE);
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
