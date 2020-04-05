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

enum SAVE_MODE {SAVE_SETTINGS, SAVE_RELAXATIONS, SAVE_DLTS, SAVE_ARRHENIUS, SAVE_CT};

stringstream&   rewrite(stringstream& buff);
UINT            GetDlgItemTextMod(HWND hwnd, int nIDItem, stringstream& str_buf);
double          ApplySettingEditBox(HWND hwnd, int nIDEditBox, int prec = 0);
string          ApplySettingEditBoxString(HWND hwnd, int nIDEditBox);
bool            EmptyEditBox(HWND hwnd, int nIDEditBox);
int             GetResPidTable(int, string const&);

double MeanSquareErrorOfTemp(std::vector<double>::const_iterator b,
                             std::vector<double>::const_iterator e);
double MeanOfTemp(std::vector<double>::const_iterator b,
                  std::vector<double>::const_iterator e);

//������/������ ��������
void write_settings();
void read_settings();
//������������� �������� ������, ���������� � ��� ��� PID �����������
void ApplySettings();
void InitializeVirtualInstruments();
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
void SaveRelaxSignalToFile(double MeanTemp, const vector<double> *vData, double dBias, double dAmp, double capacity);
void AddPointsDLTS(const vector<double> *vRelaxation, const double temp, const double capacity);
/* ���������� ���������� ��������� ����� */
string GetExtensionFile(string str);
/* ����������� �������� ���������� � ������� */
void VoltageToCapacity(double *value);
/* ���������� � ������������ */
corinfo get_corinfo(int type, double Tc);

#endif // FACILITY_H_INCLUDED
