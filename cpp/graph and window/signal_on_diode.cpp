#include <winfunc.h>
#include <vector>
#include <graph.h>
#include <sstream>
#include <iomanip>
#include "gwin.h"
#include "daq.h"
#include "variable.h"

BOOL SignalOnDiode_OnCreate(HWND, LPCREATESTRUCT);
void SignalOnDiode_OnTimer(HWND, UINT);

unsigned __stdcall SignalOnDiode(void*)
{
    MSG messages;
    gwin::gCreateWindow(hInst, hMainWindow, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    gwin::gPosition(hWnd, 0, 0);
    gwin::gSize(hWnd, 350, 250);
    while(GetMessage(&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
    return 0;
}

/* Процедура окна отображения сигнала, посылаемого на диод-образец */
LRESULT CALLBACK sswin_proc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        HANDLE_MSG(hWnd, WM_CREATE, SignalOnDiode_OnCreate);
        HANDLE_MSG(hWnd, WM_TIMER, SignalOnDiode_OnTimer);
        case WM_SIZE:
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL SignalOnDiode_OnCreate(HWND hWindow, LPCREATESTRUCT)
{
    /* Создаем новый инструмент DAQ */
    /* Измеряем с первого A0 со скоростью 100000 в течение периода генератора в мс, без триггера */
    //if(!DataAcquisitionInst.InitSession(id_DAQ, 0, FREQUENCY, Generator.period, true, 0, DAQmx_Val_Falling))
    //{
        //DestroyWindow(hWindow);
        //return TRUE;
    //}
    //DataAcquisitionInst.GetTimeAxis(xAxisSignal);
    /* Таймер обновления окна */
    SetTimer(hWindow, 0, REFRESH_TIME_DAQ, NULL);
    return TRUE;
}

/* Считываение данных и обновление по таймеру */
void SignalOnDiode_OnTimer(HWND hWnd, UINT)
{
    const double dRate = 200000;
    //int32 read = 0; /* Считыно точек по факту */
    //if(!DataAcquisitionInst.ReadAnalog(yAxisSignal, &read) || read == 0)
        //return;
    vector<double> vData;
    DAQmxReadAnalog(id_DAQ, ai_port_pulse, -1,
                    dRate, 0, DAQmx_Val_Rising, index_range, measure_time_DAQ,
                    &vData);
    /* Рассчитываем истинные значения амплитуд */
    double High = 0.0, Low = 0.0;
    double N = FREQUENCY/1000.0*Generator.period, /* Число сэмплов */
        SSW = FREQUENCY/1000.0*Generator.width/4; /* Четверть импульса в сэмплах*/
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

    gwin::gData(hWnd, &xAxisSignal, &vData);
    //InvalidateRect(hWindow, NULL, FALSE);
}

/*void SignalOnDiode_OnPaint(HWND hWindow)
{
    std::stringstream buff;
    buff << fixed << setprecision(3);
    HDC hdc;
    PAINTSTRUCT ps;
    WINDOWINFO sWindowInfo;
    static HPEN hline = CreatePen(PS_SOLID, 1, RGB(150, 0, 200));
    int sx, sy;
    static double max_y, min_y, min_x, max_x;
    // Предустановки
    GetWindowInfo(hWindow, &sWindowInfo);
    sx = sWindowInfo.rcClient.right - sWindowInfo.rcClient.left;
    sy = sWindowInfo.rcClient.bottom - sWindowInfo.rcClient.top;
    //Перерисовка графика
    hdc = BeginPaint(hWindow, &ps);
        print_graph(xAxisSignal, 1, yAxisSignal, 3, hdc, hline, min_y, max_y, min_x, max_x, sx, sy, 5, 5);
        buff << VoltInfo.High<< " V";
        TextOut(hdc, GRAPHWIDTH, GRAPHSIZE/2, buff.str().data(), buff.str().size());
        rewrite(buff) << VoltInfo.Low << " V";
        TextOut(hdc, GRAPHWIDTH, GRAPHSIZE/2 - GRAPHSIZE/10, buff.str().data(), buff.str().size());
    EndPaint(hWindow, &ps);
}*/
