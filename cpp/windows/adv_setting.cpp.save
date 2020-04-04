#include <winfunc.h>

//Оконная функция диалогового окна расширенных настроек
BOOL CALLBACK asdlg_proc(HWND hAdvSetDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static stringstream buff;
    switch(uMsg)
        {
            case WM_INITDIALOG:
            {
                //Вывод информации при инициализации
                buff << setprecision(2) << fixed;
                //Вывести текущие настройки DAQ
                rewrite(buff) << id_DAQ;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_DAQ, buff.str().data());
                //Вывести текущие настройки LakeShore
                rewrite(buff) << Thermostat.gpib;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_LAKESHORE, buff.str().data());
                //Вывести текущие настройки генератора
                rewrite(buff) << Generator.gpib;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_GENERATOR, buff.str().data());
                //Вывести текущие настройки пора аналогового входа
                rewrite(buff) << ai_port_measurement;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_AI, buff.str().data());
                //Вывести текущие настройки порта ТТЛ сигнала
                rewrite(buff) << pfi_ttl_port;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_TTL, buff.str().data());
                //Вывести текущие настройки пора аналогового входа импульсов
                rewrite(buff) << ai_port_pulse;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_AI_PULSE, buff.str().data());
                //Вывести текущие настройки канала выхода генератора
                rewrite(buff) << Generator.curr_channel;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_OUT_CHANNEL, buff.str().data());
                //Вывести текущие настройки пора аналогового входа емкости
                rewrite(buff) << ai_port_capacity;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_AI_CAPACITY, buff.str().data());
            }
            return TRUE;
            case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case ID_BUTTON_CLOSE_SETTINGS:
                        EndDialog(hAdvSetDlg, 0);
                    break;
                    case ID_BUTTON_APPLY_SETTINGS:
                        if(start.load())
                        {
                            MessageBox(hAdvSetDlg, "Stop the experiment and try again.", "Warning", MB_ICONWARNING);
                            return TRUE;
                        }
                        //Применить настройки DAQ
                        id_DAQ = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_DAQ);
                        //Применить настройки LakeShore
                        Thermostat.gpib = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_LAKESHORE);
                        //Применить настройки генератора
                        Generator.gpib = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_GENERATOR);
                        //Применить настройки пора аналогового входа
                        ai_port_measurement = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_AI);
                        //Применить настройки порта ТТЛ сигнала
                        pfi_ttl_port = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_TTL);
                        //Применить настройки пора аналогового входа импульсов
                        ai_port_pulse = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_AI_PULSE);
                        //Вывести текущие настройки канала выхода генератора
                        Generator.curr_channel = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_OUT_CHANNEL);
                        //Применить настройки пора аналогового входа емкости
                        ai_port_capacity = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_AI_CAPACITY);
                        //Применить настройки к физическим устройствам и сохранить файл настроек
                        ApplySettings();
                        write_settings();
                    break;
                }
                return TRUE;
            }
        }
        return FALSE;
}
