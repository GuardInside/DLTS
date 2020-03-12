#include <winfunc.h>
#define RANGE_HEATING_NUM          4   /* Число уровней нагрева ТЭН'а */

//Оконная функция диалогового окна настроек PID
BOOL CALLBACK pidlg_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static stringstream buff;
    static HWND hRange[10];
    switch(uMsg)
        {
            case WM_INITDIALOG:
            {
                //Вывод информации при инициализации
                buff << setprecision(2) << fixed;
                //Вывести текущие настройки всех зон
                for(int i = 0; i < QUANTITY_ZONE; i++)
                {
                    //Вывести текущие настройки верхней границы
                    rewrite(buff) << Thermostat.table[i].upper_boundary;
                    SetDlgItemText(hDlg, GetResPidTable(i, "UPPER_BOUNDARY"), buff.str().data());
                    //Вывести текущие настройки атрибута P
                    rewrite(buff) << Thermostat.table[i].P;
                    SetDlgItemText(hDlg, GetResPidTable(i, "P"), buff.str().data());
                    //Вывести текущие настройки атрибута I
                    rewrite(buff) << Thermostat.table[i].I;
                    SetDlgItemText(hDlg, GetResPidTable(i, "I"), buff.str().data());
                    //Вывести текущие настройки атрибута D
                    rewrite(buff) << Thermostat.table[i].D;
                    SetDlgItemText(hDlg, GetResPidTable(i, "D"), buff.str().data());
                    //Вывести текущие настройки уровня нагрева
                    hRange[i] = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                                            435, 349-(i*32.5), 80, 120, hDlg, NULL, hInst, NULL);
                    for (int j = 0; j < RANGE_HEATING_NUM; j++)
                        SendMessage(hRange[i], CB_ADDSTRING, 0, (LPARAM)strHeatingRange[j].data());
                    SendMessage(hRange[i], CB_SETCURSEL, static_cast<int>(Thermostat.table[i].range), 0);
                }
                return TRUE;
            }
            case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case ID_BUTTON_CLOSE_SETTINGS:
                        EndDialog(hDlg, 0);
                    break;
                    case ID_BUTTON_APPLY_SETTINGS:
                        if(start.load())
                        {
                            MessageBox(hDlg, "Stop the experiment and try again.", "Warning", MB_ICONWARNING);
                            return TRUE;
                        }
                        //Применить настройки зон
                        for(int i = 0; i < QUANTITY_ZONE; i++)
                        {
                            Thermostat.table[i].upper_boundary = ApplySettingEditBox(hDlg, GetResPidTable(i, "UPPER_BOUNDARY"));
                            Thermostat.table[i].P = ApplySettingEditBox(hDlg, GetResPidTable(i, "P"));
                            Thermostat.table[i].I = ApplySettingEditBox(hDlg, GetResPidTable(i, "I"));
                            Thermostat.table[i].D = ApplySettingEditBox(hDlg, GetResPidTable(i, "D"));
                            /* Установка уровеня мощности зоны */
                            Thermostat.table[i].range = static_cast<heatlvl>(SendMessage(hRange[i], CB_GETCURSEL, 0, 0));
                        }
                        Thermostat.ApplyZones();
                        write_settings();
                    break;
                }
                return TRUE;
            }
        }
        return FALSE;
}
