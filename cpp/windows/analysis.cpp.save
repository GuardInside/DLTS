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
    buff << bspline::order;
    SetDlgItemText(hWnd, ID_EDITCONTROL_BSPLINE_ORDER, buff.str().data());
    //Вывести текущие настройки абсолютной ошибки
    rewrite(buff) << scientific;
    rewrite(buff) << bspline::ncoeffs;
    SetDlgItemText(hWnd, ID_EDITCONTROL_BSPLINE_NCOEFFS, buff.str().data());
    //Вывести текущие настройки использования фиттинга
    CheckDlgButton(hWnd, ID_CHECKBOX_USE_FITTING_RELAX, bspline::enable);
    CheckDlgButton(hWnd, ID_CHECKBOX_USE_BSPLINE_DLTS, bspline::enable2);
    return TRUE;
}

BOOL Analysis_OnCommand(HWND hWnd, INT id, HWND, UINT)
{
    switch(id)
    {
        case ID_BUTTON_APPLY:
            {
                /* Установка режима аппроксимации */
                bool enable     = SendMessage(GetDlgItem(hWnd, ID_CHECKBOX_USE_FITTING_RELAX), BM_GETCHECK, 0, 0);
                bool enable2    = SendMessage(GetDlgItem(hWnd, ID_CHECKBOX_USE_BSPLINE_DLTS), BM_GETCHECK, 0, 0);
                /* Порядок сплайна */
                int order = ApplySettingEditBox(hWnd, ID_EDITCONTROL_BSPLINE_ORDER);
                /* Число коэффициентов */
                int ncoeffs = ApplySettingEditBox(hWnd, ID_EDITCONTROL_BSPLINE_NCOEFFS);

                if(enable || enable2)
                {
                    /* breakpoint = ncoeffs + 2 - spline order, */
                    if((ncoeffs + 2 - order) < 2)
                    {
                        MessageBox(hWnd, "Breakpoint = [ncoeffs + 2 - spline order]\nshould be more than one.", "Warning", MB_ICONWARNING);
                        return TRUE; /* Не сохраняем настройки */
                    }
                }
                /* Сохраняем настройки */
                bspline::order      = order;
                bspline::ncoeffs    = ncoeffs;
                bspline::enable     = enable;
                bspline::enable2    = enable2;
                /* Сохраняем настройки в файл */
                write_settings();
                /* Сообщение об успешном приминении настроек */
                CloseHandle((HANDLE)_beginthreadex(NULL, 0, dlg_success, NULL, 0, NULL));
                /* Пересчитываем и обновляем графики */
                SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_RELAX, 0);
                PostMessage(hMainWindow, WM_COMMAND, WM_REFRESH_DLTS, 0);
            }
            return TRUE;
        case ID_BUTTON_CLOSE:
            DestroyWindow(hWnd);
            return TRUE;
    }
    return FALSE;
}
