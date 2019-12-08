#include "winfunc.h"

#include <windows.h>
#include <string>

#define ID_ITEM_FILE        WM_USER
#define ID_ITEM_ANALYSIS    WM_USER+1

BOOL BuildMenu(HWND hWnd)
{
    HMENU hMenu = GetMenu(hWnd);
    /* File */
    HBITMAP hbmpOpen = LoadBitmap(hInst, MAKEINTRESOURCE(ID_BITMAP_OPEN));
    SetMenuItemBitmaps(hMenu, ID_MENU_OPEN, MF_BYCOMMAND, hbmpOpen, hbmpOpen);
    return EXIT_SUCCESS;
}

BOOL DrawMenu(HWND hWnd, const DRAWITEMSTRUCT* DrawSt)
{
    return EXIT_SUCCESS;
}
