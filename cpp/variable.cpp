#include <variable.h>

vector<double>      Temperature;
vector<double>      Relaxation;
vector<double>      SignalDAQ;
vector<double>      xAxisDLTS;
vector<double>      *yAxisDLTS = nullptr;
vector<double>      CorTime;
vector<double>      itsUpVoltages, itsLowVoltages;  /* Хранит значения напряжений импульсов */

vector<vector<double>>      SavedRelaxations;//Хранит все сохраненные и загруженные релаксации/
size_t index_relax = 0; //Номер текущей релаксации для отображения
size_t index_range = 0; //Номер текущего диапазона
size_t index_mode  = DLTS; //Режим работы программы DLTS или ITS
int32  offset_ai_port = 0;//Смещение номера порта ai_port при отображении в реальном времени

const string        names_wFunc[] = {"Double box-car", "Lock-in", "Exponent", "Sine"};
const string        range[] = {"±10", "±5", "±0.5", "±0.05"};
const string        strHeatingRange[] = {"Off", "Low", "Med", "High"};
const string        SettingsFile  = "settings\\settings.txt";
const string        SettingsCorFile = "settings\\correlations.txt";
const string        SettingsPIDFile = "settings\\PID.txt";
const string        Save = "save\\";
      string        FileSaveName;

double itsTemperature = 0.0; /* Температура в режиме ITS */
double CrossSection = 0.0;
double Energy = 0.0;
unsigned int aver_time = 15; //Число рассматриваемых временной интервал для расчета дрейфа температуры dT в секундах
unsigned int id_DAQ = 0;
int32 ai_port = 0;
int32 ai_port_pulse = 0;
int32 pfi_ttl_port = 0;
uInt32 measure_time_DAQ = 0; //Время измерения в мс
uInt32 averaging_DAQ = 0;    //Число отсчетов для получения одного измерения
uInt32 samples_DAQ = 0;      //Сколько сэмплов на один отсчет
float64 rate_DAQ = 0.0;      //Сэмплов в секунду
float64 gate_DAQ = 0.0;      //
/* Флаги */
bool start = false;            //Нажата кнопка старт
bool stability = false;        //Температура стабилизировалась вблизи сетпоинта
bool loading = false;          //Были загружены сохраненные релаксации
bool recalculate_dlts = false; //Пересчитывать ли DLTS график при перерисовки окна
/* Параметры коррелятора */
unsigned int CorType = DoubleBoxCar;
double correlation_c = 0.0;
double correlation_width = 0.0;
double* correlation_t1 = nullptr;
/* Описатели объектов */
HINSTANCE                   hInst;      //Описатель текущего приложения
HANDLE                      hThread_DAQ = nullptr; //Описатель потока DAQ
HWND                        hMainWindow;
HWND                        hGraph;     //Описатель окна графика температуры
HWND                        hRelax;     //Описатель окна графика релаксации
HWND                        hGraph_DAQ; //Описатель окна графика сигнала с DAQ
HWND                        hGraph_DLTS;//Описатель окна графика DLTS
HWND                        hProgress;//Прогресс чтения данных
HWND                        hGraph_Arrhenius;//Описатель окна графика Аррениуса
CRITICAL_SECTION            csDataAcquisition;//Используется всегда при чтении данных DAQ
CRITICAL_SECTION            csGlobalVariable;
