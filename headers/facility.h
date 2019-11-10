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
#include "vi.h"
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

//Запись/Чтение настроек
void write_settings();
void read_settings();
//Применяет текущие настройки зон к физическому устройству
void SetZones();
//Инициализация сборщика данных, термостата и зон для PID регулировки
void ApplySettings();
/* Функции для сохранения и загрузки save-файлов */
VOID DownloadWindow();
UINT CALLBACK LoadFile(PVOID);
VOID SaveWindow();
UINT CALLBACK SaveFile(PVOID);
//Очистка векторов осей DLTS-кривых
void ClearMemmoryDLTS();
void ClearMemmory();
void OrderRelaxation();
//Обновление DLTS-кривых
void RefreshDLTS();
void SaveRelaxSignal(double MeanTemp, const vector<double> *vData, double dVoltMin, double dVoltMax);
void AddPointsDLTS(const vector<double> *vRelaxation, const double temp);
/* Возвращает расширение открытого файла */
string GetExtensionFile(string str);

#endif // FACILITY_H_INCLUDED
