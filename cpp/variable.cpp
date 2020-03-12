#include "variable.h"

vector<double>          xAxisDLTS;
vector<double>          *yAxisDLTS = nullptr;
vector<double>          xAxisAr;
vector<vector<double>>  yAxisAr;
vector<double>          CorTime;
vector<double>          itsAmpVoltages;
vector<double>          itsBiasVoltages;  /* Хранит значения напряжений импульсов */

vector<vector<double>>  SavedRelaxations;//Хранит все сохраненные и загруженные релаксации
vector<double>          SavedCapacity;    //Хранит все сохраненные или загруженные значения емкости
atomic_size_t index_relax{0}; //Номер текущей релаксации для отображения
atomic_size_t index_range{0}; //Номер текущего диапазона
atomic<mode>  index_mode{DLTS}; //Режим работы программы DLTS или ITS
atomic_size_t index_w4{0}; //Отображать DLTS или гр. Аррениуса
atomic<int32> index_w2{0};//Смещение номера порта ai_port при отображении в реальном времени

const string        names_wFunc[] = {"Double box-car", "Lock-in", "Exponent", "Sine"};
const string        range[] = {"±10", "±5", "±0.5", "±0.05"};
const int           int_range_sula[] = {10, 30, 100, 300, 1000};
const int           int_pre_amplifier[] = {1, 3, 10, 30, 100};
const string        strHeatingRange[] = {"Off", "Low", "Med", "High"};
      string        FileSaveName;
      string        FileSavePath;

double dEfMass = ELECTRON_MASS;
double dFactorG = 1;

double itsTemperature = 0.0; /* Температура в режиме ITS */
unsigned int id_DAQ = 0;
int32 ai_port = 0;
int32 ai_port_pulse = 0;
int32 ai_port_capacity = 0;
int32 pfi_ttl_port = 0;
uInt32 measure_time_DAQ = 0; //Время измерения в мс
uInt32 averaging_DAQ = 0;    //Число отсчетов для получения одного измерения
float64 rate_DAQ = 0.0;      //Сэмплов в секунду
float64 gate_DAQ = 0.0;      //Длительность запирающего импульса
/* Настройки SULA */
int RANGE_SULA_index = 0;
int PRE_AMP_GAIN_SULA_index = 0;
/* Настройки аппроксимации */
bool AprEnableRelax = false;
bool AprEnableDLTS = false;
int  AprIter = 50;
double AprErr = 1e-6;
/* Флаги */
atomic_bool start{false};            //Нажата кнопка старт
atomic_bool stability{false};        //Температура стабилизировалась вблизи сетпоинта
atomic_bool bfNewfile{true};         //Создавать ли новый файл при старте эксперимента?
atomic_bool fix_temp{false};         //Фиксация температуры
atomic_bool auto_peak_search{true};  //Автоопределение пика методом золотого сечения
atomic_bool normaliz_dlts{false};
/* Параметры коррелятора */
unsigned int CorType = DoubleBoxCar;
double correlation_c = 0.0;
double correlation_width = 0.0; /* В мкс */
double* correlation_t1 = nullptr;
/* Описатели объектов */
HINSTANCE                   hInst;      //Описатель текущего приложения
HWND                        hMainWindow;
HWND                        hGraph;     //Описатель окна графика температуры
HWND                        hRelax;     //Описатель окна графика релаксации
HWND                        hGraph_DAQ; //Описатель окна графика сигнала с DAQ
HWND                        hGraph_DLTS;//Описатель окна графика DLTS
HWND                        hProgress;  //Прогресс чтения данных
CRITICAL_SECTION            csSavedRelaxation;
HANDLE                      hDownloadEvent;
