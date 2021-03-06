#include "variable.h"
#include "dlts_math.h"

vector<double>              xAxisDLTS;
vector<vector<double>>      yAxisDLTS;
vector<double>              xAxisITS;
vector<vector<double>>      yAxisITS;
vector<double>              xAxisAr;
vector<double>              yAxisAr;
vector<double>              yAxisArMSQ;
vector<double>              CorTc;
vector<double>              TimeAxis;


vector<vector<double>>              SavedRelaxations;   // ������ ��� ����������� � ����������� ����������
//vector<double>                      TimeAxis;
vector<double>                      SavedCapacity;      // ������ ��� ����������� ��� ����������� �������� �������
//atomic_size_t index_measurment{0};
atomic_size_t index_relax{0}; //����� ������� ���������� ��� �����������
atomic_size_t index_range{0}; //����� �������� ���������
atomic<mode>  index_mode{DLTS}; //����� ������ ��������� DLTS ��� ITS
atomic_size_t index_w4{0}; //���������� DLTS ��� ��. ���������
atomic<int32> index_w2{0};//�������� ������ ����� ai_port ��� ����������� � �������� �������

const string        range[] = {"�10", "�5", "�0.5", "�0.05"};
const int           int_range_sula[] = {10, 30, 100, 300, 1000};
const int           int_pre_amplifier[] = {1, 3, 10, 30, 100};
const string        strHeatingRange[] = {"Off", "Low", "Med", "High"};
      string        FileSaveName;
      string        FileSavePath;
const string        FileSaveExt{".dlts"};

double dEfMass = ELECTRON_MASS;
double dFactorG = 1;
double dImpurity = 1e15;

double itsTemperature = 0.0; /* ����������� � ������ ITS */
unsigned int id_DAQ = 0;
int32 ai_port_measurement = 0;
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
bool    bspline::enable{false};
bool    bspline::enable2{false};
size_t  bspline::order{4};
size_t  bspline::ncoeffs{12};
/* ����� */
atomic_bool start{false};            //������ ������ �����
atomic_bool stability{false};        //����������� ����������������� ������ ���������
atomic_bool bfNewfile{true};         //��������� �� ����� ���� ��� ������ ������������?
atomic_bool fix_temp{false};         //�������� �����������
/* ���� */
bool menu::divide{false};
bool menu::automatic{true};
//atomic_bool normaliz_dlts{false};

/* ��������� ����������� */
int WeightType = DoubleBoxCar;
double correlation_alpha = 0.1;
double correlation_c = 1.0;
double correlation_width = 1.0; /* � ��� */
double* correlation_t1 = nullptr;
/* ��������� �������� */
HINSTANCE                   hInst;      //��������� �������� ����������
HWND                        hMainWindow;
HWND                        hGraph;     //��������� ���� ������� �����������
HWND                        hRelax;     //��������� ���� ������� ����������
HWND                        hGraph_DAQ; //��������� ���� ������� ������� � DAQ
HWND                        hGraph_DLTS;//��������� ���� ������� DLTS
HWND                        hProgress;  //�������� ������ ������
CRITICAL_SECTION            csSavedData;
HANDLE                      hDownloadEvent;
