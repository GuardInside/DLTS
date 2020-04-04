#ifndef VARIABLE_H_INCLUDED
#define VARIABLE_H_INCLUDED
#include <atomic>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <windows.h>
#include <process.h>
#include "gwin.h"
#include "vi.h"
#include "daqmx.h"
using namespace std;
using namespace NIDAQmx::innards;

/* Активация режима отладки */
#define TEST_MODE

/* Сообщения главному окну */
#define WM_PAINT_RELAX  WM_USER
#define WM_PAINT_DLTS   WM_USER+1
#define WM_REFRESH_MENU WM_USER+2

/* Идентификаторы */
#define WM_NEW_CORTIME              200 //Сообщение win_weight_func для обновления графика
#define ID_PROGRESS                 999 //Идентификатор прогрессбара

#define BUFF_SIZE                   256 //Размер строкового буффера для работы со строками в стиле С

/* Идентификаторы таймеров */
#define DAQ_TIMER                   1
#define THERMOSTAT_TIMER            2

/* Настройки таймеров */
#define REFRESH_TIME_DAQ            500 //Интервала между повторными считываниями DataAcquisition
#define REFRESH_TIME_THERMOSTAT     500 //Интервала между повторными считываниями LakeShore

/* Мировые константы */
#define MAX_VOLTAGE_PULSE           (13.5)
#define MIN_VOLTAGE_PULSE           -MAX_VOLTAGE_PULSE
#define MAX_TEMPERATURE             320 //Наибольшее устанавливаемое значение SetPoint и EndPoint
#define MIN_TEMPERATURE             0   //Наименьшее устанавливаемое значение SetPoint и EndPoint
#define MAX_POINT_TEMP_GRAPH        100 //Количество точек на графике температуры
#define AVERAGING_TIME              20  //Время, необходимое для получения среднего значения T и дисперсии.
                                        //Должно быть меньше, чем MAX_POINT_TEMP_GRAPH*0.001*REFRESH_TIME_THERMOSTAT

/* Число знаков после десятично запятой для хранения данных */
#define VOLTAGE_PRECISION           6
#define THERMO_PRECISION            2
#define TIME_PRECISION              4

/* Физические константы */
#define BOLTZMANN                   (1.380649e-23)      //Дж/К
#define PLANCKS_CONSTANT_H          (6.62607015e-34)    //Дж*с
#define ELECTRON_MASS               (9.10938188e-31)    //[кг], значение по умолчанию
#define PI                          3.1415926535897932384626433832795

enum mode{DLTS, ITS};

extern vector<double>               xAxisDLTS;
extern vector<vector<double>>       yAxisDLTS;
extern vector<double>               xAxisITS;
extern vector<vector<double>>       yAxisITS;
extern vector<double>               xAxisAr;
extern vector<vector<double>>       yAxisAr;
extern vector<double>               CorTc;

/* SavedRelaxations и SavedCapacity вызываются только в кр. секции csSavedRelaxation */
extern vector<vector<double> >      SavedRelaxations; //Хранит все сохраненные или загруженные релаксации
//extern vector<double>               TimeAxis;         //Ось времени
extern vector<double>               SavedCapacity;    //Хранит все сохраненные или загруженные значения емкости
//extern atomic_size_t index_measurment;
extern atomic_size_t index_relax;   //Номер текущей релаксации для отображения
extern atomic_size_t index_range;   //Номер текущего диапазона
extern atomic<mode>  index_mode;    //Режим работы программы DLTS или ITS
extern atomic_size_t index_w4;      //Отображать DLTS или гр. Аррениуса
extern atomic<int32> index_w2;      //Смещение номера порта ai_port при отображении в реальном времени

/* Диапазоны измерений напряжения DAQ */
extern const string range[];
/* Имена корреляторов */
extern const string names_wFunc[];
extern const int    int_range_sula[];
extern const int    int_pre_amplifier[];
/* Диапазон уровней нагрева */
extern const string strHeatingRange[];
/* Имя файла для сохранения FileSaveName.txt */
extern       string FileSaveName;
extern       string FileSavePath;
extern const string FileSaveExt;

extern double dEfMass;
extern double dFactorG;

extern double itsTemperature; /* Температура в режиме ITS */
extern unsigned int id_DAQ;
extern int32 ai_port_measurement;
extern int32 ai_port_pulse;
extern int32 ai_port_capacity;
extern int32 pfi_ttl_port;
extern uInt32 measure_time_DAQ; //Время измерения
extern uInt32 averaging_DAQ;    //Число отсчетов для получения одного измерения
extern float64 rate_DAQ;        //отсчетов в секунду
extern float64 gate_DAQ;

/* Настройки SULA */
extern int RANGE_SULA_index;
extern int PRE_AMP_GAIN_SULA_index;
/* Настройки аппроксимации */
extern bool AprEnableRelax;
extern int  AprIter;
extern double AprErr;
/* Флаги */
extern atomic_bool start;               //Нажата кнопка старт
extern atomic_bool stability;           //Температура стабилизировалась вблизи сетпоинта
extern atomic_bool bfNewfile;           //Создавать ли новый файл при старте эксперимента?
extern atomic_bool fix_temp;            //Фиксация температуры
extern atomic_bool auto_peak_search;    //Автоопределение пика методом золотого сечения
extern atomic_bool normaliz_dlts;
/* Параметры коррелятора */
extern int WeightType;
extern double correlation_alpha;
extern double correlation_c;
extern double correlation_width;
extern double* correlation_t1;
/* Описатели объектов */
extern HINSTANCE                   hInst;      //Описатель текущего приложения
extern HWND                        hMainWindow;
extern HWND                        hGraph;     //Описатель окна графика температуры
extern HWND                        hGraph_DAQ; //Описатель окна графика сигнала с образца в реальном времени
extern HWND                        hRelax;     //Описатель окна графика релаксации
extern HWND                        hGraph_DLTS;//Описатель окна графика DLTS
extern HWND                        hProgress;  //Прогресс чтения данных
extern CRITICAL_SECTION            csSavedData;
extern HANDLE                      hDownloadEvent;


#endif // VARIABLE_H_INCLUDED
