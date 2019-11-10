#include <winfunc.h>

//Оконная функция диалогового окна изменения имени save-файла
BOOL CALLBACK rndlg_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static stringstream buff;
    switch(uMsg)
        {
            case WM_INITDIALOG:
            {
                //Вывести текущие настройки имени файла сохранений
                rewrite(buff) << FileSaveName;
                SetDlgItemText(hwnd, ID_EDITCONTROL_NEW_NAME, buff.str().data());
            }
            return TRUE;
            case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case ID_BUTTON_CONTINUE:
                        //Применить настройки имени файла сохранений
                        FileSaveName = ApplySettingEditBoxString(hwnd, ID_EDITCONTROL_NEW_NAME);
                        //Cохранить файл настроек
                        write_settings();
                        EndDialog(hwnd, 0);
                    break;
                }
                return TRUE;
            }
        }
        return FALSE;
}
