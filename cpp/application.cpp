#include "mainwindow.h"
#include "resource.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <iostream>

using std::cout;
using std::cin;
using std::endl;

Application::Application(HINSTANCE hInst) :
    hInstance(hInst),
    hMainWindow(NULL),
    hPlot1(NULL), hPlot2(NULL), hPlot3(NULL), hPlot4(NULL)
{
}

Application::~Application()
{
}

void Application::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        //if(!IsDialogMessage(hSettinWnd, &msg) && !IsDialogMessage(hAnalysisWnd, &msg))
        {
            /* Перехват сообщений о действиях мышки для окна DLTS графика */
            /* Сообщение из очереди удаляется стандартным обработчиком */
            /*if(index_plot_DLTS == 0 && msg.hwnd == hGraph_DLTS)
                dlts_mouse_message(msg.hwnd, msg.message, msg.wParam, msg.lParam);
            else if(msg.hwnd == hRelax)
                relax_mouse_message(msg.hwnd, msg.message, msg.wParam, msg.lParam);
            else if(msg.hwnd == hGraph_DAQ)
                daq_mouse_message(msg.hwnd, msg.message, msg.wParam, msg.lParam);*/

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

HRESULT Application::Initialize()
{
    HRESULT hr = S_OK;

    InitCommonControls();

    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    wcex.cbWndExtra = DLGWINDOWEXTRA;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszClassName = L"main window class";

    RegisterClassExW(&wcex);

    //LoadSettings();
    //ApplySettings();

    //hDownloadEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    /* Создаем окно и переходим в цикл обработки сообщений */
    hMainWindow = CreateDialog(hInstance, MAKEINTRESOURCE(ID_MAIN_WINDOW), HWND_DESKTOP, NULL);

    hr = hMainWindow ? S_OK : E_FAIL;
    if(SUCCEEDED(hr))
    {
        ShowWindow(hMainWindow, SW_NORMAL);
        UpdateWindow(hMainWindow);
    }
    return hr;
}

LRESULT CALLBACK Application::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        //HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        //HANDLE_MSG(hwnd, WM_TIMER, OnTimer);
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}


