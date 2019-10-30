#ifndef VARIABLE_H_INCLUDED
#define VARIABLE_H_INCLUDED
#include <string>
#include <vector>
#include <sstream>
#include <windows.h>
#include <process.h>
#include <NIDAQmx.h>
#include "gwin.h"
#include "vi.h"
using namespace std;

/* Активация режима отладки */
#define TEST_MODE

/* Идентификаторы */
#define WM_NEW_CORTIME              200 //Сообщение win_weight_func для обновления графика
#define ID_PROGRESS                 999 //Идентификатор прогрессбара
#define DLTS                        0
#define ITS                         1

#define BUFF_SIZE                   256 //Размер строкового буффера для работы со строками в стиле С

/* Идентификаторы таймеров */
#define MAIN_TIMER                  0
#define DAQ_TIMER                   1
#define THERMOSTAT_TIMER            2


/* Настройки таймеров */
#define REFRESH_TIME                500 //Период главного таймера. Если опрашиваем оба канала > 500 ms
#define REFRESH_TIME_DAQ            500 //Интервала между повторными считываниями DataAcquisition
#define REFRESH_TIME_THERMOSTAT     500 //Интервала между повторными считываниями LakeShore

/* Мировые константы */
#define MAX_VOLTAGE_PULSE           5
#define MIN_VOLTAGE_PULSE           -MAX_VOLTAGE_PULSE
#define MAX_TEMPERATURE             320 //Наибольшее устанавливаемое значение SetPoint и EndPoint
#define MIN_TEMPERATURE             0   //Наименьшее устанавливаемое значение SetPoint и EndPoint
#define MAX_POINT_TEMP_GRAPH        101 //Количество точек на графике температуры > TEMPERATURE_POINTS
#define MAX_POINT_RELAX_GRAPH       100 //Количество точек на графике релаксации
#define RANGE_SIZE                  4   //Число диапазонов напряжений для АЦП
#define RANGE_HEATING_SIZE          4   //Число уровней нагрева ТЭН'а

/* Физические константы */
#define BOLTZMANN                   (8.6173303e-5)      //эВ/К
#define PLANCKS_CONSTANT_H          (4.135667662e-15)   //эВ*с
#define G_CONSTANT                  2                   //Фактор вырождения g = 2*s + 1
#define MASS_ELECTRON               (9.10938188e-28)    //г
#define PI                          3.1415926535897932384626433832795

enum CORTYPE{DoubleBoxCar, LockIn, ExpW, SinW};

extern vector<double> Temperature;
extern vector<double> Relaxation;
extern vector<double> SignalDAQ;
extern vector<double> xAxisDLTS;
extern vector<double> *yAxisDLTS;
extern vector<double> CorTime;
extern vector<double> itsUpVoltages, itsLowVoltages;  /* Хранит значения напряжений импульсов в режиме ITS */

extern vector<vector<double>> SavedRelaxations; //Хранит все сохраненные или загруженные релаксации
extern size_t index_relax;  //Номер текущей релаксации для отображения
extern size_t index_range;  //Номер текущего диапазона
extern size_t index_mode;   //Режим работы программы DLTS или ITS
extern size_t index_plot_DLTS; //Отображать DLTS или гр. Аррениуса
extern int32 offset_ai_port;//Смещение номера порта ai_port при отображении в реальном времени

/* Имена файлов с соответствующими настройками */
extern const string SettingsFile;
extern const string SettingsCorFile;
extern const string SettingsPIDFile;
/* Имя папки с сохраненными релаксациями */
extern const string Save;
/* Диапазоны измерений напряжения DAQ */
extern const string range[];
/* Диапазон уровней нагрева */
extern const string strHeatingRange[];
/* Имя файла для сохранения FileSaveName.txt */
extern       string FileSaveName;
/* Имена весовых функций */
extern const string names_wFunc[];

extern double itsTemperature; /* Температура в режиме ITS */
extern unsigned int aver_time;   //Число рассматриваемых точек для расчета дрейфа температуры dT
extern unsigned int id_DAQ;
extern int32 ai_port;
extern int32 ai_port_pulse;
extern int32 pfi_ttl_port;
//extern uInt32 samples_DAQ;      //сколько сэмплов на один отсчет
extern uInt32 measure_time_DAQ; //Время измерения
extern uInt32 averaging_DAQ;    //Число отсчетов для получения одного измерения
extern float64 rate_DAQ;        //отсчетов в секунду
extern float64 gate_DAQ;
/* Флаги */
extern bool start;            //Нажата кнопка старт
extern bool stability;        //Температура стабилизировалась вблизи сетпоинта
extern bool loading;          //Были загружены сохраненные релаксации
extern bool endofits;
extern bool endofdlts;
extern bool bfDAQ0k;          //DAQ работает в штатном режиме
extern bool fbThermostat0k;   //Термостат работает в штатном режиме
extern bool bfNewfile;        //
extern bool bfAppfile;
/* Параметры коррелятора */
extern unsigned int CorType;
extern double correlation_c;
extern double correlation_width;
extern double* correlation_t1;
/* Описатели объектов */
extern HINSTANCE                   hInst;      //Описатель текущего приложения
extern HWND                        hMainWindow;
extern HWND                        hGraph;     //Описатель окна графика температуры
extern HWND                        hRelax;     //Описатель окна графика релаксации
extern HWND                        hGraph_DAQ; //Описатель окна графика сигнала с образца в реальном времени
extern HWND                        hGraph_DLTS;//Описатель окна графика DLTS
extern HWND                        hProgress;  //Прогресс чтения данных
extern CRITICAL_SECTION            csDataAcquisition;//Используется всегда при чтении данных DAQ
extern CRITICAL_SECTION            csGlobalVariable;
#endif // VARIABLE_H_INCLUDED
