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

INT_PTR CALLBACK stwin_proc(HWND, UINT, WPARAM, LPARAM); //Оконная функция окна настроек
INT_PTR CALLBACK anwin_proc(HWND, UINT, WPARAM, LPARAM);

BOOL    CALLBACK srwin_proc(HWND, UINT, WPARAM, LPARAM); //Оконная функция окна запроса старта
BOOL    CALLBACK asdlg_proc(HWND, UINT, WPARAM, LPARAM); //Advanced Settings
BOOL    CALLBACK csdlg_proc(HWND, UINT, WPARAM, LPARAM); //Correlation Settings dialog window
BOOL    CALLBACK pidlg_proc(HWND, UINT, WPARAM, LPARAM); //PID settings dialog

ATOM CreateClass(HINSTANCE, std::string, WNDPROC);

/* **************************** */
/* Функции отображения графиков */
/* **************************** */

VOID plotDAQ(gwin::gVector *vData1, gwin::gVector *vData2);
VOID PlotRelax();
VOID PlotDLTS();

/* ************************************** */
/* Специальные Функции обрабоки сообщений */
/* ************************************** */

BOOL dlts_mouse_message(HWND, UINT, WPARAM, LPARAM); //Оконная функция для обработки сообщений мыщи окна графика DLTS

/* **************************** */
UINT CALLBACK dlg_success(PVOID);

VOID StartButPush();

#endif // WINFUNC_H_INCLUDED
