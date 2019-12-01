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

/* ��������� ������ ������� */
#define TEST_MODE

/* �������������� */
#define WM_NEW_CORTIME              200 //��������� win_weight_func ��� ���������� �������
#define ID_PROGRESS                 999 //������������� ������������
#define DLTS                        0
#define ITS                         1

#define BUFF_SIZE                   256 //������ ���������� ������� ��� ������ �� �������� � ����� �

/* �������������� �������� */
#define DAQ_TIMER                   1
#define THERMOSTAT_TIMER            2

/* ��������� �������� */
#define REFRESH_TIME_DAQ            500 //��������� ����� ���������� ������������ DataAcquisition
#define REFRESH_TIME_THERMOSTAT     500 //��������� ����� ���������� ������������ LakeShore

/* ������� ��������� */
#define MAX_VOLTAGE_PULSE           5
#define MIN_VOLTAGE_PULSE           -MAX_VOLTAGE_PULSE
#define MAX_TEMPERATURE             320 //���������� ��������������� �������� SetPoint � EndPoint
#define MIN_TEMPERATURE             0   //���������� ��������������� �������� SetPoint � EndPoint
#define MAX_POINT_TEMP_GRAPH        100 //���������� ����� �� ������� �����������
#define RANGE_SIZE                  4   //����� ���������� ���������� ��� ���
#define RANGE_HEATING_SIZE          4   //����� ������� ������� ���'�
#define VOLTAGE_PRECISION           4   //�����
#define THERMO_PRECISION            2   //�������
#define TIME_PRECISION              4   //�������
#define AVERAGING_TIME              15  //�����, ����������� ��� ��������� �������� �������� T � ���������.
                                        //������ ���� ������, ��� MAX_POINT_TEMP_GRAPH*0.001*REFRESH_TIME_THERMOSTAT

/* ���������� ��������� */
#define BOLTZMANN                   (1.380649e-23)      //��/�
#define PLANCKS_CONSTANT_H          (6.62607015e-34)    //��*�
#define G_CONSTANT                  1                   //������ ���������� g = 2*s + 1 [Arb.]. �� ���������.
#define MASS_ELECTRON               (9.10938188e-31)    //[��] �� ���������.
#define PI                          3.1415926535897932384626433832795

enum CORTYPE{DoubleBoxCar, LockIn, ExpW, SinW};

extern vector<double>           xAxisDLTS;
extern vector<double>           *yAxisDLTS;
extern vector<double>           xAxisAr;
extern vector<vector<double>>   yAxisAr;
extern vector<double>           CorTime;
extern vector<double>           itsAmpVoltages;
extern vector<double>           itsBiasVoltages;  /* ������ �������� ���������� ��������� � ������ ITS */

extern vector<vector<double>> SavedRelaxations; //������ ��� ����������� ��� ����������� ����������
extern size_t index_relax;  //����� ������� ���������� ��� �����������
extern size_t index_range;  //����� �������� ���������
extern size_t index_mode;   //����� ������ ��������� DLTS ��� ITS
extern size_t index_plot_DLTS; //���������� DLTS ��� ��. ���������
extern int32 offset_ai_port;//�������� ������ ����� ai_port ��� ����������� � �������� �������

/* ��������� ��������� ���������� DAQ */
extern const string range[];
/* �������� ������� ������� */
extern const string strHeatingRange[];
/* ��� ����� ��� ���������� FileSaveName.txt */
extern       string FileSaveName;
extern       string FileSavePath;
/* ����� ������� ������� */
extern const string names_wFunc[];

extern double dEfMass;
extern double dFactorG;

extern double itsTemperature; /* ����������� � ������ ITS */
extern unsigned int id_DAQ;
extern int32 ai_port;
extern int32 ai_port_pulse;
extern int32 pfi_ttl_port;
extern uInt32 measure_time_DAQ; //����� ���������
extern uInt32 averaging_DAQ;    //����� �������� ��� ��������� ������ ���������
extern float64 rate_DAQ;        //�������� � �������
extern float64 gate_DAQ;

extern int RANGE_SULA;          //Capaciti range
extern double CONST_02_SULA;
extern double PRE_AMP_GAIN_SULA;
/* ����� */
extern bool start;              //������ ������ �����
extern bool stability;          //����������� ����������������� ������ ���������
extern bool bfDAQ0k;            //DAQ �������� � ������� ������
extern bool fbThermostat0k;     //��������� �������� � ������� ������
extern bool bfNewfile;          //��������� �� ����� ���� ��� ������ ������������?
extern bool fix_temp;           //�������� �����������
extern bool auto_peak_search;   //��������������� ���� ������� �������� �������
/* ��������� ����������� */
extern unsigned int CorType;
extern double correlation_c;
extern double correlation_width;
extern double* correlation_t1;
/* ��������� �������� */
extern HINSTANCE                   hInst;      //��������� �������� ����������
extern HWND                        hMainWindow;
extern HWND                        hGraph;     //��������� ���� ������� �����������
extern HWND                        hGraph_DAQ; //��������� ���� ������� ������� � ������� � �������� �������
extern HWND                        hRelax;     //��������� ���� ������� ����������
extern HWND                        hGraph_DLTS;//��������� ���� ������� DLTS
extern HWND                        hProgress;  //�������� ������ ������
extern CRITICAL_SECTION            csDataAcquisition;//������������ ������ ��� ������ ������ DAQ
extern HANDLE                      hDownloadEvent;
#endif // VARIABLE_H_INCLUDED
