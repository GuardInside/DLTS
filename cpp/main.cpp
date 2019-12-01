#define WINVER 0x0500

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <process.h>
#include <exception>

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
UINT Thermostat_Process();
UINT CALLBACK DataAcquisition_Process(void*);
UINT CALLBACK ReadITS(void*);
UINT CALLBACK ReadDLTS(void*);

HWND hSettinWnd = NULL;         /* ��������� ���� �������� */
HWND hAnalysisWnd = NULL;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
        hInst = hInstance;
        MSG messages;
        /* ������������ ������ ���� */
        CreateClass(hInst, "main window class", mwwin_proc);
        /* ��������� ��� ��������� �� setting-����� */
        read_settings();
        /* �������������� �������-�����������             */
        /* � ��������� ��������� � ���������� ����������� */
        ApplySettings();
        /* �������������� ������� ���� */
        InitializeCriticalSection(&csDataAcquisition);
        hDownloadEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
        /* ���������� ������������ ���������� */
        InitCommonControls();
        /* ������� ���� � ��������� � ���� ��������� ��������� */
        hMainWindow = CreateDialog(hInstance, MAKEINTRESOURCE(ID_MAIN_WINDOW), HWND_DESKTOP, NULL);

        while (GetMessage(&messages, NULL, 0, 0))
        {
            if(!IsDialogMessage(hSettinWnd, &messages) && !IsDialogMessage(hAnalysisWnd, &messages))
            {
                /* �������� ��������� � ��������� ����� ��� ���� DLTS ������� */
                if(index_plot_DLTS == 0 && messages.hwnd == hGraph_DLTS)
                {
                    dlts_mouse_message(messages.hwnd, messages.message, messages.wParam, messages.lParam);
                    /* ��������� �� ������� ��������� ����������� ������������ */
                }
                TranslateMessage(&messages);
                DispatchMessage(&messages);
            }
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
    /* ��������� ���� */
    MENUINFO hMenuInfo {0};
	hMenuInfo.cbSize = sizeof(hMenuInfo);
	hMenuInfo.fMask = MIM_APPLYTOSUBMENUS | MIM_STYLE;
	hMenuInfo.dwStyle = MNS_AUTODISMISS | MNS_NOCHECK;
    HMENU hMenu = GetMenu(hwnd);
    SetMenuInfo(hMenu, &hMenuInfo);
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
    SetTimer(hwnd, DAQ_TIMER, REFRESH_TIME_DAQ, NULL);
    SetTimer(hwnd, THERMOSTAT_TIMER, REFRESH_TIME_THERMOSTAT, NULL);
    return TRUE;
}

void MainWindow_OnCommand(HWND hwnd, int id, HWND, UINT)
{
    switch(id)
    {
        case ID_BUTTON_PREVIOUS_AI_PORT:
            if(offset_ai_port == 0 || !bfDAQ0k)
                break;
            offset_ai_port--;
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_NEXT_AI_PORT:
            if(offset_ai_port == 1 || !bfDAQ0k)
                break;
            offset_ai_port++;
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_PREVIOUS_DLTS:
            if(index_plot_DLTS == 0)
                break;
            index_plot_DLTS--;
            PlotDLTS();
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_NEXT_DLTS:
            if(index_plot_DLTS == 1)
                break;
            index_plot_DLTS++;
            PlotDLTS();
            SetFocus(hMainWindow);
            break;
        /* ������������ ������������ � ���� ���������� */
        case ID_BUTTON_PREVIOUS_RELAXATION:
            if(index_relax == 0 || SavedRelaxations.empty())
                break;
            index_relax--;
            PlotRelax();
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_NEXT_RELAXATION:
            if(index_relax == SavedRelaxations.size()-1 || SavedRelaxations.empty())
                break;
            index_relax++;
            PlotRelax();
            SetFocus(hMainWindow);
            break;
        /* ��������� ���� */
        case ID_MENU_SAVE_RELAXATIONS:
            SaveWindow(SAVE_RELAXATIONS);
            break;
        case ID_MENU_SAVE_DLTS:
            SaveWindow(SAVE_DLTS);
            break;
        case ID_MENU_SAVE_ARRHENIUS:
            SaveWindow(SAVE_ARRHENIUS);
            break;
        case ID_MENU_CLEAR_MEMORY:
            ClearMemmory();
            break;
        case ID_MENU_AUTO_PEAK_DETECTING:
            ::auto_peak_search = true;
            CloseHandle((HANDLE)_beginthreadex(NULL, 0, dlg_success, NULL, 0, NULL));
            PlotDLTS();
            break;
        /* ��������� ���� */
        case ID_MENU_OPEN:
            DownloadWindow();
            break;
        /* ��������� */
        case ID_MENU_FITTING_EXPONENTIAL:
            if(!IsWindow(hAnalysisWnd))
                hAnalysisWnd = CreateDialog(hInst, MAKEINTRESOURCE(ID_ANALYSIS_WINDOW), HWND_DESKTOP, anwin_proc);
            break;
        case ID_BUTTON_SETTINGS:
            if(!IsWindow(hSettinWnd))
                hSettinWnd = CreateDialog(hInst, MAKEINTRESOURCE(ID_SETTINGS_WINDOW), HWND_DESKTOP, stwin_proc);
            break;
        case ID_CHECKBOX_FIX_TEMPERATURE:
            {
                static double dPrevTemp = 0.0;
                stringstream buff;
                /* ����������� ���� �������� ����������� */
                if(fix_temp == false)
                {
                    Thermostat.Write("SETP?");
                    Thermostat.ReadDigit(dPrevTemp);
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
                    rewrite(buff) << "SETP " << dPrevTemp << "K";
                    Thermostat.Write(buff);
                    /* ��������� RANGE � ������������ � ������� ����� */
                    if(start == true)
                        rewrite(buff) << "RANG " << Thermostat.ZoneTable.GetActuallyHeatRange();
                    else rewrite(buff) << "RANG 0";
                    Thermostat.Write(buff);
                }
                (fix_temp == true)? fix_temp = false : fix_temp = true;
            }
            break;
        /* �����/���� */
        case ID_BUTTON_START:
            if(start == false && !Thermostat.range_is_correct())
            {
                MessageBox(hwnd, "Invalid set point or end point value,\nor step temperature sign.", "Warning", MB_ICONWARNING);
                break;
            }
            if(start == false)
                DialogBox(hInst,  MAKEINTRESOURCE(ID_START_WINDOW), hwnd, srwin_proc);
            else if(start == true)
                StartButPush();
            break;
        /* ��������� ���������� � ��������� �������� ����������� */
        case ID_BUTTON_SET:
            {
                double BeginPoint = 0.0, EndPoint = 0.0;
                /* ��������� ��������� �������� ����������� �� ���� ����� */
                if(EmptyEditBox(hwnd, ID_EDITCONTROL_BEGIN))
                    BeginPoint = Thermostat.BeginPoint;
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
                    Thermostat.BeginPoint = BeginPoint;
                    Thermostat.EndPoint = EndPoint;
                }
            }
            break;
        /* ����� */
        case ID_BUTTON_EXIT:
                Thermostat.Write("RANG 0");
                DestroyWindow(hwnd);
            break;
    }
}


VOID MainWindow_OnTimer(HWND hwnd, UINT id)
{
    static HANDLE thDataAcquisition = NULL; /* ��������� ������ ��� ������ � DAQ �� ������� */
    switch(id)
    {
        case DAQ_TIMER:
            if(bfDAQ0k)
            {
                DWORD ExitCode = 0;
                if(thDataAcquisition == NULL || (GetExitCodeThread(thDataAcquisition, &ExitCode) && ExitCode != STILL_ACTIVE))
                {
                    if(thDataAcquisition)
                        CloseHandle(thDataAcquisition);
                    thDataAcquisition = (HANDLE)_beginthreadex(NULL, 0, DataAcquisition_Process, NULL, 0, NULL);
                }
            }
            else gDefaultPlot(hGraph_DAQ, "The NIDAQ isn't being initialized.\nYou should change advanced settings.");
            break;
        case THERMOSTAT_TIMER:
            #ifndef TEST_MODE
            if(fbThermostat0k)
            {
            #endif // TEST_MODE
            /* ��������� ����� ����� ������������ */
            Thermostat_Process();
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
    gwin::gVector vData1, vData2;
    stringstream buff;
    if(ai_port+offset_ai_port == ai_port_pulse)
    {
        double dBias = 0.0, dAmp = 0.0;
        MeasurePulse(&vData2, &dBias, &dAmp);
        buff << fixed << "AI: " << ai_port + offset_ai_port << endl
             << setprecision(3) << "Bias [V] =  " << dBias << endl << "Amp [V] = " << dAmp;
        gAdditionalInfo(hGraph_DAQ, buff.str());
    }
    else
    {
        double capacity = 0.0;
        int port_c = 2; /* AI_2 */
        DAQMeasure_Capacity(port_c, &capacity);
        capacity *= CONST_02_SULA*RANGE_SULA;
        MyDAQMeasure(&vData2, 1, measure_time_DAQ*0.001, ai_port);
        for(auto &Y: vData2)
        {
            Y = CONST_02_SULA*Y*RANGE_SULA / PRE_AMP_GAIN_SULA;
        }
        buff << fixed << "AI: " << ai_port + offset_ai_port << endl
             << "Cap. [pF] = " << fixed << setprecision(3) << capacity;
        gAdditionalInfo(hGraph_DAQ, buff.str());
    }
    for(size_t i = 0; i < vData2.size(); i++)
        vData1.push_back(0.001*gate_DAQ + i*(1.0/rate_DAQ)*1000.0);
    plotDAQ(&vData1, &vData2);
    return 0;
}

/* ���������� ������ � ���������� � ���������� ���������� */
UINT Thermostat_Process()
{
    static vector<double> vTemp;
    double dTemp = 0.0, HeatPercent = 0.0, SetPoint = 0.0;
    stringstream buff;
    buff << setprecision(2) << fixed;
    /* �������� �������� ������� ����������� */
    Thermostat.Write("CDAT?");
    Thermostat.ReadDigit(dTemp);
    dTemp = round(dTemp, 2);
    #ifdef TEST_MODE
        srand(time(NULL));
        dTemp = rand()%50+10;
        dTemp = round(dTemp,2);
    #endif
    /* ��������� ���������� �������� */
    vTemp.push_back(dTemp);
    /* ������� ������ �������� */
    if(vTemp.size() > MAX_POINT_TEMP_GRAPH)
        vTemp.erase(vTemp.begin());
    /* �������� ���� � ��������� */
    Thermostat.Write("HEAT?");
    Thermostat.ReadDigit(HeatPercent);
    rewrite(buff) << "Power " << HeatPercent << "%";
    SetDlgItemText(hMainWindow, ID_STR_POWER, buff.str().data());
    /* ������� ����������� */
    rewrite(buff) << "Temperature " << dTemp << " K ";
    SetDlgItemText(hMainWindow, ID_STR_CHANNEL_A, buff.str().data());
    /* ������� ������� �������� SetPoint */
    Thermostat.Write("SETP?");
    Thermostat.ReadDigit(SetPoint);
    rewrite(buff) << "Set point " << SetPoint << " K";
    SetDlgItemText(hMainWindow, ID_STR_SETPOINT, buff.str().data());
    /* ������������ � ������� Mean-square error */
    double dAverSqFluct = AverSqFluct(vTemp);
    rewrite(buff) << "Mean-squared error " << dAverSqFluct << " K";
    SetDlgItemText(hMainWindow, ID_STR_DISPERSION, buff.str().data());
    /* ��������� ������� ������������ ������ */
    if(start == true && stability == false && fix_temp == false && dAverSqFluct <= Thermostat.TempDisp)
    {
        double dMean = round(mean(vTemp), THERMO_PRECISION);
        /* ��������� ������� ������������ ������ */
        if(fabs(dMean - Thermostat.BeginPoint) <= Thermostat.TempDisp) //dAverSqFluct?
        {
            stability = true;
            double* MeanTemp = new double(dMean);
            if(index_mode == DLTS)
            {
                HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ReadDLTS, (PVOID)MeanTemp, 0, NULL);
                CloseHandle(hThread);
            }
            else if(index_mode == ITS)
            {
                HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ReadITS, (PVOID)MeanTemp, 0, NULL);
                CloseHandle(hThread);
            }
            else return -1;
        }
    }
    /* ��������� ����������� ������� */
    gVector vData1;
    for(size_t i = 0; i < vTemp.size(); i++)
        vData1.push_back(i*REFRESH_TIME_THERMOSTAT*0.001);
    if(dAverSqFluct < 0.01) gBand(hGraph, 0, REFRESH_TIME_THERMOSTAT*0.001*MAX_POINT_TEMP_GRAPH, (dTemp-0.5)<0?0:(dTemp-0.5), (dTemp+0.5));
    else gBand(hGraph, 0, REFRESH_TIME_THERMOSTAT*0.001*MAX_POINT_TEMP_GRAPH, 0, 0);
    gData(hGraph, &vData1, &vTemp);
    return 0;
}

UINT CALLBACK ReadDLTS(void* MeanTemp_)
{
    try{
    stringstream buff;
    buff << fixed << setprecision(2);
    vector<double> Relaxation;
    double MeanTemp = *(double*)MeanTemp_;
    int offset = 0; /* �������� �������� � ������� ���������� ���������� ��� ������ ��������� */
    WaitForSingleObject(hDownloadEvent, INFINITE);
    /* �������� �� ������������� ���������� � ������� ������������ */
    for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
    {
        /* ����� ������� ��� ���������� */
        if(MeanTemp > *it) offset++;
        else if(MeanTemp == *it)
        {
            /* ���� ���������� � ������� ������������ ���������� */
            goto J1;
        }
    }
    #ifndef TEST_MODE
    MyDAQMeasure(&Relaxation, averaging_DAQ, measure_time_DAQ*0.001, ai_port, TRUE);
    #endif // TEST_MODE
    /* ������� ���������� ������ */
    #ifdef TEST_MODE
    for(double t = gate_DAQ*0.0000001; t < measure_time_DAQ*0.001; t += 1.0/rate_DAQ)
        Relaxation.push_back(-0.025*exp(-t/(8.9737*0.001) + 0.5755));
    #endif // TEST_MODE
    /* ��������� ����������, ����� ����� ����������� ������������� ����� ���� */
    SavedRelaxations.insert(SavedRelaxations.begin()+offset, Relaxation);
    /* ��������� ����� �� DLTS ������ */
    AddPointsDLTS(&Relaxation, MeanTemp);
    /* ��������� ���������� � ���� */
    SaveRelaxSignal(MeanTemp, &Relaxation, 0.0, 0.0);
    delete (double*)MeanTemp_;
    index_relax = offset;
    PlotRelax();
    PlotDLTS();
    J1:
    /* ��������� ����� ����� ������������ */
    Thermostat.BeginPoint += Thermostat.TempStep;
    if(!Thermostat.range_is_correct()) StartButPush();
    /* ������� ������� �������� Begin Point */
    rewrite(buff) << "Begin point " << Thermostat.BeginPoint << " K";
    SetDlgItemText(hMainWindow, ID_STR_BEGIN, buff.str().data());
    /* ������������� ����� Set Point */
    rewrite(buff) << "SETP " << Thermostat.BeginPoint << "K";
    Thermostat.Write(buff);
    /* ���������� ���� ������������ */
    stability = false;
    } catch(out_of_range  &e){ MessageBox(0, e.what(), "Exception in ReadDLTS", 0); StartButPush(); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in ReadDLTS", 0); StartButPush(); }
    return 0;
}

UINT CALLBACK ReadITS(void* MeanTemp)
{
    try{
    WaitForSingleObject(hDownloadEvent, INFINITE);
    double dVoltBias = 0.0, dVoltAmp = 0.0;
    double dBeginAmplitude = Generator.begin_amplitude;
    vector<double> Relaxation;
    bool endofits = false;
    /* � ������ ITS ������, ���� ���� ����������� */
    while(endofits == false && start == true)
    {
        Relaxation.clear();
        #ifndef TEST_MODE
        MyDAQMeasure(&Relaxation, averaging_DAQ, measure_time_DAQ*0.001, ai_port, TRUE);
        #endif // TEST_MODE
        /* ������� ���������� ������ */
        #ifdef TEST_MODE
        for(double t = gate_DAQ*0.0000001; t < measure_time_DAQ*0.001; t += 1.0/rate_DAQ)
            Relaxation.push_back(-0.025*exp(-t/((8.9737+Generator.begin_amplitude)*0.001) + 0.5755));
        #endif // TEST_MODE
        /* ��������� ����������, ����� ����� ����������� ������������� ����� ���� */
        SavedRelaxations.push_back(Relaxation);
        index_relax = SavedRelaxations.size() - 1;
        /* ��������� �������� ��������� ������������ �������� */
        MeasurePulse(NULL, &dVoltBias, &dVoltAmp);
        itsBiasVoltages.push_back(dVoltBias);
        itsAmpVoltages.push_back(dVoltAmp);
        /* �������������� ��������� ��������� */
        Generator.begin_amplitude += Generator.step_voltage;
        if(Generator.begin_amplitude < Generator.end_amplitude)
        {
            Generator.begin_amplitude = dBeginAmplitude;
            endofits = true;
        }
        Generator.Apply();
        /* ��������� ���������� � ���� */
        SaveRelaxSignal(*(double*)MeanTemp, &Relaxation, dVoltBias, dVoltAmp);
        delete (double*)MeanTemp;
        /* ������������ ������ */
        PlotRelax();
    }
    /* ���������� ���� ������������ */
    } catch(out_of_range  &e){ MessageBox(0, e.what(), "Exception in ReadITS", 0); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in ReadITS", 0); }
    if(start == true)
        StartButPush();
    return 0;
}
