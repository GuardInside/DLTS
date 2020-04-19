#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <exception>
#include <atomic>
#include <memory>

#include "variable.h"
#include "gwin.h"
#include "resource.h"
#include "winfunc.h"
#include "vi.h"
#include "daq.h"
#include "facility.h"
#include "dlts_math.h"
#include "daqmx.h"

using namespace std;
using namespace gwin;
BOOL MainWindow_OnCreate(HWND, LPCREATESTRUCT);
VOID MainWindow_OnCommand(HWND, INT, HWND, UINT);
VOID MainWindow_OnTimer(HWND, UINT);

/* Считываени данных с LakeShore и обновление информации */
UINT Thermostat_Process();

atomic_bool bfRefreshW2{true};
UINT CALLBACK ReadRelaxAndDraw(void*);
UINT CALLBACK ReadPulsesAndDraw(void*);
UINT CALLBACK ReadDLTS(void*);
UINT CALLBACK ReadITS(void*);

HWND hSettinWnd = NULL;         /* Описатель окна настроек */
HWND hAnalysisWnd = NULL;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
        hInst = hInstance;
        MSG messages;
        CreateClass(hInst, "main window class", mwwin_proc);
        /* Считываем все настройки из setting-файла */
        read_settings();
        /* Инициализируем объекты-инструменты             */
        /* и применяем настройки к физическим устройствам */
        Generator.Reset();
        ApplySettings();
        /* Инициализируем объекты ядра */
        InitializeCriticalSection(&csDataAcquisition);
        InitializeCriticalSection(&csSavedData);
        InitializeCriticalSection(&csDAQPaint);
        hDownloadEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
        InitCommonControls();
        hMainWindow = CreateDialog(hInstance, MAKEINTRESOURCE(ID_MAIN_WINDOW), HWND_DESKTOP, NULL);

        /* Тесты */
        //Test(LDLTS);
        while (GetMessage(&messages, NULL, 0, 0))
        {
            if(!IsDialogMessage(hSettinWnd, &messages) && !IsDialogMessage(hAnalysisWnd, &messages))
            {
                /* Перехват сообщений о действиях мышки для окна DLTS графика */
                /* Сообщение из очереди удаляется стандартным обработчиком */
                if(index_w4.load() == 0 && messages.hwnd == hGraph_DLTS)
                    dlts_mouse_message(messages.hwnd, messages.message, messages.wParam, messages.lParam);
                /* Установка фокуса ввода */
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
    gPosition(hGraph, 13, 24);
    gSize(hGraph, 399, 276);
    gPrecision(hGraph, 0, 2);
    gTitle(hGraph, "Temperature from thermostat");
    gAxisInfo(hGraph, "Time [sec]", "Temperature [K]");
    gDefaultPlot(hGraph, "\0");
    hGraph_DAQ = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DAQ, 425, 24);
    gSize(hGraph_DAQ, 399, 276);
    gPrecision(hGraph_DAQ, 0, 3);
    gBand(hGraph_DAQ, 0, 0, -10.0, 10.0);
    gDefaultPlot(hGraph_DAQ, "\0");
    hRelax = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hRelax, 13, 325);
    gSize(hRelax, 399, 276);
    gPrecision(hRelax, 0, 3);
    gBand(hRelax, 0, 0, -10.0, 10.0);
    gDefaultPlot(hRelax, "\0");
    hGraph_DLTS = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DLTS, 425, 325);
    gSize(hGraph_DLTS, 399, 276);
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
    static HANDLE thCalculationDLTS = 0;
    switch(id)
    {
        case WM_REFRESH_DLTS:
            if(WaitForSingleObject(thCalculationDLTS, 0) == WAIT_TIMEOUT)
            {
                StopRefreshDLTS.store(true);
                WaitForSingleObject(thCalculationDLTS, INFINITE);
            }

            CloseHandle(thCalculationDLTS);
            thCalculationDLTS = (HANDLE)_beginthreadex(NULL, 0, RefreshDLTS, 0, 0, NULL);
            StopRefreshDLTS.store(false);
            break;
        case WM_PAINT_RELAX:
            EnterCriticalSection(&csSavedData);
            PlotRelax();
            LeaveCriticalSection(&csSavedData);
            break;
        case WM_PAINT_DLTS:
            EnterCriticalSection(&csSavedData);
            if(index_mode.load() == DLTS)
                PlotDLTS(xAxisDLTS, yAxisDLTS);
            else if(index_mode.load() == ITS)
                PlotDLTS(xAxisITS, yAxisITS);
            LeaveCriticalSection(&csSavedData);
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
            EnterCriticalSection(&csSavedData);
            if(index_relax.load() == 0 || SavedRelaxations.empty())
            {
                LeaveCriticalSection(&csSavedData);
                break;
            }
            LeaveCriticalSection(&csSavedData);
            index_relax--;
            PlotRelax();
            SetFocus(hMainWindow);
            break;
        case ID_BUTTON_NEXT_RELAXATION:
            EnterCriticalSection(&csSavedData);
            if(index_relax.load() == SavedRelaxations.size()-1 || SavedRelaxations.empty())
            {
                LeaveCriticalSection(&csSavedData);
                break;
            }
            LeaveCriticalSection(&csSavedData);
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
        /* Сохранить файл */
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
        case ID_MENU_SAVE_CT:
            SaveWindow(SAVE_CT);
            break;
        case ID_MENU_CLEAR_MEMORY:
            ClearMemmory();
            break;
        case ID_MENU_AUTO_PEAK_DETECTING:
            menu::automatic = (menu::automatic ? false : true);
            SendMessage(hMainWindow, WM_COMMAND, WM_REFRESH_MENU, 0);
            SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
            break;
        case ID_MENU_DIVIDE_S_ON_TC:
            menu::divide = (menu::divide ? false : true);
            SendMessage(hMainWindow, WM_COMMAND, WM_REFRESH_MENU, 0);
            SendMessage(hMainWindow, WM_COMMAND, WM_REFRESH_DLTS, 0);
            break;
        case WM_REFRESH_MENU:
            CheckMenuItem(GetSubMenu(GetMenu(hwnd), 1), ID_MENU_AUTO_PEAK_DETECTING, menu::automatic ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(GetMenu(hwnd), 1), ID_MENU_DIVIDE_S_ON_TC, menu::divide ? MF_CHECKED : MF_UNCHECKED);
            break;
        /* Загрузить файл */
        case ID_MENU_OPEN:
            DownloadWindow();
            break;
        /* Настройки */
        case ID_MENU_FITTING_BSPLINE:
            if(!IsWindow(hAnalysisWnd))
                hAnalysisWnd = CreateDialog(hInst, MAKEINTRESOURCE(ID_ANALYSIS_WINDOW), HWND_DESKTOP, anwin_proc);
            break;
        case ID_BUTTON_SETTINGS:
            if(!IsWindow(hSettinWnd))
                hSettinWnd = CreateDialog(hInst, MAKEINTRESOURCE(ID_SETTINGS_WINDOW), HWND_DESKTOP, stwin_proc);
            break;
        case ID_CHECKBOX_FIX_TEMPERATURE:
            {
                static double prevSetPoint = 0.0;
                /* Активирован флаг фиксации температуры */
                if(fix_temp.load() == false)
                {
                    Thermostat.CurrentSetPoint(&prevSetPoint);
                    double CurrentTemperature = 0.0;
                    Thermostat.CurrentTemp(&CurrentTemperature);
                    /* Установка сетпоинта для термостата как текущую температуру */
                    Thermostat.SetPoint(CurrentTemperature);
                    /* Установка RANGE в соответствии с текущей зоной */
                    Thermostat.SwitchHeater(vi::switcher::on);
                }
                else
                {
                    /* Установка сетпоинта в соответствии с настройками */
                    Thermostat.SetPoint(prevSetPoint);
                    /* Установка RANGE в соответствии с текущей зоной */
                    if(start.load() == true && stability.load() == false)
                        Thermostat.SwitchHeater(vi::switcher::on);
                    else
                        Thermostat.SwitchHeater(vi::switcher::off);
                }
                (fix_temp.load() == true)? fix_temp.store(false) : fix_temp.store(true);
            }
            break;
        /* Старт/стоп */
        case ID_BUTTON_START:
            if(fix_temp.load() == true)
                SendMessage(hMainWindow, WM_COMMAND, ID_CHECKBOX_FIX_TEMPERATURE, 0);
            if(start.load() == false && !Thermostat.range_is_correct())
            {
                MessageBox(hwnd, "Invalid set point or end point value,\nor step temperature sign.", "Warning", MB_ICONWARNING);
                break;
            }
            if(start.load() == false)
            {
                DialogBox(hInst,  MAKEINTRESOURCE(ID_START_WINDOW_DLTS), hwnd, srwin_proc);
            }
            else if(start.load() == true)
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

    gwin::gVector vData2;
    Measuring(&vData2, 1, measure_time_DAQ, ai_port_measurement, index_range.load());

    stringstream buff;
    buff << fixed << "AI: " << ai_port_measurement << endl
         << "Cap. [pF] = " << fixed << setprecision(3) << capacity;
    gAdditionalInfo(hGraph_DAQ, buff.str());

    //for(size_t i = 0; i < vData2.size(); i++)
        //vData1.push_back(0.001*gate_DAQ + i*(1.0/rate_DAQ)*1000.0);
    plotDAQ(&TimeAxis, &vData2);
    bfRefreshW2.store(true);
    return 0;
}
UINT CALLBACK ReadPulsesAndDraw(void*)
{
    gwin::gVector vData2;
    double dBias = 0.0, dAmp = 0.0;
    PulsesMeasuring(&vData2, &dBias, &dAmp);

    stringstream buff;
    buff << fixed << "AI: " << ai_port_pulse << endl
         << setprecision(3) << "Bias [V] =  " << dBias << endl << "Amp [V] = " << dAmp;
    gAdditionalInfo(hGraph_DAQ, buff.str());

    //plotDAQ(&TimeAxis, &vData2);

    bfRefreshW2.store(true);
    return 0;
}

/* Считываени данных с термостата и обновление информации */
UINT Thermostat_Process()
{
    static vector<double> vTemp, vTime;
    double dTemp = 0.0, HeatPercent = 0.0, SetPoint = 0.0;
    stringstream buff;
    buff << setprecision(THERMO_PRECISION) << fixed;
    /* Получаем значение текущей температуры */
    Thermostat.CurrentTemp(&dTemp);
    #ifdef TEST_MODE
        //srand(time(NULL));
        dTemp = 295;//rand()%50+10;
    #endif
    dTemp = round(dTemp, 2);
    /* Сохраняем полученное значение */
    vTemp.push_back(dTemp);
    /* Удаляем старое значение */
    if(vTemp.size() > MAX_POINT_TEMP_GRAPH)
        vTemp.erase(vTemp.begin());
    else /* Ось времени заполняется до MAX_POINT_TEMP_GRAPH точек и остается неизменной до конца работы программы */
        vTime.push_back( (vTemp.size() - 1 )*REFRESH_TIME_THERMOSTAT*0.001);

    /* Мощность ТЭНа в процентах */
    Thermostat.CurrentHeatPercent(&HeatPercent);
    rewrite(buff) << "Power " << HeatPercent << "%";
    SetDlgItemText(hMainWindow, ID_STR_POWER, buff.str().data());
    /* Текущая температура */
    rewrite(buff) << "Temperature " << dTemp << " K ";
    SetDlgItemText(hMainWindow, ID_STR_CHANNEL_A, buff.str().data());
    /* Выводим текущее значение SetPoint */
    Thermostat.CurrentSetPoint(&SetPoint);
    rewrite(buff) << "Set point " << SetPoint << " K";
    SetDlgItemText(hMainWindow, ID_STR_SETPOINT, buff.str().data());
    /* Рассчитываем и выводим Mean-square error */
    double MSE = MeanSquareErrorOfTemp(vTemp.cbegin(), vTemp.cend());
    rewrite(buff) << "Mean-squared error " << MSE << " K";
    SetDlgItemText(hMainWindow, ID_STR_DISPERSION, buff.str().data());

    /* Проверяем условие стабилизации первое */
    if(start.load() == true && stability.load() == false && fix_temp.load() == false && MSE <= Thermostat.TempDisp)
    {
        double dMean = round( MeanOfTemp(vTemp.cbegin(), vTemp.cend()), THERMO_PRECISION );
        /* Проверяем условие стабилизации второе */
        if(std::abs(dMean - Thermostat.BeginPoint) <= Thermostat.TempDisp)
        {
            stability.store(true);
            CloseHandle((HANDLE)_beginthreadex(NULL, 0, ReadDLTS, (PVOID)(new double(dMean)), 0, NULL));
        }
    }
    /* Обновляем изображение графика */
    if(MSE < 0.01)
        gBand(hGraph, 0, REFRESH_TIME_THERMOSTAT*0.001*MAX_POINT_TEMP_GRAPH, (dTemp-0.5)<0?0:(dTemp-0.5), (dTemp+0.5));
    else
        gBand(hGraph, 0, REFRESH_TIME_THERMOSTAT*0.001*MAX_POINT_TEMP_GRAPH, 0, 0);
    gData(hGraph, &vTime, &vTemp);
    return 0;
}

/*
    Потоковые функции, определенные ниже, строго говоря, не соответствуют
    требованиям потокобезопасности. Тем не менее, их использования именнов
    в таком виде в рамках данного проекта допустимо.
*/

/* Если релаксация с Т уже есть, то возвращает true */
bool TryFindRelaxation(double T, size_t *offset)
{
    bool bfAlreadyExist = false;
    *offset = 0; /* Величина смещения в массиве сохраненых релаксаций для записи следующей */
    for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
    {
        /* Выбор позиции для сохранения */
        if(T > *it) (*offset)++;
        else if(T == *it)
        {
            /* Если релаксация с текущей температурой существует */
            bfAlreadyExist = true;
        }
    }
    return bfAlreadyExist;
}

void PrepareNextTemp()
{
    Thermostat.BeginPoint += Thermostat.TempStep;
    if(!Thermostat.range_is_correct())
        StartButPush();
    stringstream buff;
    buff << fixed << setprecision(THERMO_PRECISION)
         << "Begin point " << Thermostat.BeginPoint << " K";
    SetDlgItemText(hMainWindow, ID_STR_BEGIN, buff.str().data());

    Thermostat.SetPoint(Thermostat.BeginPoint);
}

vector<vector<double>> TransientMeasuring()
{
    vector<vector<double>> Buffer;
    Buffer.resize(averaging_DAQ);
    int i = 0;
    double extra_gate = 0.0;
    if(Generator.is_active.load())
        extra_gate = Generator.width * 1000;
    NIDAQmx::Task Device("DLTS Task");
    Device.AddChannel(id_DAQ, ai_port_measurement, index_range.load());
    Device.SetupFiniteAcquisition(rate_DAQ, measure_time_DAQ);
    Device.SetupTrigger(pfi_ttl_port, DAQmx_Val_Rising, gate_DAQ + extra_gate);
    EnterCriticalSection(&csDataAcquisition);
    for_each(Buffer.begin(), Buffer.end(), [&](vector<double> &subBuffer)
             {
                 SendMessage(hProgress, PBM_SETPOS, 100.0*(i++)/averaging_DAQ, 0);
                 Device.Start();
                 Device.TryRead(&subBuffer);
                 Device.Stop();
             });
    LeaveCriticalSection(&csDataAcquisition);
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    return Buffer;
}

void DataProcessing(vector<vector<double>> &&Buffer, vector<double> *Relaxation)
{
    int i = 0;
    size_t BufferSize = rate_DAQ * measure_time_DAQ*0.001;
    Relaxation->resize(BufferSize, 0.0);
    for_each(Buffer.begin(), Buffer.end(), [&](vector<double> &subBuffer)
             {
                 SendMessage(hProgress, PBM_SETPOS, 100.0*(i++)/averaging_DAQ, 0);
                 std::transform(subBuffer.cbegin(), subBuffer.cend(), Relaxation->cbegin(), Relaxation->begin(), std::plus<double>());
             });
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    i = 0;
    for_each(Relaxation->begin(), Relaxation->end(), [&](double &value)
             {
                 SendMessage(hProgress, PBM_SETPOS, 100.0*(i++)/BufferSize, 0);
                 value /= averaging_DAQ;
             });
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
}

UINT CALLBACK ReadDLTS(void* ptrMeanTemp)
{
    try{
        double capacity = 1.0;
        double MeanTemp = *(double*)ptrMeanTemp;
            delete (double*)ptrMeanTemp;
        WaitForSingleObject(hDownloadEvent, INFINITE);
        size_t offset = 0;
        if( TryFindRelaxation(MeanTemp, &offset) )
        {
            PrepareNextTemp();
            stability.store(false);
        }
        else
        {
            CapacityMeasuring(ai_port_capacity, &capacity);
            VoltageToCapacity(&capacity);

            vector<double> Relaxation;
            DataProcessing(TransientMeasuring() , &Relaxation);

            PrepareNextTemp();
            stability.store(false);
            /* Сохраняем релаксации, чтобы иметь возможность переключаться между ними */
            EnterCriticalSection(&csSavedData);
                SavedRelaxations.insert(SavedRelaxations.begin()+offset, Relaxation);
                SavedCapacity.insert(SavedCapacity.begin()+offset, capacity);
            LeaveCriticalSection(&csSavedData);
            AddPointToDLTS(&Relaxation, MeanTemp, capacity);
            SaveRelaxSignalToFile(MeanTemp, &Relaxation, 0.0, 0.0, capacity);
            index_relax.store(offset);
            PostMessage(hMainWindow, WM_COMMAND, WM_PAINT_RELAX, 0);
            PostMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
        }
        stability.store(false);
    } catch(out_of_range const &e){ MessageBox(0, e.what(), "Exception in ReadDLTS", 0); StartButPush(); }
    catch(NIDAQmx::DAQException const &e)
    {
        bfDAQ0k.store(false);
        MessageBox(HWND_DESKTOP, e.description().c_str(), "Exception @DAQException", MB_ICONWARNING);
    }
    catch(std::runtime_error const &e)
    {
        bfDAQ0k.store(false);
        MessageBox(HWND_DESKTOP, e.what(), "Warning", MB_ICONWARNING);
    }
      catch(...){ MessageBox(0, "Some exception.", "Exception in ReadDLTS", 0); StartButPush(); }
    return 0;
}
