#include <variable.h>

vector<double>      Temperature;
vector<double>      Relaxation;
vector<double>      SignalDAQ;
vector<double>      xAxisDLTS;
vector<double>      *yAxisDLTS = nullptr;
vector<double>      CorTime;
vector<double>      itsUpVoltages, itsLowVoltages;  /* ������ �������� ���������� ��������� */

vector<vector<double>>      SavedRelaxations;//������ ��� ����������� � ����������� ����������/
size_t index_relax = 0; //����� ������� ���������� ��� �����������
size_t index_range = 0; //����� �������� ���������
size_t index_mode  = DLTS; //����� ������ ��������� DLTS ��� ITS
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
double CrossSection = 0.0;
double Energy = 0.0;
unsigned int aver_time = 15; //����� ��������������� ��������� �������� ��� ������� ������ ����������� dT � ��������
unsigned int id_DAQ = 0;
int32 ai_port = 0;
int32 ai_port_pulse = 0;
int32 pfi_ttl_port = 0;
uInt32 measure_time_DAQ = 0; //����� ��������� � ��
uInt32 averaging_DAQ = 0;    //����� �������� ��� ��������� ������ ���������
uInt32 samples_DAQ = 0;      //������� ������� �� ���� ������
float64 rate_DAQ = 0.0;      //������� � �������
float64 gate_DAQ = 0.0;      //
/* ����� */
bool start = false;            //������ ������ �����
bool stability = false;        //����������� ����������������� ������ ���������
bool loading = false;          //���� ��������� ����������� ����������
bool recalculate_dlts = false; //������������� �� DLTS ������ ��� ����������� ����
/* ��������� ����������� */
unsigned int CorType = DoubleBoxCar;
double correlation_c = 0.0;
double correlation_width = 0.0;
double* correlation_t1 = nullptr;
/* ��������� �������� */
HINSTANCE                   hInst;      //��������� �������� ����������
HANDLE                      hThread_DAQ = nullptr; //��������� ������ DAQ
HWND                        hMainWindow;
HWND                        hGraph;     //��������� ���� ������� �����������
HWND                        hRelax;     //��������� ���� ������� ����������
HWND                        hGraph_DAQ; //��������� ���� ������� ������� � DAQ
HWND                        hGraph_DLTS;//��������� ���� ������� DLTS
HWND                        hProgress;//�������� ������ ������
HWND                        hGraph_Arrhenius;//��������� ���� ������� ���������
CRITICAL_SECTION            csDataAcquisition;//������������ ������ ��� ������ ������ DAQ
CRITICAL_SECTION            csGlobalVariable;
