#include "winfunc.h"
#include "ini.h"

BOOL Analysis_OnInit(HWND, HWND, LPARAM);
BOOL Analysis_OnCommand(HWND, INT, HWND, UINT);

INT_PTR CALLBACK anwin_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, Analysis_OnInit);
        HANDLE_MSG(hWnd, WM_COMMAND, Analysis_OnCommand);
    }
    return FALSE;
}

BOOL Analysis_OnInit(HWND hWnd, HWND, LPARAM)
{
    stringstream buff;
    buff << setprecision(0);
    /* Вывод информации при инициализации */
    buff << AprIter;
    SetDlgItemText(hWnd, ID_EDITCONTROL_ITERATION, buff.str().data());
    //Вывести текущие настройки абсолютной ошибки
    rewrite(buff) << scientific;
    rewrite(buff) << AprErr;
    SetDlgItemText(hWnd, ID_EDITCONTROL_ABS_ERROR, buff.str().data());
    //Вывести текущие настройки использования фиттинга
    CheckDlgButton(hWnd, ID_CHECKBOX_USE_FITTING_RELAX, AprEnableRelax);
    CheckDlgButton(hWnd, ID_CHECKBOX_USE_FITTING_DLTS, AprEnableDLTS);
    return TRUE;
}

BOOL Analysis_OnCommand(HWND hWnd, INT id, HWND, UINT)
{
    switch(id)
    {
        case ID_BUTTON_APPLY:
            {
                bool prevEnableRelax = ::AprEnableRelax;
                bool prevEnableDLTS = ::AprEnableDLTS;
                /* Предельное число итераций */
                AprIter = ApplySettingEditBox(hWnd, ID_EDITCONTROL_ITERATION);
                /* Абсолютная погрешность */
                stringstream buff;
                buff << setprecision(0) << scientific;
                GetDlgItemTextMod(hWnd, ID_EDITCONTROL_ABS_ERROR, buff);
                AprErr = atof(buff.str().data());
                /* Установка режима аппроксимации */
                AprEnableRelax = SendMessage(GetDlgItem(hWnd, ID_CHECKBOX_USE_FITTING_RELAX), BM_GETCHECK, 0, 0);
                AprEnableDLTS = SendMessage(GetDlgItem(hWnd, ID_CHECKBOX_USE_FITTING_DLTS), BM_GETCHECK, 0, 0);

                write_settings();
                /* Сообщение об успешном приминении настроек */
                HANDLE hThreadSuccess = (HANDLE)_beginthreadex(NULL, 0, dlg_success, NULL, 0, NULL);
                CloseHandle(hThreadSuccess);
                /* Пересчитываем и обновляем графики */
                if(prevEnableRelax != AprEnableRelax)
                    SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_RELAX, 0);
                if(prevEnableDLTS != AprEnableDLTS)
                    SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
            }
            return TRUE;
        case ID_BUTTON_CLOSE:
            DestroyWindow(hWnd);
            return TRUE;
    }
    return FALSE;
}
