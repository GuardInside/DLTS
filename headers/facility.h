#ifndef FACILITY_H_INCLUDED
#define FACILITY_H_INCLUDED
#include <string>
#include <sstream>
#include <cmath>
#include <windows.h>
#include <commctrl.h>
#include <iomanip>
#include <commdlg.h>
#include "vi.h"
#include "daq.h"
#include <dlts_math.h>
#include "resource.h"
#include "gwin.h"
#include "variable.h"
using namespace std;

enum SAVE_MODE {SAVE_RELAXATIONS, SAVE_DLTS, SAVE_ARRHENIUS};

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
/* ������� ��� ���������� � �������� save-������ */
VOID DownloadWindow();
UINT CALLBACK LoadFile(PVOID);
VOID SaveWindow(SAVE_MODE);
UINT CALLBACK SaveFile(PVOID);
//������� �������� ���� DLTS-������
void ClearMemmoryDLTS();
void ClearMemmory();
void OrderRelaxation();
//���������� DLTS-������
void RefreshDLTS();
void SaveRelaxSignal(double MeanTemp, const vector<double> *vData, double dBias, double dAmp);
void AddPointsDLTS(const vector<double> *vRelaxation, const double temp);
/* ���������� ���������� ��������� ����� */
string GetExtensionFile(string str);
/* ����������� �������� ���������� � ������� */
void VoltageToCapacity(double *value);

#endif // FACILITY_H_INCLUDED
