#ifndef FACILITY_H_INCLUDED
#define FACILITY_H_INCLUDED
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include <windows.h>
#include <commctrl.h>
#include <iomanip>
#include <commdlg.h>
#include <GPIB.h>
#include "daq.h"
#include <dlts_math.h>
#include "resource.h"
#include "gwin.h"
#include "variable.h"
using namespace std;

stringstream&   rewrite(stringstream& buff);
UINT            GetDlgItemTextMod(HWND hwnd, int nIDItem, stringstream& str_buf);
double          ApplySettingEditBox(HWND hwnd, int nIDEditBox, int prec = 0);
string          ApplySettingEditBoxString(HWND hwnd, int nIDEditBox);
bool            EmptyEditBox(HWND hwnd, int nIDEditBox);

//������/������ ��������
void write_settings();
void read_settings();
//��������� ������� ��������� ��� � ����������� ����������
void SetZones();
//������������� �������� ������, ���������� � ��� ��� PID �����������
void ApplySettings();
//������ ������ ������
void StartButPush(HWND hwnd);
//������� �������� ���� DLTS-������
void ClearAxisDLTS();
//��������� Save-���� � ������������ ��������� ����������, ������ DLTS-������ � ������ ���������
UINT CALLBACK DownloadFile(void*);
UINT CALLBACK SaveFile(void*);
//���������� DLTS-������
void RefreshDLTS();
void SaveRelaxSignal(double MeanTemp, const vector<double>);
//����������� SetPoint �� �������� StepTemp � ������� ���� ������������
void prepare_next_set_point();
/* ���������� ���������� ��������� ����� */
string GetExtensionFile(char*, int);

#endif // FACILITY_H_INCLUDED