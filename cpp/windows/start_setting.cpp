#include "winfunc.h"

VOID StartWnd_OnShow(HWND, BOOL, UINT);
VOID StartWnd_OnCommand(HWND, int, HWND, UINT);

LRESULT CALLBACK dlwin_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hWnd, WM_SHOWWINDOW, StartWnd_OnShow);
        HANDLE_MSG(hWnd, WM_COMMAND, StartWnd_OnCommand);
        //HANDLE_MSG(hwnd, WM_TIMER, MainWindow_OnTimer);
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    }
    return 0;
}

VOID StartWnd_OnShow(HWND hWnd, BOOL, UINT)
{
    stringstream buff;
    rewrite(buff) << FileSaveName;
    SetDlgItemText(hWnd, ID_EDITCONTROL_SAVE_FILE_NAME, buff.str().data());
    CheckRadioButton(hWnd, ID_RADIO_APPEND, ID_RADIO_NEW, ID_RADIO_NEW);
}

VOID StartWnd_OnCommand(HWND hWnd, int id, HWND, UINT)
{
    static BOOL bfNewfile = 0, bfAppfile = -1;
    switch(id)
    {
        case ID_BUTTON_CONTINUE:
            ::bfNewfile = bfNewfile;
            ::bfAppfile = bfAppfile;
            StartButPush(hMainWindow);
        case ID_BUTTON_CANCEL:
            DestroyWindow(hWnd);
            break;
        case ID_RADIO_NEW:
        case ID_RADIO_APPEND:
            bfNewfile != bfNewfile;
            bfAppfile != bfAppfile;
            break;
    }
}

void StartButPush(HWND hwnd)
{
    /* Реализовать запрос на очистку DLTS-кривых и всех буферов перед */
    stringstream buff;
    if(start == false)
        buff << "Stop";
    else if(start == true)
        buff << "Start";
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_SET), start);
    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_END), start);
    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_BEGIN), start);
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_SETTINGS), start);
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_EXIT), start);
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_LOAD), start);
    SetDlgItemText(hwnd, ID_BUTTON_START, buff.str().data());
    if(bfNewfile)
    {
        /* Очистка буферов, если была произведена загрузка */
        if(loading == true)
        {
            loading = false;
            ClearMemmory();
            PlotRelax();
            PlotDLTS();
        }

    }
    if(start == false)
    {
        /* Установка флагов */
        start = true;
        if(index_mode == ITS) endofits = false;
        else if(index_mode == DLTS) endofdlts = false;
        /* Установка сетпоинта для термостата */
        rewrite(buff) << "SETP " << Thermostat.SetPoint << "K";
        Thermostat.Write(buff);
        /* Установка RANGE в соответствии с текущей зоной */
        rewrite(buff) << "RANG " << Thermostat.ZoneTable.GetActuallyHeatRange();
        Thermostat.Write(buff);
    }
    else if(start == true)
    {
        stability = false;
        start = false;
        if(index_mode == ITS) endofits = true;
        else if(index_mode == DLTS) endofdlts = true;
        /* Стоп нагрев в текущей зоне */
        Thermostat.Write("RANG 0");
    }
}

//StartButPush(hMainWindow);
