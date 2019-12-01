#include "winfunc.h"

BOOL StartWnd_OnInit(HWND, HWND, LPARAM);
BOOL StartWnd_OnCommand(HWND, int, HWND, UINT);

INT_PTR CALLBACK srwin_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, StartWnd_OnInit);
        HANDLE_MSG(hWnd, WM_COMMAND, StartWnd_OnCommand);
    }
    return FALSE;
}

BOOL StartWnd_OnInit(HWND hWnd, HWND, LPARAM)
{
    stringstream buff;
    rewrite(buff) << FileSaveName;
    SetDlgItemText(hWnd, ID_EDITCONTROL_SAVE_FILE_NAME, buff.str().data());
    CheckRadioButton(hWnd, ID_RADIO_APPEND, ID_RADIO_NEW, ID_RADIO_NEW);
    return TRUE;
}

BOOL StartWnd_OnCommand(HWND hWnd, int id, HWND, UINT)
{
    switch(id)
    {
        case ID_BUTTON_CONTINUE:
            if(IsDlgButtonChecked(hWnd, ID_RADIO_NEW) == BST_CHECKED)
                bfNewfile = true;
            else if(IsDlgButtonChecked(hWnd, ID_RADIO_APPEND) == BST_CHECKED)
                bfNewfile = false;
            //Применить настройки имени файла сохранений
            FileSaveName = ApplySettingEditBoxString(hWnd, ID_EDITCONTROL_SAVE_FILE_NAME);
            StartButPush();
        case ID_BUTTON_CANCEL:
            EndDialog(hWnd, 0);
            return TRUE;
    }
    return FALSE;
}

void StartButPush()
{
    /* Реализовать запрос на очистку DLTS-кривых и всех буферов перед */
    stringstream buff;
    if(start == false)
        buff << "Stop";
    else if(start == true)
        buff << "Start";
    EnableWindow(GetDlgItem(hMainWindow, ID_BUTTON_SET), start);
    EnableWindow(GetDlgItem(hMainWindow, ID_EDITCONTROL_END), start);
    EnableWindow(GetDlgItem(hMainWindow, ID_EDITCONTROL_BEGIN), start);
    EnableWindow(GetDlgItem(hMainWindow, ID_BUTTON_EXIT), start);
    SetDlgItemText(hMainWindow, ID_BUTTON_START, buff.str().data());
    if(start == false)
    {
        if(bfNewfile == true || index_mode == ITS)
        {
            /* Очищаем буфферы при запуске с флагом bfNewfilew */
            ClearMemmory();
        }
        else
        {
            HANDLE thDownload = (HANDLE)_beginthreadex(NULL, 0, LoadFile, NULL, 0, NULL);
            CloseHandle(thDownload);
        }
        /* Установка флагов */
        start = true;
        /* Установка сетпоинта для термостата */
        rewrite(buff) << "SETP " << Thermostat.BeginPoint << "K";
        Thermostat.Write(buff);
        /* Установка RANGE в соответствии с текущей зоной */
        rewrite(buff) << "RANG " << Thermostat.ZoneTable.GetActuallyHeatRange();
        Thermostat.Write(buff);
    }
    else if(start == true)
    {
        stability = false;
        start = false;
        /* Стоп нагрев в текущей зоне */
        Thermostat.Write("RANG 0");
    }
}
