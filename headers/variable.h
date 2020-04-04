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

/* ��������� ������ ������� */
#define TEST_MODE

/* ��������� �������� ���� */
#define WM_PAINT_RELAX  WM_USER
#define WM_PAINT_DLTS   WM_USER+1
#define WM_REFRESH_MENU WM_USER+2

/* �������������� */
#define WM_NEW_CORTIME              200 //��������� win_weight_func ��� ���������� �������
#define ID_PROGRESS                 999 //������������� ������������

#define BUFF_SIZE                   256 //������ ���������� ������� ��� ������ �� �������� � ����� �

/* �������������� �������� */
#define DAQ_TIMER                   1
#define THERMOSTAT_TIMER            2

/* ��������� �������� */
#define REFRESH_TIME_DAQ            500 //��������� ����� ���������� ������������ DataAcquisition
#define REFRESH_TIME_THERMOSTAT     500 //��������� ����� ���������� ������������ LakeShore

/* ������� ��������� */
#define MAX_VOLTAGE_PULSE           (13.5)
#define MIN_VOLTAGE_PULSE           -MAX_VOLTAGE_PULSE
#define MAX_TEMPERATURE             320 //���������� ��������������� �������� SetPoint � EndPoint
#define MIN_TEMPERATURE             0   //���������� ��������������� �������� SetPoint � EndPoint
#define MAX_POINT_TEMP_GRAPH        100 //���������� ����� �� ������� �����������
#define AVERAGING_TIME              20  //�����, ����������� ��� ��������� �������� �������� T � ���������.
                                        //������ ���� ������, ��� MAX_POINT_TEMP_GRAPH*0.001*REFRESH_TIME_THERMOSTAT

/* ����� ������ ����� ��������� ������� ��� �������� ������ */
#define VOLTAGE_PRECISION           6
#define THERMO_PRECISION            2
#define TIME_PRECISION              4

/* ���������� ��������� */
#define BOLTZMANN                   (1.380649e-23)      //��/�
#define PLANCKS_CONSTANT_H          (6.62607015e-34)    //��*�
#define ELECTRON_MASS               (9.10938188e-31)    //[��], �������� �� ���������
#define PI                          3.1415926535897932384626433832795

enum mode{DLTS, ITS};

extern vector<double>               xAxisDLTS;
extern vector<vector<double>>       yAxisDLTS;
extern vector<double>               xAxisITS;
extern vector<vector<double>>       yAxisITS;
extern vector<double>               xAxisAr;
extern vector<vector<double>>       yAxisAr;
extern vector<double>               CorTc;

/* SavedRelaxations � SavedCapacity ���������� ������ � ��. ������ csSavedRelaxation */
extern vector<vector<double> >      SavedRelaxations; //������ ��� ����������� ��� ����������� ����������
//extern vector<double>               TimeAxis;         //��� �������
extern vector<double>               SavedCapacity;    //������ ��� ����������� ��� ����������� �������� �������
//extern atomic_size_t index_measurment;
extern atomic_size_t index_relax;   //����� ������� ���������� ��� �����������
extern atomic_size_t index_range;   //����� �������� ���������
extern atomic<mode>  index_mode;    //����� ������ ��������� DLTS ��� ITS
extern atomic_size_t index_w4;      //���������� DLTS ��� ��. ���������
extern atomic<int32> index_w2;      //�������� ������ ����� ai_port ��� ����������� � �������� �������

/* ��������� ��������� ���������� DAQ */
extern const string range[];
/* ����� ������������ */
extern const string names_wFunc[];
extern const int    int_range_sula[];
extern const int    int_pre_amplifier[];
/* �������� ������� ������� */
extern const string strHeatingRange[];
/* ��� ����� ��� ���������� FileSaveName.txt */
extern       string FileSaveName;
extern       string FileSavePath;
extern const string FileSaveExt;

extern double dEfMass;
extern double dFactorG;

extern double itsTemperature; /* ����������� � ������ ITS */
extern unsigned int id_DAQ;
extern int32 ai_port_measurement;
extern int32 ai_port_pulse;
extern int32 ai_port_capacity;
extern int32 pfi_ttl_port;
extern uInt32 measure_time_DAQ; //����� ���������
extern uInt32 averaging_DAQ;    //����� �������� ��� ��������� ������ ���������
extern float64 rate_DAQ;        //�������� � �������
extern float64 gate_DAQ;

/* ��������� SULA */
extern int RANGE_SULA_index;
extern int PRE_AMP_GAIN_SULA_index;
/* ��������� ������������� */
extern bool AprEnableRelax;
extern int  AprIter;
extern double AprErr;
/* ����� */
extern atomic_bool start;               //������ ������ �����
extern atomic_bool stability;           //����������� ����������������� ������ ���������
extern atomic_bool bfNewfile;           //��������� �� ����� ���� ��� ������ ������������?
extern atomic_bool fix_temp;            //�������� �����������
extern atomic_bool auto_peak_search;    //��������������� ���� ������� �������� �������
extern atomic_bool normaliz_dlts;
/* ��������� ����������� */
extern int WeightType;
extern double correlation_alpha;
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
extern CRITICAL_SECTION            csSavedData;
extern HANDLE                      hDownloadEvent;


#endif // VARIABLE_H_INCLUDED
