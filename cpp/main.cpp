#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <process.h>
#include "gwin.h"
#include "variable.h"
#include "resource.h"
#include "winfunc.h"
#include "vi.h"
#include "daq.h"
#include "facility.h"
#include "dlts_math.h"

using namespace std;
using namespace gwin;
BOOL MainWindow_OnCreate(HWND, LPCREATESTRUCT);
VOID MainWindow_OnCommand(HWND, int, HWND, UINT);
VOID MainWindow_OnTimer(HWND, UINT);

/* ���������� ������ � LakeShore � ���������� ���������� */
UINT CALLBACK Thermostat_Process(void*);
UINT CALLBACK DataAcquisition_Process(void*);
UINT CALLBACK ReadAverDAQ(void*);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
        hInst = hInstance;
        MSG messages;
        /* ������������ ������ ���� */
        CreateClass(hInst, "main window class", mwwin_proc);
        CreateClass(hInst, "main settings window class", stwin_proc);
        CreateClass(hInst, "dialog window class", dlwin_proc);
        /* ��������� ��� ��������� �� setting-����� */
        read_settings();
        /* �������������� �������-�����������             */
        /* � ��������� ��������� � ���������� ����������� */
        ApplySettings();
        /* �������������� ������� ���� */
        InitializeCriticalSection(&csDataAcquisition);
        InitializeCriticalSection(&csGlobalVariable);
        /* ���������� ������������ ���������� */
        InitCommonControls();
        /* ������� ���� � ��������� � ���� ��������� ��������� */
        hMainWindow = CreateDialog(hInstance, MAKEINTRESOURCE(ID_MAIN_WINDOW), 0, nullptr);

        while (GetMessage (&messages, NULL, 0, 0))
        {
            TranslateMessage(&messages);
            DispatchMessage(&messages);
        }
        return 0;
}

//������� ������� ��������� ����������� ����
LRESULT CALLBACK mwwin_proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hwnd, WM_CREATE, MainWindow_OnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND, MainWindow_OnCommand);
        HANDLE_MSG(hwnd, WM_TIMER, MainWindow_OnTimer);
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}

BOOL MainWindow_OnCreate(HWND hwnd, LPCREATESTRUCT)
{
    /* �������� ���� */
    hGraph = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph, 10, 22);
    gSize(hGraph, 399, 276);
    gPrecision(hGraph, 0, 2);
    gDefaultPlot(hGraph, "\0");
    hGraph_DAQ = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DAQ, 422, 22);
    gSize(hGraph_DAQ, 399, 276);
    gPrecision(hGraph_DAQ, 0, 3);
    gDefaultPlot(hGraph_DAQ, "\0");
    hRelax = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hRelax, 10, 323);
    gSize(hRelax, 399, 276);
    gPrecision(hRelax, 0, 3);
    gDefaultPlot(hRelax, "\0");
    hGraph_DLTS = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DLTS, 422, 323);
    gSize(hGraph_DLTS, 399, 276);
    gPrecision(hGraph_DLTS, 2, 3);
    gDefaultPlot(hGraph_DLTS, "\0");
    /* ��������� ���������� */
    hProgress = CreateWindow(PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD, 850, 195, 140, 20, hwnd, (HMENU)ID_PROGRESS, hInst, NULL);
        SendMessage(hProgress, PBM_SETRANGE, 0, 100 << 16);
        SendMessage(hProgress, PBM_SETPOS, 0, 0);
    /* ������� */
    SetTimer(hwnd, MAIN_TIMER, REFRESH_TIME, NULL);
    SetTimer(hwnd, DAQ_TIMER, REFRESH_TIME, NULL);
    SetTimer(hwnd, THERMOSTAT_TIMER, REFRESH_TIME, NULL);
    return TRUE;
}

void MainWindow_OnCommand(HWND hwnd, int id, HWND, UINT)
{
    static HANDLE thDownload = nullptr, /* ��������� ������ ��� �������� */
           thSave = nullptr,            /* ��������� ������ ��� ���������� */
           thSettings = nullptr,
           thStartWnd = nullptr;        /* ��������� ������ ��� ���� �������� */
    switch(id)
    {
        case ID_BUTTON_PREVIOUS_AI_PORT:
            if(offset_ai_port == 0 || !bfDAQ0k)
                break;
            offset_ai_port--;
            break;
        case ID_BUTTON_NEXT_AI_PORT:
            if(offset_ai_port == 1 || !bfDAQ0k)
                break;
            offset_ai_port++;
            break;
        case ID_BUTTON_PREVIOUS_DLTS:
            if(index_plot_DLTS == 0)
                break;
            index_plot_DLTS--;
            PlotDLTS();
            break;
        case ID_BUTTON_NEXT_DLTS:
            if(index_plot_DLTS == 1)
                break;
            index_plot_DLTS++;
            PlotDLTS();
            break;
        /* ������������ ������������ � ���� ���������� */
        case ID_BUTTON_PREVIOUS_RELAXATION:
            if(index_relax == 0 || SavedRelaxations.empty())
                break;
            index_relax--;
            PlotRelax();
            break;
        case ID_BUTTON_NEXT_RELAXATION:
            if(index_relax == SavedRelaxations.size()-1 || SavedRelaxations.empty())
                break;
            index_relax++;
            PlotRelax();
            break;
        /* ��������� ���� */
        case ID_BUTTON_SAVE:
        if(thSave == nullptr || WaitForSingleObject(thSave, 0) == WAIT_OBJECT_0)
                thSave = (HANDLE)_beginthreadex(nullptr, 0, SaveFile, 0, 0, nullptr);
            break;
        /* ��������� ���� */
        case ID_BUTTON_LOAD:
            if(thDownload == nullptr || WaitForSingleObject(thDownload, 0) == WAIT_OBJECT_0)
                thDownload = (HANDLE)_beginthreadex(nullptr, 0, DownloadFile, 0, 0, nullptr);
            break;
        /* ��������� */
        case ID_BUTTON_SETTINGS:
            thCreateWindow(thSettings, ID_SETTINGS_WINDOW);
            break;
        case ID_CHECKBOX_FIX_TEMPERATURE:
            {
                static bool fix_temp = false;
                stringstream buff;
                /* ����������� ���� �������� ����������� */
                if(fix_temp == false)
                {
                    double CurrentTemperature = 0.0;
                    Thermostat.Write("CDAT?");
                    Thermostat.ReadDigit(CurrentTemperature);
                    /* ��������� ��������� ��� ���������� ��� ������� ����������� */
                    rewrite(buff) << "SETP " << CurrentTemperature << "K";
                    Thermostat.Write(buff);
                    /* ��������� RANGE � ������������ � ������� ����� */
                    rewrite(buff) << "RANG " << Thermostat.ZoneTable.GetActuallyHeatRange();
                    Thermostat.Write(buff);
                }
                else
                {
                    /* ��������� ��������� � ������������ � ����������� */
                    rewrite(buff) << "SETP " << Thermostat.SetPoint << "K";
                    Thermostat.Write(buff);
                    /* ��������� RANGE � ������������ � ������� ����� */
                    rewrite(buff) << "RANG " << Thermostat.ZoneTable.GetActuallyHeatRange();
                    Thermostat.Write(buff);
                }
                fix_temp = !fix_temp;
            }
            break;
        /* �����/���� */
        case ID_BUTTON_START:
            if(start == false && !Thermostat.range_is_correct())
            {
                MessageBox(hwnd, "Invalid set point or end point value.", "Warning", MB_ICONWARNING);
                break;
            }
            if(start == false)
                thCreateWindow(thStartWnd, ID_START_WINDOW);
            if(start == true)
                StartButPush(hwnd);
            break;
        /* ��������� ���������� � ��������� �������� ����������� */
        case ID_BUTTON_SET:
            {
                double BeginPoint = 0.0, EndPoint = 0.0;
                /* ��������� ��������� �������� ����������� �� ���� ����� */
                if(EmptyEditBox(hwnd, ID_EDITCONTROL_BEGIN))
                    BeginPoint = Thermostat.SetPoint;
                else
                    BeginPoint = ApplySettingEditBox(hwnd, ID_EDITCONTROL_BEGIN);
                /* ��������� �������� �������� ����������� �� ���� ����� */
                if(EmptyEditBox(hwnd, ID_EDITCONTROL_END))
                    EndPoint = Thermostat.EndPoint;
                else
                    EndPoint = ApplySettingEditBox(hwnd, ID_EDITCONTROL_END);
                /* �������� ������������ ��������� �������� */
                if(EndPoint < MIN_TEMPERATURE)
                    MessageBox(hwnd, "End point should be more than 0.00 K.", "Warning", MB_ICONWARNING);
                else if(EndPoint > MAX_TEMPERATURE)
                    MessageBox(hwnd, "End point should be less than 320.00 K.", "Warning", MB_ICONWARNING);
                else if(BeginPoint < MIN_TEMPERATURE)
                    MessageBox(hwnd, "Begin point should be more than 0.00 K.", "Warning", MB_ICONWARNING);
                else if(BeginPoint > MAX_TEMPERATURE)
                    MessageBox(hwnd, "Begin point should be less than 320.00 K.", "Warning", MB_ICONWARNING);
                else
                {
                    stringstream buff;
                    buff << setprecision(2) << fixed;
                    /* � ������ ����������� ���� ��������� ���������� � ���� */
                    SetDlgItemText(hwnd, ID_EDITCONTROL_BEGIN, "");
                    rewrite(buff) << "Begin point " << BeginPoint << " K";
                    SetDlgItemText(hwnd, ID_STR_BEGIN, buff.str().data());

                    SetDlgItemText(hwnd, ID_EDITCONTROL_END, "");
                    rewrite(buff) << "End point " << EndPoint << " K";
                    SetDlgItemText(hwnd, ID_STR_END, buff.str().data());
                    /* ��������� �������� ���������� ���������� */
                    Thermostat.SetPoint = BeginPoint;
                    Thermostat.EndPoint = EndPoint;
                    break;
                }
                break;
            }
        /* ����� */
        case ID_BUTTON_EXIT:
            DestroyWindow(hwnd);
            break;
    }
}

VOID MainWindow_OnTimer(HWND hwnd, UINT id)
{
    static HANDLE thDataAcquisition, //��������� ������ ��� ������ � DAQ �� �������
            thThermostatAcquisition; //��������� ������ ��� ������ � ���������� �� �������
    switch(id)
    {
        case MAIN_TIMER:
            if(start)
            {
                stringstream buff;
                buff << setprecision(2) << fixed;
                double Dispersion = 0.0, Mean = 0.0;
                if((index_mode == DLTS && endofdlts == true) ||
                   (index_mode == ITS && endofits == true))
                StartButPush(hwnd);
                //��������� ������������ � ��������� ������ ����� ��� ���������� ������
                else if(stability == false)
                {
                    /* ������������ Mean-square error */
                    Dispersion = AverSqFluct(Temperature);
                    /* ��������� ������� ������������ ������ */
                    if(Dispersion <= Thermostat.TempDisp)
                    {
                        Mean = mean(Temperature);
                        /* ��������� ������� ������������ ������ ������ */
                        if(fabs(Mean - Thermostat.SetPoint) <= Thermostat.TempDisp)
                        {
                            stability = true;
                            HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, ReadAverDAQ, &Mean, 0, nullptr);
                            CloseHandle(hThread);
                        }
                    }
                }
            }
            break;
        case DAQ_TIMER:
            if(bfDAQ0k)
            {
                if(thDataAcquisition == nullptr || WaitForSingleObject(thDataAcquisition, 0) == WAIT_OBJECT_0)
                {
                    CloseHandle(thDataAcquisition);
                    thDataAcquisition = (HANDLE)_beginthreadex(nullptr, 0, DataAcquisition_Process, 0, 0, nullptr);
                }
            }
            else gDefaultPlot(hGraph_DAQ, "The NIDAQ isn't being initialized.\nYou should change advanced settings.");
            break;
        case THERMOSTAT_TIMER:
            #ifndef TEST_MODE
            if(fbThermostat0k)
            {
                #endif // TEST_MODE
                if(thThermostatAcquisition == nullptr || WaitForSingleObject(thThermostatAcquisition, 0) == WAIT_OBJECT_0)
                {
                    CloseHandle(thThermostatAcquisition);
                    thThermostatAcquisition = (HANDLE)_beginthreadex(nullptr, 0, Thermostat_Process, 0, 0, nullptr);
                }
            #ifndef TEST_MODE
            }
            else gDefaultPlot(hGraph_DAQ, "The thermostat LakeShore isn't being initialized.\nYou should change advanced settings.");
                #endif // TEST_MODE
            break;
    }
}

/* ��������� ������ � ����������� ����� */
UINT CALLBACK DataAcquisition_Process(void*)
{

    if(ai_port+offset_ai_port == ai_port_pulse)
    {
        double dMinVoltage = 0.0, dMaxVoltage = 0.0;
        MeasurePulse(&SignalDAQ, &dMinVoltage, &dMaxVoltage);
        stringstream buff;
        buff << setprecision(3) << "[" << dMinVoltage << "," << dMaxVoltage << "]";
        gAdditionalInfo(hGraph_DAQ, buff.str());
    }
    else
    {
        MyDAQMeasure(&SignalDAQ, 1, measure_time_DAQ*0.001, ai_port);
        gAdditionalInfo(hGraph_DAQ, "\0");
    }
    plotDAQ();
    return 0;
}

/* ���������� ������ � ���������� � ���������� ���������� */
UINT CALLBACK Thermostat_Process(void*)
{
    double CurrentTemperature = 0.0, HeatPercent = 0.0, SetPoint = 0.0;
    stringstream buff;
    buff << setprecision(2) << fixed;
    /* �������� �������� ������� ����������� */
    Thermostat.Write("CDAT?");
    Thermostat.ReadDigit(CurrentTemperature);
    #ifdef TEST_MODE
        srand(time(NULL));
        CurrentTemperature = rand()%50+10;
    #endif
    /* ��������� ���������� �������� */
    Temperature.push_back(CurrentTemperature);
    /* ������� ������ �������� */
    if(Temperature.size() > MAX_POINT_TEMP_GRAPH)
        Temperature.erase(Temperature.begin());
    /* �������� ���� � ��������� */
    Thermostat.Write("HEAT?");
    Thermostat.ReadDigit(HeatPercent);
    rewrite(buff) << "Power " << HeatPercent << "%";
    SetDlgItemText(hMainWindow, ID_STR_POWER, buff.str().data());
    /* ������� ����������� */
    rewrite(buff) << "Temperature " << CurrentTemperature << " K ";
    SetDlgItemText(hMainWindow, ID_STR_CHANNEL_A, buff.str().data());
    /* ������������ � ������� Mean-square error */
    rewrite(buff) << "Mean-squared error " << AverSqFluct(Temperature) << " K";
    SetDlgItemText(hMainWindow, ID_STR_DISPERSION, buff.str().data());
    /* ������� ������� �������� SetPoint */
    Thermostat.Write("SETP?");
    Thermostat.ReadDigit(SetPoint);
    rewrite(buff) << "Set point " << SetPoint << " K";
    SetDlgItemText(hMainWindow, ID_STR_SETPOINT, buff.str().data());
    /* ��������� ����������� ������� */
    gVector vData1;
    for(size_t i = 0; i < Temperature.size(); i++)
        vData1.push_back(i*REFRESH_TIME*0.001);
    gBand(hGraph, 0, 50, (CurrentTemperature-5)<0?0:(CurrentTemperature-5), (CurrentTemperature+5));
    gData(hGraph, &vData1, &Temperature);
    return 0;
}

UINT CALLBACK ReadAverDAQ(void* mean_temperature)
{
    if(index_mode == DLTS && !Thermostat.range_is_correct())
    {
        endofdlts = true;
        return 0;
    }
    double MeanTemp = *((double*)mean_temperature);
    double dVoltMin = 0.0, dVoltMax = 0.0;
    double dBeginVolt = Generator.begin_voltage, dEndVolt = Generator.end_voltage;
    /* � ������ ITS ������, ���� ���� ����������� */
    do
    {
        MyDAQMeasure(&Relaxation, averaging_DAQ, measure_time_DAQ*0.001, ai_port, TRUE);
        //��������� ����������, ����� ����� ����������� ������������� ����� ����
        SavedRelaxations.push_back(Relaxation);
        index_relax = SavedRelaxations.size() - 1; //��� �������, ������ ��� ������
        if(index_mode == DLTS)
        {
            AddPointsDLTS(MeanTemp);                    //��������� ����� �� DLTS
            prepare_next_set_point();
        }
        else if(index_mode == ITS)
        {
            /* ��������� �������� ��������� ������������ �������� */
            MeasurePulse(NULL, &dVoltMin, &dVoltMax);
            if(Generator.begin_voltage + Generator.step_voltage > Generator.end_voltage)
            {
                Generator.begin_voltage = dBeginVolt;
                Generator.end_voltage = dEndVolt;
                Generator.Apply();
                endofits = true;
                stability = false;
                break; // ����� �� ����� while
            }
            Generator.begin_voltage += Generator.step_voltage;
            Generator.Apply();
            stringstream buff;
            buff << setprecision(3) << "[" << dVoltMin << "," << dVoltMax << "]";
            gAdditionalInfo(hRelax, buff.str());
        }
        SaveRelaxSignal(MeanTemp, Relaxation, dVoltMin, dVoltMax);//��������� ��������� � ����
        PlotRelax();
        PlotDLTS();
    }while(start == true && stability == true && endofits == false);
    stability = false;
    return 0;
}
