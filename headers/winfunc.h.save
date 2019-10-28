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
#include <GPIB.h>
#include <daq.h>
#include <resource.h>
#include <graph.h>
#include <dlts_math.h>
#include "gwin.h"

BOOL RedrawWindow_Relax(const gwin::gVector, const gwin::gVector);

LRESULT CALLBACK mwwin_proc(HWND, UINT, WPARAM, LPARAM); //Main dialog window
LRESULT CALLBACK stwin_proc(HWND, UINT, WPARAM, LPARAM); //General settings dialog

LRESULT CALLBACK arwin_proc(HWND, UINT, WPARAM, LPARAM); //Arrhenius dialog window
BOOL    CALLBACK asdlg_proc(HWND, UINT, WPARAM, LPARAM); //Advanced Settings

BOOL    CALLBACK csdlg_proc(HWND, UINT, WPARAM, LPARAM); //Correlation Settings dialog window
BOOL    CALLBACK pidlg_proc(HWND, UINT, WPARAM, LPARAM); //PID settings dialog

ATOM CreateClass(HINSTANCE, std::string, WNDPROC);

/* Создает поток и окно для отображения сигнала на образце */
UINT CALLBACK SignalOnDiode(void*);
/* Точка входа в поток, создающий окно */
UINT CALLBACK fMain_thCreateWindow(void*);
/* Запуск одного экземпляра потока работы с окном */
void thCreateWindow(HANDLE& thArrhenius, int);

#endif // WINFUNC_H_INCLUDED
