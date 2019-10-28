#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <process.h>

#include "gwin.h"
#include "variable.h"
#include "resource.h"
#include <winfunc.h>
#include <GPIB.h>
#include <daq.h>
#include <facility.h>
#include <dlts_math.h>

using namespace std;
using namespace gwin;
BOOL MainWindow_OnCreate(HWND, LPCREATESTRUCT);
VOID MainWindow_OnCommand(HWND, int, HWND, UINT);
VOID MainWindow_OnTimer(HWND, UINT);

/* Считываени данных с LakeShore и обновление информации */
UINT CALLBACK Thermostat_Process(void*);
UINT CALLBACK DataAcquisition_Process(void*);
UINT CALLBACK ReadAverDAQ(void*);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
        hInst = hInstance;
        MSG messages;
        /* Регистрируем классы окон */
        CreateClass(hInst, "main window class", mwwin_proc);
        CreateClass(hInst, "main settings window class", stwin_proc);
        CreateClass(hInst, "Arrhenius graph window class", arwin_proc);
        /* Считываем все настройки из setting-файла */
        read_settings();
        /* Инициализируем объекты-инструменты             */
        /* и применяем настройки к физическим устройствам */
        ApplySettings();
        /* Инициализируем объекты ядра */
        InitializeCriticalSection(&csDataAcquisition);
        InitializeCriticalSection(&csGlobalVariable);
        /* Подгружаем динамические библиотеки */
        InitCommonControls();
        /* Создаем окно и переходим в цикл обработки сообщений */
        hMainWindow = CreateDialog(hInstance, MAKEINTRESOURCE(ID_MAIN_WINDOW), 0, nullptr);
        while (GetMessage (&messages, NULL, 0, 0))
        {
            TranslateMessage(&messages);
            DispatchMessage(&messages);
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
    /* Дочернии окна */
    hGraph = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph, 10, 22);
    gSize(hGraph, 340, 200);
    hGraph_DAQ = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DAQ, 370, 22);
    gSize(hGraph_DAQ, 340, 200);
    hRelax = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hRelax, 10, 249);
    gSize(hRelax, 340, 210);
    hGraph_DLTS = gCreateWindow(hInst, hwnd, WS_CHILD|WS_DLGFRAME|WS_VISIBLE);
    gPosition(hGraph_DLTS, 370, 249);
    gSize(hGraph_DLTS, 340, 210);
    /* Индикатор выполнения */
    hProgress = CreateWindow(PROGRESS_CLASS, NULL, WS_VISIBLE | WS_CHILD, 733, 195, 140, 20, hwnd, (HMENU)ID_PROGRESS, hInst, NULL);
        SendMessage(hProgress, PBM_SETRANGE, 0, 100 << 16);
        SendMessage(hProgress, PBM_SETPOS, 0, 0);
    /* Таймеры */
    SetTimer(hwnd, MAIN_TIMER, REFRESH_TIME, NULL);
    SetTimer(hwnd, DAQ_TIMER, REFRESH_TIME, NULL);
    SetTimer(hwnd, THERMOSTAT_TIMER, REFRESH_TIME, NULL);
    return TRUE;
}

void MainWindow_OnCommand(HWND hwnd, int id, HWND, UINT)
{
    static HANDLE thDownload = nullptr, /* Описатель потока для загрузки */
           thSave = nullptr,            /* Описатель потока для сохранения */
           thSettings = nullptr,        /* Описатель потока для окна настроек */
           thArrhenius = nullptr;       /* Описатель потока для окна графика Аррениуса */
    switch(id)
    {
        case ID_BUTTON_PREVIOUS_AI_PORT:
            if(offset_ai_port == 0)
                break;
            else
            {
                EnterCriticalSection(&csGlobalVariable);
                offset_ai_port--;
                LeaveCriticalSection(&csGlobalVariable);
            }
            break;
        case ID_BUTTON_NEXT_AI_PORT:
            if(offset_ai_port == 1)
                break;
            else
            {
                EnterCriticalSection(&csGlobalVariable);
                offset_ai_port++;
                LeaveCriticalSection(&csGlobalVariable);
            }
            break;
        /* График Аррениуса */
        case ID_BUTTON_GET_ARRHENIUS_GRAPH:
            thCreateWindow(thArrhenius, ID_ARRHENIUS_WINDOW);
            break;
        break;
        /* Переключение отображаемых в окне релаксаций */
        case ID_BUTTON_PREVIOUS_RELAXATION:
            if(index_relax == 0 || SavedRelaxations.empty())
                break;
            else index_relax--;
            PlotRelax();
            break;
        case ID_BUTTON_NEXT_RELAXATION:
            if(index_relax == SavedRelaxations.size()-1 || SavedRelaxations.empty())
                break;
            else index_relax++;
            PlotRelax();
            break;
        /* Сохранить файл */
        case ID_BUTTON_SAVE:
        if(thSave == nullptr || WaitForSingleObject(thSave, 0) == WAIT_OBJECT_0)
                thSave = (HANDLE)_beginthreadex(nullptr, 0, SaveFile, 0, 0, nullptr);
            break;
        /* Загрузить файл */
        case ID_BUTTON_LOAD:
            if(thDownload == nullptr || WaitForSingleObject(thDownload, 0) == WAIT_OBJECT_0)
                thDownload = (HANDLE)_beginthreadex(nullptr, 0, DownloadFile, 0, 0, nullptr);
            break;
        /* Настройки */
        case ID_BUTTON_SETTINGS:
            thCreateWindow(thSettings, ID_SETTINGS_WINDOW);
            break;
        case ID_CHECKBOX_FIX_TEMPERATURE:
            {
                static bool fix_temp = false;
                stringstream buff;
                /* Активирован флаг фиксации температуры */
                if(fix_temp == false)
                {
                    double CurrentTemperature = 0.0;
                    Thermostat.Write("CDAT?");
                    Thermostat.ReadDigit(CurrentTemperature);
                    /* Установка сетпоинта для термостата как текущую температуру */
                    rewrite(buff) << "SETP " << CurrentTemperature << "K";
                    Thermostat.Write(buff);
                    /* Установка RANGE в соответствии с текущей зоной */
                    rewrite(buff) << "RANG " << ZoneTable.GetActuallyHeatRange();
                    Thermostat.Write(buff);
                }
                else
                {
                    /* Установка сетпоинта в соответствии с настройками */
                    rewrite(buff) << "SETP " << Thermostat.SetPoint << "K";
                    Thermostat.Write(buff);
                    /* Установка RANGE в соответствии с текущей зоной */
                    rewrite(buff) << "RANG " << ZoneTable.GetActuallyHeatRange();
                    Thermostat.Write(buff);
                }
                fix_temp = !fix_temp;
            }
            break;
        /* Старт/стоп */
        case ID_BUTTON_START:
            if(start == false && !Thermostat.range_is_correct())
            {
                MessageBox(hwnd, "Invalid set point or end point value.", "Warning", MB_ICONWARNING);
                break;
            }
            StartButPush(hwnd);
            break;
        /* Установка начального и конечного значений температуры */
        case ID_BUTTON_SET:
            {
                double BeginPoint = 0.0, EndPoint = 0.0;
                /* Считываем начальное значение температуры из поля ввода */
                if(EmptyEditBox(hwnd, ID_EDITCONTROL_BEGIN))
                    BeginPoint = Thermostat.SetPoint;
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
                    Thermostat.SetPoint = BeginPoint;
                    Thermostat.EndPoint = EndPoint;
                    break;
                }
                break;
            }
        /* Выход */
        case ID_BUTTON_EXIT:
            DestroyWindow(hwnd);
            break;
    }
}

VOID MainWindow_OnTimer(HWND hwnd, UINT id)
{
    static HANDLE thDataAcquisition = nullptr, //Описатель потока для чтения с DAQ по таймеру
        thThermostatAcquisition = nullptr; //Описатель потока для чтения с термостата по таймеру
    switch(id)
    {
        case MAIN_TIMER:
            if(start)
            {
                stringstream buff;
                buff << setprecision(2) << fixed;
                double Dispersion = 0.0, Mean = 0.0;
                if((index_mode == DLTS && Thermostat.SetPoint > Thermostat.EndPoint) ||
                   (index_mode == ITS && Generator.begin_voltage > Generator.end_voltage))
                StartButPush(hwnd);
                //Проверить стабилизацию и запустить второй поток для считывания данных
                else if(stability == false)
                {
                    /* Рассчитываем и выводим Mean-square error */
                    Dispersion = AverSqFluct(Temperature);
                    rewrite(buff) << "Mean-squared error " << Dispersion << " K";
                    SetDlgItemText(hwnd, ID_STR_DISPERSION, buff.str().data());
                    /* Проверяем условие стабилизации первое */
                    if(Dispersion <= Thermostat.TempDisp)
                    {
                        Mean = mean(Temperature);
                        /* Проверяем условие стабилизации первое второе */
                        if(fabs(Mean - Thermostat.SetPoint) <= Thermostat.TempDisp)
                        {
                            stability = true;
                            hThread_DAQ = (HANDLE)_beginthreadex(nullptr, 0, ReadAverDAQ, &Mean, 0, nullptr);
                        }
                    }
                }
            }
            break;
        case DAQ_TIMER:
            if(thDataAcquisition == nullptr || WaitForSingleObject(thDataAcquisition, 0) == WAIT_OBJECT_0)
                thDataAcquisition = (HANDLE)_beginthreadex(nullptr, 0, DataAcquisition_Process, 0, 0, nullptr);
            break;
        case THERMOSTAT_TIMER:
            if(thThermostatAcquisition == nullptr || WaitForSingleObject(thThermostatAcquisition, 0) == WAIT_OBJECT_0)
                thThermostatAcquisition = (HANDLE)_beginthreadex(nullptr, 0, Thermostat_Process, 0, 0, nullptr);
            break;
    }
}

/* Считываем данные с аналогового входа */
UINT CALLBACK DataAcquisition_Process(void*)
{
    EnterCriticalSection(&csGlobalVariable);

    EnterCriticalSection(&csDataAcquisition);
    DAQmxReadAnalog(id_DAQ, ai_port+offset_ai_port, pfi_ttl_port,
                    rate_DAQ, gate_DAQ, DAQmx_Val_Rising, index_range, measure_time_DAQ,
                    &SignalDAQ);
    LeaveCriticalSection(&csDataAcquisition);
    if(ai_port+offset_ai_port == ai_port_pulse)
    {
        gVector vData;
        EnterCriticalSection(&csDataAcquisition);
        DAQmxReadAnalog(id_DAQ, ai_port_pulse, pfi_ttl_port,
                    rate_DAQ, gate_DAQ, DAQmx_Val_Rising, index_range, measure_time_DAQ,
                    &vData);
        LeaveCriticalSection(&csDataAcquisition);
        /* Рассчитываем истинные значения амплитуд */
        double High = 0.0, Low = 0.0;
        double N = rate_DAQ/1000.0*Generator.period, /* Число сэмплов */
            SSW = rate_DAQ/1000.0*Generator.width/4; /* Четверть импульса в сэмплах*/
        /* Пробегаем по точка до импульса */
        for(int i = 0; i < N/2-SSW; i++)
            High += vData[i];
        /* Пробегаем по точка после импульса */
        for(int i = N/2+4*SSW+SSW; i < N; i++)
            High += vData[i];
        High /= N/2-SSW + (N - (N/2+4*SSW+SSW));
        /* Пробегаем по точка соответствующим импульсу */
        for(int i = N/2+SSW; i < N/2+3*SSW; i++)
            Low += vData[i];
        Low /= 2*SSW;
        stringstream buff;
        buff << setprecision(3) << "[" << Low << "," << High << "]";
        gAdditionalInfo(hGraph_DAQ, buff.str());
    }
    LeaveCriticalSection(&csGlobalVariable);
    plotDAQ();
    return 0;
}

/* Считываени данных с термостата и обновление информации */
UINT CALLBACK Thermostat_Process(void*)
{
    double CurrentTemperature = 0.0, HeatPercent = 0.0, SetPoint = 0.0;
    stringstream buff;
    buff << setprecision(2) << fixed;
    /* Получаем значение текущей температуры */
    Thermostat.Write("CDAT?");
    Thermostat.ReadDigit(CurrentTemperature);
    #ifdef TEST_MODE
        srand(time(NULL));
        CurrentTemperature = rand()%20+300; //Для проверки
    #endif
    //EnterCriticalSection(&csPlotTempInRealTime);
    /* Сохраняем полученное значение */
    Temperature.push_back(CurrentTemperature);
    /* Удаляем старое значение */
    if(Temperature.size() > MAX_POINT_TEMP_GRAPH)
        Temperature.erase(Temperature.begin());
    //LeaveCriticalSection(&csPlotTempInRealTime);
    /* Мощность ТЭНа в процентах */
    Thermostat.Write("HEAT?");
    Thermostat.ReadDigit(HeatPercent);
    rewrite(buff) << "Power " << HeatPercent << "%";
    SetDlgItemText(hMainWindow, ID_STR_POWER, buff.str().data());
    /* Текущая температура */
    rewrite(buff) << "Temperature " << CurrentTemperature << " K ";
    SetDlgItemText(hMainWindow, ID_STR_CHANNEL_A, buff.str().data());
    /* Выводим текущее значение SetPoint */
    Thermostat.Write("SETP?");
    Thermostat.ReadDigit(SetPoint);
    rewrite(buff) << "Set point " << SetPoint << " K";
    SetDlgItemText(hMainWindow, ID_STR_SETPOINT, buff.str().data());
    /* Обновляем изображение графика */
    gVector vData1;
    for(size_t i = 0; i < Temperature.size(); i++)
        vData1.push_back(i*REFRESH_TIME*0.001);
    gData(hGraph, &vData1, &Temperature);
    return 0;
}

UINT CALLBACK ReadAverDAQ(void* mean_temperature)
{
    double MeanTemp = *((double*)mean_temperature);
    gVector vData, vResult;
    vData.reserve(samples_DAQ);
    vResult.reserve(samples_DAQ);
    /* В случае ITS режима, цикл ниже повторяется */
    do
    {
        int progress = 0;
        gVector(samples_DAQ, 0).swap(vResult);
        for(uInt32 i = 0; i < averaging_DAQ; i++)
        {
            /* Запись данных */
            EnterCriticalSection(&csDataAcquisition);
            DAQmxReadAnalog(id_DAQ, ai_port, pfi_ttl_port,
                    rate_DAQ, gate_DAQ, DAQmx_Val_Rising, index_range, measure_time_DAQ,
                    &vData);
            LeaveCriticalSection(&csDataAcquisition);
            /* Сохранение данных */
            for(auto it = vData.begin(), resit = vResult.begin(); it != vData.end(); it++, resit++)
                *resit += *it;
            /* Обновление информации о ходе записи */
            progress = 99 * i/(averaging_DAQ);
            SendMessage(hProgress, PBM_SETPOS, progress, 0);
        }
        EnterCriticalSection(&csGlobalVariable);
            gVector(samples_DAQ, 0).swap(Relaxation);
            for(auto it = vResult.begin(), resit = Relaxation.begin(); it != vResult.end(); it++, resit++)
                *resit = (*it)/averaging_DAQ;
            //Сохраняем релаксации, чтобы иметь возможность переключаться между ними
            SavedRelaxations.push_back(Relaxation);
            index_relax = SavedRelaxations.size() - 1; //без единицы, потому что индекс
        LeaveCriticalSection(&csGlobalVariable);
        if(index_mode == DLTS)
        {
            AddPointsDLTS(MeanTemp);                    //Добавляем точку на DLTS
            prepare_next_set_point();
        }
        else if(index_mode == ITS)
        {
            /* Вычисляем истинные параметры заполняющего импульса */
            Generator.mesure_pulse();
            if(Generator.begin_voltage + Generator.step_voltage > Generator.end_voltage)
                break; /* Выход из цикла while */
            Generator.begin_voltage += Generator.step_voltage;
            Generator.Apply();
        }
        SaveRelaxSignal(MeanTemp, vResult);          //Сохраняем результат в файл
        stability = false;
        SendMessage(hProgress, PBM_SETPOS, 0, 0);   //Очищаем Progress Bar
        PlotRelax();
        PlotDLTS();
        InvalidateRect((HWND)hGraph_Arrhenius, NULL, FALSE);
    }while(index_mode == ITS);
    return 0;
}

