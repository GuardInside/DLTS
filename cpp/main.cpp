#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <exception>
#include <atomic>

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
VOID MainWindow_OnCommand(HWND, INT, HWND, UINT);
VOID MainWindow_OnTimer(HWND, UINT);

/* ���������� ������ � LakeShore � ���������� ���������� */
UINT Thermostat_Process();

atomic_bool bfRefreshW2{true};
UINT CALLBACK ReadRelaxAndDraw(void*);
UINT CALLBACK ReadPulsesAndDraw(void*);
atomic_bool bfMeasurePermit{true};
UINT CALLBACK ReadITS(void*);
UINT CALLBACK ReadDLTS(void*);

HWND hSettinWnd = NULL;         /* ��������� ���� �������� */
HWND hAnalysisWnd = NULL;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
        hInst = hInstance;
        MSG messages;
        CreateClass(hInst, "main window class", mwwin_proc);
        /* ��������� ��� ��������� �� setting-����� */
        read_settings();
        /* �������������� �������-�����������             */
        /* � ��������� ��������� � ���������� ����������� */
        ApplySettings();
        /* �������������� ������� ���� */
        InitializeCriticalSection(&csDataAcquisition);
        InitializeCriticalSection(&csSavedRelaxation);
        InitializeCriticalSection(&csDAQPaint);
        hDownloadEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
        InitCommonControls();
        hMainWindow = CreateDialog(hInstance, MAKEINTRESOURCE(ID_MAIN_WINDOW), HWND_DESKTOP, NULL);

        while (GetMessage(&messages, NULL, 0, 0))
        {
            if(!IsDialogMessage(hSettinWnd, &messages) && !IsDialogMessage(hAnalysisWnd, &messages))
            {
                /* �������� ��������� � ��������� ����� ��� ���� DLTS ������� */
                /* ��������� �� ������� ��������� ����������� ������������ */
                if(index_w4.load() == 0 && messages.hwnd == hGraph_DLTS)
                    dlts_mouse_message(messages.hwnd, messages.message, messages.wParam, messages.lParam);
                /* ��������� ������ ����� */
                RECT rectRelax;
                RECT rectDLTS;
                GetWindowRect(hRelax, &rectRelax);
                GetWindowRect(hGraph_DAQ, &rectDLTS);

                if(messages.message == WM_MOUSEWHEEL)
                {
                    if(PtInRect(&rectRelax, messages.pt))
                        relax_mouse_message(hRelax, messages.message, messages.wParam, messages.lParam);
                    else if(PtInRect(&rectDLTS, messages.pt) && index_w4.load() == 0)
                        daq_mouse_message(hGraph_DAQ, messages.message, messages.wParam, messages.lParam);
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
    SendMessage(hwnd, WM_COMMAND, WM_PAINT_RELAX, 0);
    /* ��������� ���� */
    BuildMenu(hwnd);
    /* �������� ���� */
    hGraph = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph, 10, 22);
    gSize(hGraph, 399, 276);
    gPrecision(hGraph, 0, 2);
    gAxisInfo(hGraph, "Time [sec]", "T e m p e r a t u r e  [ K ]");
    gDefaultPlot(hGraph, "\0");
    hGraph_DAQ = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DAQ, 422, 22);
    gSize(hGraph_DAQ, 399, 276);
    gPrecision(hGraph_DAQ, 0, 3);
    gBand(hGraph_DAQ, 0, 0, -10.0, 10.0);
    gAxisInfo(hGraph_DAQ, "Time [ms]", "V o l t a g e [ V ]");
    gDefaultPlot(hGraph_DAQ, "\0");
    hRelax = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hRelax, 10, 323);
    gSize(hRelax, 399, 276);
    gPrecision(hRelax, 0, 3);
    gBand(hRelax, 0, 0, -10.0, 10.0);
    gAxisInfo(hRelax, "Time [ms]", "V o l t a g e [ V ]");
    gDefaultPlot(hRelax, "\0");
    hGraph_DLTS = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DLTS, 422, 323);
    gSize(hGraph_DLTS, 399, 276);
    gAxisInfo(hGraph_DLTS, "Temperature [K]", "S [ A r b ]");
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
        case WM_PAINT_RELAX:
            PlotRelax();
            break;
        case WM_PAINT_DLTS:
            PlotDLTS();
            break;
        case ID_BUTTON_PREVIOUS_W2:
            if(index_w2.load() == 0 || !bfDAQ0k)
                break;
            index_w2--;
            if(index_w2.load() == 0)
                daq_mouse_message(hGraph_DAQ, WM_MOUSEWHEEL, 0, 0);
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_NEXT_W2:
            if(index_w2.load() == 1 || !bfDAQ0k)
                break;
            index_w2++;
            if(index_w2.load() == 1)
                gBand(hGraph_DAQ, 0, 0, 0, 0);
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_PREVIOUS_RELAXATION:
            EnterCriticalSection(&csSavedRelaxation);
            if(index_relax.load() == 0 || SavedRelaxations.empty())
            {
                LeaveCriticalSection(&csSavedRelaxation);
                break;
            }
            LeaveCriticalSection(&csSavedRelaxation);
            index_relax--;
            PlotRelax();
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_NEXT_RELAXATION:
            EnterCriticalSection(&csSavedRelaxation);
            if(index_relax.load() == SavedRelaxations.size()-1 || SavedRelaxations.empty())
            {
                LeaveCriticalSection(&csSavedRelaxation);
                break;
            }
            LeaveCriticalSection(&csSavedRelaxation);
            index_relax++;
            PlotRelax();
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_PREVIOUS_W4:
            if(index_w4.load() == 0)
                break;
            index_w4--;
            SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_NEXT_W4:
            if(index_w4.load() == 2)
                break;
            index_w4++;
            SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
            SetFocus(hMainWindow);
            break;
        /* ��������� ���� */
        case ID_MENU_SAVE_SETTINGS:
            SaveWindow(SAVE_SETTINGS);
            break;
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
            auto_peak_search.store(auto_peak_search.load() ? false : true);
            CheckMenuItem(GetSubMenu(GetMenu(hwnd), 1), ID_MENU_AUTO_PEAK_DETECTING, auto_peak_search.load() ? MF_CHECKED : MF_UNCHECKED);
            SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
            break;
        case ID_MENU_NORMALIZE_TO_ONE:
            normaliz_dlts.store(normaliz_dlts.load() ? false : true);
            CheckMenuItem(GetSubMenu(GetMenu(hwnd), 1), ID_MENU_NORMALIZE_TO_ONE, normaliz_dlts.load() ? MF_CHECKED : MF_UNCHECKED);
            SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
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
                if(fix_temp.load() == false)
                {
                    Thermostat.CurrentTemp(&dPrevTemp);
                    double CurrentTemperature = 0.0;
                    Thermostat.CurrentTemp(&CurrentTemperature);
                    /* ��������� ��������� ��� ���������� ��� ������� ����������� */
                    Thermostat.SetPoint(CurrentTemperature);
                    /* ��������� RANGE � ������������ � ������� ����� */

                    Thermostat.SwitchHeater(vi::switcher::on);
                }
                else
                {
                    /* ��������� ��������� � ������������ � ����������� */
                    Thermostat.SetPoint(dPrevTemp);
                    /* ��������� RANGE � ������������ � ������� ����� */
                    if(start.load() == true)
                        Thermostat.SwitchHeater(vi::switcher::on);
                    else
                        Thermostat.SwitchHeater(vi::switcher::off);
                }
                (fix_temp.load() == true)? fix_temp.store(false) : fix_temp.store(true);
            }
            break;
        /* �����/���� */
        case ID_BUTTON_START:
            if(start.load() == false && !Thermostat.range_is_correct())
            {
                MessageBox(hwnd, "Invalid set point or end point value,\nor step temperature sign.", "Warning", MB_ICONWARNING);
                break;
            }
            if(start.load() == false)
            {
                switch(index_mode.load())
                {
                    case mode::DLTS:
                        DialogBox(hInst,  MAKEINTRESOURCE(ID_START_WINDOW_DLTS), hwnd, srwin_proc);
                        break;
                    case mode::AITS:
                        DialogBox(hInst,  MAKEINTRESOURCE(ID_START_WINDOW_AITS), hwnd, srwin_proc);
                        break;
                    case mode::BITS:
                        break;
                    case mode::PITS:
                        break;
                    case mode::CV:
                        DialogBox(hInst,  MAKEINTRESOURCE(ID_START_WINDOW_CV), hwnd, srwin_proc);
                        break;
                }
            }
            else if(start.load() == true)
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
                Generator.Channel(vi::switcher::off);
                Thermostat.SwitchHeater(vi::switcher::off);
                DestroyWindow(hwnd);
            break;
    }
}


VOID MainWindow_OnTimer(HWND hwnd, UINT id)
{
    switch(id)
    {
        case DAQ_TIMER:
            if(bfDAQ0k.load() == true)
            {
                if(bfRefreshW2.load() == true)
                {
                    if(index_w2.load() == 0)
                    {
                        bfRefreshW2.store(false);
                        CloseHandle((HANDLE)_beginthreadex(NULL, 0, ReadRelaxAndDraw, 0, 0, NULL));
                    }
                    else if(index_w2.load() == 1)
                    {
                        bfRefreshW2.store(false);
                        CloseHandle((HANDLE)_beginthreadex(NULL, 0, ReadPulsesAndDraw, 0, 0, NULL));
                    }
                    else MessageBox(HWND_DESKTOP, "Undefined case in timer function.", "Error", MB_ICONERROR);
                }
            }
            else gDefaultPlot(hGraph_DAQ, "The NIDAQ isn't being initialized.\nYou should change gpib setting.");
            break;
        case THERMOSTAT_TIMER:
            #ifndef TEST_MODE
            if(Thermostat.bfvi0k.load())
            {
            #endif // TEST_MODE
            Thermostat_Process();
            #ifndef TEST_MODE
            }
            else gDefaultPlot(hGraph, "The thermostat LakeShore isn't being initialized.\nYou should change gpib setting.");
            #endif // TEST_MODE
            break;
    }
}

UINT CALLBACK ReadRelaxAndDraw(void*)
{
    double capacity = 0.0;
    CapacityMeasuring(ai_port_capacity, &capacity);
    VoltageToCapacity(&capacity);

    gwin::gVector vData1, vData2;
    Measuring(&vData2, 1, measure_time_DAQ*0.001, ai_port, index_range.load());

    stringstream buff;
    buff << fixed << "AI: " << ai_port << endl
         << "Cap. [pF] = " << fixed << setprecision(3) << capacity;
    gAdditionalInfo(hGraph_DAQ, buff.str());

    for(size_t i = 0; i < vData2.size(); i++)
        vData1.push_back(0.001*gate_DAQ + i*(1.0/rate_DAQ)*1000.0);
    plotDAQ(&vData1, &vData2);
    bfRefreshW2.store(true);
    return 0;
}
UINT CALLBACK ReadPulsesAndDraw(void*)
{
    gwin::gVector vData1, vData2;
    double dBias = 0.0, dAmp = 0.0;
    PulsesMeasuring(&vData2, &dBias, &dAmp);

    stringstream buff;
    buff << fixed << "AI: " << ai_port_pulse << endl
         << setprecision(3) << "Bias [V] =  " << dBias << endl << "Amp [V] = " << dAmp;
    gAdditionalInfo(hGraph_DAQ, buff.str());

    for(size_t i = 0; i < vData2.size(); i++)
        vData1.push_back(0.001*gate_DAQ + i*(1.0/rate_DAQ)*1000.0);
    plotDAQ(&vData1, &vData2);

    bfRefreshW2.store(true);
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
    Thermostat.CurrentTemp(&dTemp);
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
    Thermostat.CurrentHeatPercent(&HeatPercent);
    rewrite(buff) << "Power " << HeatPercent << "%";
    SetDlgItemText(hMainWindow, ID_STR_POWER, buff.str().data());
    /* ������� ����������� */
    rewrite(buff) << "Temperature " << dTemp << " K ";
    SetDlgItemText(hMainWindow, ID_STR_CHANNEL_A, buff.str().data());
    /* ������� ������� �������� SetPoint */
    Thermostat.CurrentSetPoint(&SetPoint);
    rewrite(buff) << "Set point " << SetPoint << " K";
    SetDlgItemText(hMainWindow, ID_STR_SETPOINT, buff.str().data());
    /* ������������ � ������� Mean-square error */
    try{
    double MSE = MeanSquareErrorOfTemp(vTemp.cbegin(), vTemp.cend());
    rewrite(buff) << "Mean-squared error " << MSE << " K";
    SetDlgItemText(hMainWindow, ID_STR_DISPERSION, buff.str().data());
    /* ��������� ������� ������������ ������ */
    if(start.load() == true && stability.load() == false && fix_temp.load() == false && MSE <= Thermostat.TempDisp)
    {
        double dMean = round( MeanOfTemp(vTemp.cbegin(), vTemp.cend()), THERMO_PRECISION );
        /* ��������� ������� ������������ ������ */
        if(std::abs(dMean - Thermostat.BeginPoint) <= Thermostat.TempDisp)
        {
            if(bfMeasurePermit.load() == true)
            {
                stability.store(true);
                double *MeanTemp = new double(dMean);
                switch(index_mode.load())
                {
                    case DLTS:
                        CloseHandle((HANDLE)_beginthreadex(NULL, 0, ReadDLTS, (PVOID)MeanTemp, 0, NULL));
                        break;
                    case AITS:
                        CloseHandle((HANDLE)_beginthreadex(NULL, 0, ReadITS, (PVOID)MeanTemp, 0, NULL));
                        break;
                    case BITS:
                        break;
                    case PITS:
                        break;
                    case CV:
                        break;
                }
            }
        }
    }
    /* ��������� ����������� ������� */
    gVector vData1;
    for(size_t i = 0; i < vTemp.size(); i++)
        vData1.push_back(i*REFRESH_TIME_THERMOSTAT*0.001);
    if(MSE < 0.01) gBand(hGraph, 0, REFRESH_TIME_THERMOSTAT*0.001*MAX_POINT_TEMP_GRAPH, (dTemp-0.5)<0?0:(dTemp-0.5), (dTemp+0.5));
    else gBand(hGraph, 0, REFRESH_TIME_THERMOSTAT*0.001*MAX_POINT_TEMP_GRAPH, 0, 0);
    gData(hGraph, &vData1, &vTemp);
    }catch(std::exception const &e)
    {
        MessageBox(HWND_DESKTOP, e.what(), "Exception window", 0);
    }
    return 0;
}

/*
    ��������� �������, ������������ ����, ������ ������, �� �������������
    ����������� ������������������. ��� �� �����, �� ������������� �������
    � ����� ���� � ������ ������� ������� ���������.
*/

UINT CALLBACK ReadDLTS(void* ptrMeanTemp)
{
    bfMeasurePermit.store(false);
    try{
        stringstream buff;
        buff << fixed << setprecision(2);
        vector<double> Relaxation;
        Relaxation.reserve(rate_DAQ*measure_time_DAQ*0.001);
        double MeanTemp = *(double*)ptrMeanTemp;
            delete (double*)ptrMeanTemp;
        int offset = 0; /* �������� �������� � ������� ���������� ���������� ��� ������ ��������� */
        double capacity = 1.0;
        WaitForSingleObject(hDownloadEvent, INFINITE);
        /* �������� �� ������������� ���������� � ������� ������������ */
        bool bfAlreadyExist = false;
        for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
        {
            /* ����� ������� ��� ���������� */
            if(MeanTemp > *it) offset++;
            else if(MeanTemp == *it)
            {
                /* ���� ���������� � ������� ������������ ���������� */
                bfAlreadyExist = true;
            }
        }
        if(bfAlreadyExist)
        {
            Thermostat.BeginPoint += Thermostat.TempStep;   // TUS
            if(!Thermostat.range_is_correct()) StartButPush();
            /* ������� ������� �������� Begin Point */
            rewrite(buff) << "Begin point " << Thermostat.BeginPoint << " K";
            SetDlgItemText(hMainWindow, ID_STR_BEGIN, buff.str().data());

            Thermostat.SetPoint(Thermostat.BeginPoint);
        }
        else
        {
            #ifndef TEST_MODE
            Measuring(&Relaxation, averaging_DAQ, measure_time_DAQ*0.001, ai_port, index_range.load(), TRUE); // ������������
            CapacityMeasuring(::ai_port_capacity, &capacity);

            Thermostat.BeginPoint += Thermostat.TempStep;
            if(!Thermostat.range_is_correct()) StartButPush();
            /* ������� ������� �������� Begin Point */
            rewrite(buff) << "Begin point " << Thermostat.BeginPoint << " K";
            SetDlgItemText(hMainWindow, ID_STR_BEGIN, buff.str().data());

            Thermostat.SetPoint(Thermostat.BeginPoint);

            VoltageToCapacity(&capacity);
            #endif // TEST_MODE
            /* ������� ���������� ������ */
            #ifdef TEST_MODE
            for(double t = gate_DAQ*0.0000001; t < measure_time_DAQ*0.001; t += 1.0/rate_DAQ)
                Relaxation.push_back(-0.025*exp(-t/(8.9737*0.001) + 0.5755));
            #endif // TEST_MODE
            /* ��������� ����������, ����� ����� ����������� ������������� ����� ���� */
            EnterCriticalSection(&csSavedRelaxation);
                SavedRelaxations.insert(SavedRelaxations.begin()+offset, Relaxation);
                SavedCapacity.insert(SavedCapacity.begin()+offset, capacity);
            LeaveCriticalSection(&csSavedRelaxation);
            /* ��������� ����� �� DLTS ������ */
            AddPointsDLTS(&Relaxation, MeanTemp, capacity);   // TUS
            /* ��������� ���������� � ���� */
            SaveRelaxSignal(MeanTemp, &Relaxation, 0.0, 0.0, capacity);
            index_relax.store(offset);
            SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_RELAX, 0);
            SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
        }
        stability.store(false);
        bfMeasurePermit.store(true);
    } catch(out_of_range const &e){ MessageBox(0, e.what(), "Exception in ReadDLTS", 0); StartButPush(); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in ReadDLTS", 0); StartButPush(); }
    return 0;
}

UINT CALLBACK ReadITS(void* ptrMeanTemp)
{
    bfMeasurePermit.store(false);
    try{
        WaitForSingleObject(hDownloadEvent, INFINITE);
        double dVoltBias = 0.0, dVoltAmp = 0.0;
        double begin_amp = Generator.begin_amp;
        double end_amp   = Generator.end_amp;
        double step_amp  = Generator.step_amp;
        vector<double> Relaxation;
        Relaxation.reserve(rate_DAQ*measure_time_DAQ*0.001);
        /* � ������ ITS ������, ���� ���� ����������� */
        while(begin_amp <= end_amp && start.load() == true)
        {
            Relaxation.clear();
            Generator.Amp(begin_amp);
            #ifndef TEST_MODE
            Measuring(&Relaxation, averaging_DAQ, measure_time_DAQ*0.001, ai_port, index_range.load(), TRUE);
            #endif // TEST_MODE
            /* ������� ���������� ������ */
            #ifdef TEST_MODE
            for(double t = gate_DAQ*0.0000001; t < measure_time_DAQ*0.001; t += 1.0/rate_DAQ)
                Relaxation.push_back(-0.025*exp(-t/((8.9737+Generator.begin_amp)*0.001) + 0.5755));
            #endif // TEST_MODE
            /* ��������� ����������, ����� ����� ����������� ������������� ����� ���� */
            EnterCriticalSection(&csSavedRelaxation);
                SavedRelaxations.push_back(Relaxation);
                index_relax.store(SavedRelaxations.size() - 1);
            LeaveCriticalSection(&csSavedRelaxation);
            /* ��������� �������� ��������� ������������ �������� */
            PulsesMeasuring(NULL, &dVoltBias, &dVoltAmp);
            itsBiasVoltages.push_back(dVoltBias);
            itsAmpVoltages.push_back(dVoltAmp);
            /* �������������� ��������� ��������� */
            begin_amp += step_amp;
            /* ��������� ���������� � ���� */
            SaveRelaxSignal(*(double*)ptrMeanTemp, &Relaxation, dVoltBias, dVoltAmp, 0.0);
            /* ������������ ������ */
            SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_RELAX, 0);
        }
        delete (double*)ptrMeanTemp;
        bfMeasurePermit.store(true);
    /* ���������� ���� ������������ */
    } catch(out_of_range const &e){ MessageBox(0, e.what(), "Exception in ReadITS", 0); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in ReadITS", 0); }
    if(start.load() == true)
        StartButPush();
    return 0;
}
