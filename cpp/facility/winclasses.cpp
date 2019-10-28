#include <winfunc.h>

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
