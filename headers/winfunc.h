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
#include <dlts_math.h>
#include "gwin.h"

/* *************** */
/* ������� ������� */
/* *************** */

LRESULT CALLBACK mwwin_proc(HWND, UINT, WPARAM, LPARAM); //Main dialog window

INT_PTR CALLBACK stwin_proc(HWND, UINT, WPARAM, LPARAM); //������� ������� ���� ��������
INT_PTR CALLBACK anwin_proc(HWND, UINT, WPARAM, LPARAM); //������� ������� ���� �������

BOOL    CALLBACK srwin_proc(HWND, UINT, WPARAM, LPARAM); //������� ������� ���� ������� ������
BOOL    CALLBACK asdlg_proc(HWND, UINT, WPARAM, LPARAM); //Advanced Settings
BOOL    CALLBACK csdlg_proc(HWND, UINT, WPARAM, LPARAM); //Correlation Settings dialog window
BOOL    CALLBACK pidlg_proc(HWND, UINT, WPARAM, LPARAM); //PID settings dialog

ATOM CreateClass(HINSTANCE, std::string, WNDPROC);

/* **************************** */
/* ������� ����������� �������� */
/* **************************** */

extern CRITICAL_SECTION csDAQPaint;

VOID plotDAQ(gwin::gVector *vData1, gwin::gVector *vData2);
VOID PlotRelax(); // �������� ������ ����� SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_RELAX, 0);
VOID PlotDLTS(const gwin::gVector &xAxis, const gwin::gMulVector &yAxis);  // �������� ������ ����� SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);

/* ************************************** */
/* ����������� ������� �������� ��������� */
/* ************************************** */

BOOL dlts_mouse_message(HWND, UINT, WPARAM, LPARAM); //������� ������� ��� ��������� ��������� ���� ���� ������� DLTS
BOOL relax_mouse_message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL daq_mouse_message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/* ************************************ */
/* ������� ����������� � ��������� ���� */
/* ************************************ */
BOOL BuildMenu(HWND hWnd);


UINT CALLBACK dlg_success(PVOID);

VOID StartButPush();

#endif // WINFUNC_H_INCLUDED
