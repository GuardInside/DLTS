#ifndef WINFUNC_H_INCLUDED
#define WINFUNC_H_INCLUDED
#include <string>
#include <sstream>
#include <windows.h>
#include <windowsx.h>
#include <algorithm>
#include <cmath>
#include "variable.h"
#include <facility.h>
#include "vi.h"
#include <daq.h>
#include "resource.h"
#include <graph.h>
#include <dlts_math.h>
#include "gwin.h"

/* *************** */
/* Оконные функции */
/* *************** */

LRESULT CALLBACK mwwin_proc(HWND, UINT, WPARAM, LPARAM); //Main dialog window
LRESULT CALLBACK stwin_proc(HWND, UINT, WPARAM, LPARAM); //General settings dialog
LRESULT CALLBACK dlwin_proc(HWND, UINT, WPARAM, LPARAM); //Start dialog window

BOOL    CALLBACK asdlg_proc(HWND, UINT, WPARAM, LPARAM); //Advanced Settings
BOOL    CALLBACK csdlg_proc(HWND, UINT, WPARAM, LPARAM); //Correlation Settings dialog window
BOOL    CALLBACK pidlg_proc(HWND, UINT, WPARAM, LPARAM); //PID settings dialog

ATOM CreateClass(HINSTANCE, std::string, WNDPROC);

/* ******************************************* */
/* Функции создания диалога в отдельном потоке */
/* ******************************************* */

UINT CALLBACK fMain_thCreateWindow(void*);
VOID thCreateWindow(HANDLE&, int);

VOID StartButPush(HWND hwnd);

#endif // WINFUNC_H_INCLUDED
