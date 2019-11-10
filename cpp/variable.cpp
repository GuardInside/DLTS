#include "variable.h"

vector<double>          xAxisDLTS;
vector<double>          *yAxisDLTS = nullptr;
vector<double>          xAxisAr;
vector<vector<double>>  yAxisAr;
vector<double>          CorTime;
vector<double>          itsUpVoltages;
vector<double>          itsLowVoltages;  /* ������ �������� ���������� ��������� */

vector<vector<double>>      SavedRelaxations;//������ ��� ����������� � ����������� ����������/
size_t index_relax = 0; //����� ������� ���������� ��� �����������
size_t index_range = 0; //����� �������� ���������
size_t index_mode  = DLTS; //����� ������ ��������� DLTS ��� ITS
size_t index_plot_DLTS = 0; //���������� DLTS ��� ��. ���������
int32  offset_ai_port = 0;//�������� ������ ����� ai_port ��� ����������� � �������� �������

const string        names_wFunc[] = {"Double box-car", "Lock-in", "Exponent", "Sine"};
const string        range[] = {"�10", "�5", "�0.5", "�0.05"};
const string        strHeatingRange[] = {"Off", "Low", "Med", "High"};
const string        SettingsFile  = "settings\\settings.txt";
const string        SettingsCorFile = "settings\\correlations.txt";
const string        SettingsPIDFile = "settings\\PID.txt";
const string        Save = "save\\";
      string        FileSaveName;

double itsTemperature = 0.0; /* ����������� � ������ ITS */
unsigned int aver_time = 15; //����� ��������������� ��������� �������� ��� ������� ������ ����������� dT � ��������
unsigned int id_DAQ = 0;
int32 ai_port = 0;
int32 ai_port_pulse = 0;
int32 pfi_ttl_port = 0;
uInt32 measure_time_DAQ = 0; //����� ��������� � ��
uInt32 averaging_DAQ = 0;    //����� �������� ��� ��������� ������ ���������
//uInt32 samples_DAQ = 0;      //������� ������� �� ���� ������
float64 rate_DAQ = 0.0;      //������� � �������
float64 gate_DAQ = 0.0;      //
/* ����� */
bool start = false;            //������ ������ �����
bool stability = false;        //����������� ����������������� ������ ���������
bool bfDAQ0k = true;          //DAQ �������� � ������� ������
bool fbThermostat0k = true;   //��������� �������� � ������� ������
bool bfNewfile = true;        //
/* ��������� ����������� */
unsigned int CorType = DoubleBoxCar;
double correlation_c = 0.0;
double correlation_width = 0.0;
double* correlation_t1 = nullptr;
/* ��������� �������� */
HINSTANCE                   hInst;      //��������� �������� ����������
HWND                        hMainWindow;
HWND                        hGraph;     //��������� ���� ������� �����������
HWND                        hRelax;     //��������� ���� ������� ����������
HWND                        hGraph_DAQ; //��������� ���� ������� ������� � DAQ
HWND                        hGraph_DLTS;//��������� ���� ������� DLTS
HWND                        hProgress;  //�������� ������ ������
CRITICAL_SECTION            csDataAcquisition;//������������ ������ ��� ������ ������ DAQ
HANDLE                      hDownloadEvent;
