#include "variable.h"

vector<double>          xAxisDLTS;
vector<double>          *yAxisDLTS = nullptr;
vector<double>          xAxisAr;
vector<vector<double>>  yAxisAr;
vector<double>          CorTime;
vector<double>          itsAmpVoltages;
vector<double>          itsBiasVoltages;  /* ������ �������� ���������� ��������� */

vector<vector<double>>  SavedRelaxations;//������ ��� ����������� � ����������� ����������
vector<double>          SavedCapacity;    //������ ��� ����������� ��� ����������� �������� �������
atomic_size_t index_relax{0}; //����� ������� ���������� ��� �����������
atomic_size_t index_range{0}; //����� �������� ���������
atomic<mode>  index_mode{DLTS}; //����� ������ ��������� DLTS ��� ITS
atomic_size_t index_w4{0}; //���������� DLTS ��� ��. ���������
atomic<int32> index_w2{0};//�������� ������ ����� ai_port ��� ����������� � �������� �������

const string        names_wFunc[] = {"Double box-car", "Lock-in", "Exponent", "Sine"};
const string        range[] = {"�10", "�5", "�0.5", "�0.05"};
const int           int_range_sula[] = {10, 30, 100, 300, 1000};
const int           int_pre_amplifier[] = {1, 3, 10, 30, 100};
const string        strHeatingRange[] = {"Off", "Low", "Med", "High"};
      string        FileSaveName;
      string        FileSavePath;

double dEfMass = ELECTRON_MASS;
double dFactorG = 1;

double itsTemperature = 0.0; /* ����������� � ������ ITS */
unsigned int id_DAQ = 0;
int32 ai_port = 0;
int32 ai_port_pulse = 0;
int32 ai_port_capacity = 0;
int32 pfi_ttl_port = 0;
uInt32 measure_time_DAQ = 0; //����� ��������� � ��
uInt32 averaging_DAQ = 0;    //����� �������� ��� ��������� ������ ���������
float64 rate_DAQ = 0.0;      //������� � �������
float64 gate_DAQ = 0.0;      //������������ ����������� ��������
/* ��������� SULA */
int RANGE_SULA_index = 0;
int PRE_AMP_GAIN_SULA_index = 0;
/* ��������� ������������� */
bool AprEnableRelax = false;
bool AprEnableDLTS = false;
int  AprIter = 50;
double AprErr = 1e-6;
/* ����� */
atomic_bool start{false};            //������ ������ �����
atomic_bool stability{false};        //����������� ����������������� ������ ���������
atomic_bool bfNewfile{true};         //��������� �� ����� ���� ��� ������ ������������?
atomic_bool fix_temp{false};         //�������� �����������
atomic_bool auto_peak_search{true};  //��������������� ���� ������� �������� �������
atomic_bool normaliz_dlts{false};
/* ��������� ����������� */
unsigned int CorType = DoubleBoxCar;
double correlation_c = 0.0;
double correlation_width = 0.0; /* � ��� */
double* correlation_t1 = nullptr;
/* ��������� �������� */
HINSTANCE                   hInst;      //��������� �������� ����������
HWND                        hMainWindow;
HWND                        hGraph;     //��������� ���� ������� �����������
HWND                        hRelax;     //��������� ���� ������� ����������
HWND                        hGraph_DAQ; //��������� ���� ������� ������� � DAQ
HWND                        hGraph_DLTS;//��������� ���� ������� DLTS
HWND                        hProgress;  //�������� ������ ������
CRITICAL_SECTION            csSavedRelaxation;
HANDLE                      hDownloadEvent;
