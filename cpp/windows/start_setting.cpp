#include "winfunc.h"

VOID StartWnd_OnShow(HWND, BOOL, UINT);
VOID StartWnd_OnCommand(HWND, int, HWND, UINT);

LRESULT CALLBACK dlwin_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hWnd, WM_SHOWWINDOW, StartWnd_OnShow);
        HANDLE_MSG(hWnd, WM_COMMAND, StartWnd_OnCommand);
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
    switch(id)
    {
        case ID_BUTTON_CONTINUE:
            if(IsDlgButtonChecked(hWnd, ID_RADIO_NEW) == BST_CHECKED)
            {
                bfNewfile = true;
                bfAppfile = false;
            }
            else if(IsDlgButtonChecked(hWnd, ID_RADIO_APPEND) == BST_CHECKED)
            {
                bfNewfile = false;
                bfAppfile = true;
            }
            //Применить настройки имени файла сохранений
            FileSaveName = ApplySettingEditBoxString(hWnd, ID_EDITCONTROL_SAVE_FILE_NAME);
            StartButPush(hMainWindow);
        case ID_BUTTON_CANCEL:
            DestroyWindow(hWnd);
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
    if(bfNewfile == true && start == false)
    {
        MessageBox(0,"clear","",0);
        /* Очистка буферов, если была произведена загрузка */
        ClearMemmory();
        gwin::gDefaultPlot(hRelax, "\0");
        gwin::gDefaultPlot(hGraph_DLTS, "\0");
    }
    if(bfAppfile == true && start == false)
    {
        string SavePath = Save + FileSaveName;
        if(index_mode == DLTS) SavePath += ".dlts";
        else if(index_mode == ITS) SavePath += ".its";
        //ClearMemmory();
        LoadFile(SavePath);
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
