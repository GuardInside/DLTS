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
VOID MainWindow_OnCommand(HWND, INT, HWND, UINT);
VOID MainWindow_OnTimer(HWND, UINT);

/* Считываени данных с LakeShore и обновление информации */
UINT Thermostat_Process();
UINT CALLBACK DataAcquisition_Process(void*);
UINT CALLBACK ReadITS(void*);
UINT CALLBACK ReadDLTS(void*);

HWND hSettinWnd = NULL;         /* Описатель окна настроек */
HWND hAnalysisWnd = NULL;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
        hInst = hInstance;
        MSG messages;
        /* Регистрируем классы окон */
        CreateClass(hInst, "main window class", mwwin_proc);
        /* Считываем все настройки из setting-файла */
        read_settings();
        /* Инициализируем объекты-инструменты             */
        /* и применяем настройки к физическим устройствам */
        ApplySettings();
        /* Инициализируем объекты ядра */
        InitializeCriticalSection(&csDataAcquisition);
        hDownloadEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
        /* Подгружаем динамические библиотеки */
        InitCommonControls();
        /* Создаем окно и переходим в цикл обработки сообщений */
        hMainWindow = CreateDialog(hInstance, MAKEINTRESOURCE(ID_MAIN_WINDOW), HWND_DESKTOP, NULL);

        while (GetMessage(&messages, NULL, 0, 0))
        {
            if(!IsDialogMessage(hSettinWnd, &messages) && !IsDialogMessage(hAnalysisWnd, &messages))
            {
                /* Перехват сообщений о действиях мышки для окна DLTS графика */
                /* Сообщение из очереди удаляется стандартным обработчиком */
                if(index_plot_DLTS == 0 && messages.hwnd == hGraph_DLTS)
                    dlts_mouse_message(messages.hwnd, messages.message, messages.wParam, messages.lParam);
                else if(messages.hwnd == hRelax)
                    relax_mouse_message(messages.hwnd, messages.message, messages.wParam, messages.lParam);
                else if(messages.hwnd == hGraph_DAQ)
                    daq_mouse_message(messages.hwnd, messages.message, messages.wParam, messages.lParam);

                TranslateMessage(&messages);
                DispatchMessage(&messages);
            }
        }
        return 0;
}

//Оконная функция основного диалогового окна
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
    /* Настройка меню */
    BuildMenu(hwnd);
    /* Дочернии окна */
    hGraph = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph, 10, 22);
    gSize(hGraph, 399, 276);
    gPrecision(hGraph, 0, 2);
    gDefaultPlot(hGraph, "\0");
    hGraph_DAQ = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DAQ, 422, 22);
    gSize(hGraph_DAQ, 399, 276);
    gPrecision(hGraph_DAQ, 0, 3);
    gBand(hGraph_DAQ, 0, 0, -10.0, 10.0);
    gDefaultPlot(hGraph_DAQ, "\0");
    hRelax = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hRelax, 10, 323);
    gSize(hRelax, 399, 276);
    gPrecision(hRelax, 0, 3);
    gBand(hRelax, 0, 0, -10.0, 10.0);
    gDefaultPlot(hRelax, "\0");
    hGraph_DLTS = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DLTS, 422, 323);
    gSize(hGraph_DLTS, 399, 276);
    gPrecision(hGraph_DLTS, 2, 3);
    gDefaultPlot(hGraph_DLTS, "\0");
    /* Индикатор выполнения */
    hProgress = CreateWindow(PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD, 850, 195, 140, 20, hwnd, (HMENU)ID_PROGRESS, hInst, NULL);
        SendMessage(hProgress, PBM_SETRANGE, 0, 100 << 16);
        SendMessage(hProgress, PBM_SETPOS, 0, 0);
    /* Таймеры */
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
        /* Переключение отображаемых в окне релаксаций */
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
        /* Сохранить файл */
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
        /* Загрузить файл */
        case ID_MENU_OPEN:
            DownloadWindow();
            break;
        /* Настройки */
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
                /* Активирован флаг фиксации температуры */
                if(fix_temp == false)
                {
                    Thermostat.Write("SETP?");
                    Thermostat.ReadDigit(dPrevTemp);
                    double CurrentTemperature = 0.0;
                    Thermostat.Write("CDAT?");
                    Thermostat.ReadDigit(CurrentTemperature);
                    /* Установка сетпоинта для термостата как текущую температуру */
                    rewrite(buff) << "SETP " << CurrentTemperature << "K";
                    Thermostat.Write(buff);
                    /* Установка RANGE в соответствии с текущей зоной */
                    rewrite(buff) << "RANG " << Thermostat.ZoneTable.GetActuallyHeatRange();
                    Thermostat.Write(buff);
                }
                else
                {
                    /* Установка сетпоинта в соответствии с настройками */
                    rewrite(buff) << "SETP " << dPrevTemp << "K";
                    Thermostat.Write(buff);
                    /* Установка RANGE в соответствии с текущей зоной */
                    if(start == true)
                        rewrite(buff) << "RANG " << Thermostat.ZoneTable.GetActuallyHeatRange();
                    else rewrite(buff) << "RANG 0";
                    Thermostat.Write(buff);
                }
                (fix_temp == true)? fix_temp = false : fix_temp = true;
            }
            break;
        /* Старт/стоп */
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
        /* Установка начального и конечного значений температуры */
        case ID_BUTTON_SET:
            {
                double BeginPoint = 0.0, EndPoint = 0.0;
                /* Считываем начальное значение температуры из поля ввода */
                if(EmptyEditBox(hwnd, ID_EDITCONTROL_BEGIN))
                    BeginPoint = Thermostat.BeginPoint;
                else
                    BeginPoint = ApplySettingEditBox(hwnd, ID_EDITCONTROL_BEGIN);
                /* Считываем конечное значение температуры из поля ввода */
                if(EmptyEditBox(hwnd, ID_EDITCONTROL_END))
                    EndPoint = Thermostat.EndPoint;
                else
                    EndPoint = ApplySettingEditBox(hwnd, ID_EDITCONTROL_END);
                /* Проверка корректности считанных значений */
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
                    /* В случае корректного вода обновляем информацию в окне */
                    SetDlgItemText(hwnd, ID_EDITCONTROL_BEGIN, "");
                    rewrite(buff) << "Begin point " << BeginPoint << " K";
                    SetDlgItemText(hwnd, ID_STR_BEGIN, buff.str().data());

                    SetDlgItemText(hwnd, ID_EDITCONTROL_END, "");
                    rewrite(buff) << "End point " << EndPoint << " K";
                    SetDlgItemText(hwnd, ID_STR_END, buff.str().data());
                    /* Установка значений глобальных переменных */
                    Thermostat.BeginPoint = BeginPoint;
                    Thermostat.EndPoint = EndPoint;
                }
            }
            break;
        /* Выход */
        case ID_BUTTON_EXIT:
                Thermostat.Write("RANG 0");
                DestroyWindow(hwnd);
            break;
    }
}


VOID MainWindow_OnTimer(HWND hwnd, UINT id)
{
    static HANDLE thDataAcquisition = NULL; /* Описатель потока для чтения с DAQ по таймеру */
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
            /* Установка флага конца эксперимента */
            Thermostat_Process();
            #ifndef TEST_MODE
            }
            else gDefaultPlot(hGraph_DAQ, "The thermostat LakeShore isn't being initialized.\nYou should change advanced settings.");
            #endif // TEST_MODE
            break;
    }
}

/* Считываем данные с аналогового входа */
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
        VoltageToCapacity(&capacity);
        MyDAQMeasure(&vData2, 5, measure_time_DAQ*0.001, ai_port);
        for(auto &Y: vData2)
            VoltageToCapacity(&Y);
        buff << fixed << "AI: " << ai_port + offset_ai_port << endl
             << "Cap. [pF] = " << fixed << setprecision(3) << capacity;
        gAdditionalInfo(hGraph_DAQ, buff.str());
    }
    for(size_t i = 0; i < vData2.size(); i++)
        vData1.push_back(0.001*gate_DAQ + i*(1.0/rate_DAQ)*1000.0);
    plotDAQ(&vData1, &vData2);
    return 0;
}

/* Считываени данных с термостата и обновление информации */
UINT Thermostat_Process()
{
    static vector<double> vTemp;
    double dTemp = 0.0, HeatPercent = 0.0, SetPoint = 0.0;
    stringstream buff;
    buff << setprecision(2) << fixed;
    /* Получаем значение текущей температуры */
    Thermostat.Write("CDAT?");
    Thermostat.ReadDigit(dTemp);
    dTemp = round(dTemp, 2);
    #ifdef TEST_MODE
        srand(time(NULL));
        dTemp = rand()%50+10;
        dTemp = round(dTemp,2);
    #endif
    /* Сохраняем полученное значение */
    vTemp.push_back(dTemp);
    /* Удаляем старое значение */
    if(vTemp.size() > MAX_POINT_TEMP_GRAPH)
        vTemp.erase(vTemp.begin());
    /* Мощность ТЭНа в процентах */
    Thermostat.Write("HEAT?");
    Thermostat.ReadDigit(HeatPercent);
    rewrite(buff) << "Power " << HeatPercent << "%";
    SetDlgItemText(hMainWindow, ID_STR_POWER, buff.str().data());
    /* Текущая температура */
    rewrite(buff) << "Temperature " << dTemp << " K ";
    SetDlgItemText(hMainWindow, ID_STR_CHANNEL_A, buff.str().data());
    /* Выводим текущее значение SetPoint */
    Thermostat.Write("SETP?");
    Thermostat.ReadDigit(SetPoint);
    rewrite(buff) << "Set point " << SetPoint << " K";
    SetDlgItemText(hMainWindow, ID_STR_SETPOINT, buff.str().data());
    /* Рассчитываем и выводим Mean-square error */
    double dAverSqFluct = AverSqFluct(vTemp);
    rewrite(buff) << "Mean-squared error " << dAverSqFluct << " K";
    SetDlgItemText(hMainWindow, ID_STR_DISPERSION, buff.str().data());
    /* Проверяем условие стабилизации первое */
    if(start == true && stability == false && fix_temp == false && dAverSqFluct <= Thermostat.TempDisp)
    {
        double dMean = round(mean(vTemp), THERMO_PRECISION);
        /* Проверяем условие стабилизации второе */
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
    /* Обновляем изображение графика */
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
    int offset = 0; /* Величина смещения в массиве сохраненых релаксаций для записи следующей */
    WaitForSingleObject(hDownloadEvent, INFINITE);
    /* Проверка на существование релаксации с текущей температурой */
    for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
    {
        /* Выбор позиции для сохранения */
        if(MeanTemp > *it) offset++;
        else if(MeanTemp == *it)
        {
            /* Если релаксация с текущей температурой существует */
            goto J1;
        }
    }
    #ifndef TEST_MODE
    MyDAQMeasure(&Relaxation, averaging_DAQ, measure_time_DAQ*0.001, ai_port, TRUE);
    #endif // TEST_MODE
    /* Создаем релаксацию руками */
    #ifdef TEST_MODE
    for(double t = gate_DAQ*0.0000001; t < measure_time_DAQ*0.001; t += 1.0/rate_DAQ)
        Relaxation.push_back(-0.025*exp(-t/(8.9737*0.001) + 0.5755));
    #endif // TEST_MODE
    /* Сохраняем релаксации, чтобы иметь возможность переключаться между ними */
    SavedRelaxations.insert(SavedRelaxations.begin()+offset, Relaxation);
    /* Добавляем точку на DLTS кривую */
    AddPointsDLTS(&Relaxation, MeanTemp);
    /* Сохраняем релаксацию в файл */
    SaveRelaxSignal(MeanTemp, &Relaxation, 0.0, 0.0);
    delete (double*)MeanTemp_;
    index_relax = offset;
    PlotRelax();
    PlotDLTS();
    J1:
    /* Установка флага конца эксперимента */
    Thermostat.BeginPoint += Thermostat.TempStep;
    if(!Thermostat.range_is_correct()) StartButPush();
    /* Выводим текущее значение Begin Point */
    rewrite(buff) << "Begin point " << Thermostat.BeginPoint << " K";
    SetDlgItemText(hMainWindow, ID_STR_BEGIN, buff.str().data());
    /* Устанавливаем новый Set Point */
    rewrite(buff) << "SETP " << Thermostat.BeginPoint << "K";
    Thermostat.Write(buff);
    /* Сбрасываем флаг стабилизации */
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
    /* В случае ITS режима, цикл ниже повторяется */
    while(endofits == false && start == true)
    {
        Relaxation.clear();
        #ifndef TEST_MODE
        MyDAQMeasure(&Relaxation, averaging_DAQ, measure_time_DAQ*0.001, ai_port, TRUE);
        #endif // TEST_MODE
        /* Создаем релаксацию руками */
        #ifdef TEST_MODE
        for(double t = gate_DAQ*0.0000001; t < measure_time_DAQ*0.001; t += 1.0/rate_DAQ)
            Relaxation.push_back(-0.025*exp(-t/((8.9737+Generator.begin_amplitude)*0.001) + 0.5755));
        #endif // TEST_MODE
        /* Сохраняем релаксации, чтобы иметь возможность переключаться между ними */
        SavedRelaxations.push_back(Relaxation);
        index_relax = SavedRelaxations.size() - 1;
        /* Вычисляем истинные параметры заполняющего импульса */
        MeasurePulse(NULL, &dVoltBias, &dVoltAmp);
        itsBiasVoltages.push_back(dVoltBias);
        itsAmpVoltages.push_back(dVoltAmp);
        /* Подготавливаем следующее измерение */
        Generator.begin_amplitude += Generator.step_voltage;
        if(Generator.begin_amplitude < Generator.end_amplitude)
        {
            Generator.begin_amplitude = dBeginAmplitude;
            endofits = true;
        }
        Generator.Apply();
        /* Сохраняем релаксацию в файл */
        SaveRelaxSignal(*(double*)MeanTemp, &Relaxation, dVoltBias, dVoltAmp);
        delete (double*)MeanTemp;
        /* Отрисовываем график */
        PlotRelax();
    }
    /* Сбрасываем флаг стабилизации */
    } catch(out_of_range  &e){ MessageBox(0, e.what(), "Exception in ReadITS", 0); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in ReadITS", 0); }
    if(start == true)
        StartButPush();
    return 0;
}
