#include <winfunc.h>
#define ID_COMBOBOX_VOLTAGE_RANGE 200
#define ID_COMBOBOX_MODE          201
#define WM_INITDLG                WM_USER

BOOL SettingsWindow_OnCreate(HWND, LPCREATESTRUCT);
VOID SettingsWindow_OnCommand(HWND, int, HWND, UINT);

/* Оконная функция окна настроек */
LRESULT CALLBACK stwin_proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hwnd, WM_CREATE, SettingsWindow_OnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND, SettingsWindow_OnCommand);
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}


BOOL SettingsWindow_OnCreate(HWND hwnd, LPCREATESTRUCT)
{
    HWND hComboBox_range, hComboBox_mode;
    //Вывести возможны настройки диапазонов напряжений
    hComboBox_range = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
    167, 63, 95, 125, hwnd, (HMENU)(ID_COMBOBOX_VOLTAGE_RANGE), hInst, NULL);
    for (int i = 0; i < RANGE_SIZE; i++)
        SendMessage(hComboBox_range, CB_ADDSTRING, 0, (LPARAM)range[i].data());
    SendMessage(hComboBox_range, CB_SETCURSEL, index_range, 0);
    /* Настройка режима */
    hComboBox_mode = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
    555, 120, 100, 125, hwnd, (HMENU)(ID_COMBOBOX_MODE), hInst, NULL);
        SendMessage(hComboBox_mode, CB_ADDSTRING, 0, (LPARAM)"DLTS");
        SendMessage(hComboBox_mode, CB_ADDSTRING, 0, (LPARAM)"ITS");
    SendMessage(hComboBox_mode, CB_SETCURSEL, index_mode, 0);
    /* Вывод значений в эдитбокс */
    PostMessage(hwnd, WM_COMMAND, WM_INITDLG, 0);
    return TRUE;
}

void SettingsWindow_OnCommand(HWND hwnd, int ID, HWND, UINT codeNotify)
{
    switch(ID)
    {
        case WM_INITDLG:
            {
                stringstream buff;
                buff << setprecision(2) << fixed;
                /* Вывод информации при инициализации */
                //Вывести текущие настройки шага стабилизации
                rewrite(buff) << Thermostat.TempStep;
                SetDlgItemText(hwnd, ID_EDITCONTROL_STEP, buff.str().data());
                //Вывести текущие настройки допустимого разброса
                rewrite(buff) << Thermostat.TempDisp;
                SetDlgItemText(hwnd, ID_EDITCONTROL_DISPERSION, buff.str().data());
                //Вывести текущие настройки времени усреднения
                rewrite(buff) << aver_time;
                SetDlgItemText(hwnd, ID_EDITCONTROL_AVERAGING_TIME, buff.str().data());
                //Вывести текущие настройки сэмплов
                rewrite(buff) << measure_time_DAQ;
                SetDlgItemText(hwnd, ID_EDITCONTROL_MEASURE_TIME, buff.str().data());
                //Вывести текущие настройки скорости чтения DQA
                rewrite(buff) << setprecision(0) << rate_DAQ << setprecision(2);
                SetDlgItemText(hwnd, ID_EDITCONTROL_RATE, buff.str().data());
                //Вывести текущие настройки усреднения считанных данных DAQ
                rewrite(buff) << averaging_DAQ;
                SetDlgItemText(hwnd, ID_EDITCONTROL_AVERAGING, buff.str().data());
                //Вывести текущие настройки длительности ворот
                rewrite(buff) << gate_DAQ;
                SetDlgItemText(hwnd, ID_EDITCONTROL_GATE, buff.str().data());
                //Вывести текущие настройки имени файла сохранений
                rewrite(buff) << FileSaveName;
                SetDlgItemText(hwnd, ID_EDITCONTROL_FILE_SAVE_NAME, buff.str().data());
                /* Настройки генератора импульсов */
                //Вывести текущие настройки периода
                rewrite(buff) << Generator.period;
                SetDlgItemText(hwnd, ID_EDITCONTROL_PERIOD, buff.str().data());
                //Вывести текущие настройки ширины импульсов
                rewrite(buff) << Generator.width;
                SetDlgItemText(hwnd, ID_EDITCONTROL_WIDTHPULSE, buff.str().data());
                //Вывести текущие настройки верхней границы импульса в вольтах
                rewrite(buff) << setprecision(3);
                rewrite(buff) << Generator.voltage_up;
                SetDlgItemText(hwnd, ID_EDITCONTROL_UPPERBOUNDERY, buff.str().data());
                //Вывести текущие настройки нижней границы импульса в вольтах
                rewrite(buff) << Generator.voltage_low;
                SetDlgItemText(hwnd, ID_EDITCONTROL_LOWERBOUNDERY, buff.str().data());
                //Вывести текущие настройки шага ITS в вольтах
                rewrite(buff) << Generator.step_voltage;
                SetDlgItemText(hwnd, ID_EDITCONTROL_STEP_VOLTAGE, buff.str().data());
                //Вывести текущие настройки старта ITS в вольтах
                rewrite(buff) << Generator.begin_voltage;
                SetDlgItemText(hwnd, ID_EDITCONTROL_BEGIN_VOLTAGE, buff.str().data());
                //Вывести текущие настройки конца ITS в вольтах
                rewrite(buff) << Generator.end_voltage;
                SetDlgItemText(hwnd, ID_EDITCONTROL_END_VOLTAGE, buff.str().data());
                rewrite(buff) << setprecision(2);
                //Вывести текущие настройки режима генератора (SULA\AGILENT)
                CheckDlgButton(hwnd, ID_CHECKBOX_USE_GENERATOR, Generator.is_active);
                if(!Generator.is_active)
                {
                    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_PERIOD), FALSE);
                    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_WIDTHPULSE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_UPPERBOUNDERY), FALSE);
                    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_LOWERBOUNDERY), FALSE);
                }
                /* Блокировка опций в зависимости от режима */
                BOOL state = TRUE;
                if(index_mode == ITS)
                    state = FALSE;
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_STEP),           state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_UPPERBOUNDERY),  state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_STEP_VOLTAGE),  !state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_BEGIN_VOLTAGE), !state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_END_VOLTAGE),   !state);
            }
        break;
        /* Показать окно с сигналом, идущим на образец */
        case ID_BUTTON_PID_SETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(ID_PID_SETTINGS_WINDOW), hwnd, (DLGPROC)pidlg_proc);
        break;
        case ID_BUTTON_ADVANCED_SETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(ID_ADVANCED_SETTINGS_WINDOW), hwnd, (DLGPROC)asdlg_proc);
        break;
        case ID_BUTTON_CORRELATION_SETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(ID_CORRELATION_SETTINGS_WINDOW), hwnd, (DLGPROC)csdlg_proc);
        break;
        case ID_BUTTON_CLOSE_SETTINGS:
            DestroyWindow(hwnd);
        break;
        /* Изменилось состояние чекбокса */
        case ID_CHECKBOX_USE_GENERATOR:
            {
                BOOL state = TRUE;
                if(Generator.is_active)
                    state = FALSE;
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_PERIOD), state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_WIDTHPULSE), state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_UPPERBOUNDERY), state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_LOWERBOUNDERY), state);
                Generator.is_active = state;
            }
        break;
        case ID_COMBOBOX_MODE:
            if(codeNotify == CBN_SELCHANGE)
            {
                size_t index_mode = SendMessage(GetDlgItem(hwnd, ID_COMBOBOX_MODE), CB_GETCURSEL, 0, 0);;
                BOOL state = TRUE;
                if(index_mode == ITS)
                    state = FALSE;
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_STEP),           state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_UPPERBOUNDERY),  state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_STEP_VOLTAGE),  !state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_BEGIN_VOLTAGE), !state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_END_VOLTAGE),   !state);
            }
            break;
        case ID_BUTTON_APPLY_SETTINGS:
            {
                bool alright = true;
                /* Проверка диапазонов введенных значений */
                /*if(ApplySettingEditBox(hwnd, ID_EDITCONTROL_GATE, 2) == 0.0)
                {
                    MessageBox(hwnd, "Gate should be more than 0.0", "Warning", MB_ICONWARNING);
                    break;
                }*/
                /* Общие настройки */
                //Применить настройки имени файла сохранений
                FileSaveName = ApplySettingEditBoxString(hwnd, ID_EDITCONTROL_FILE_SAVE_NAME);
                //Применить настройки режима работы программы
                index_mode = SendMessage(GetDlgItem(hwnd, ID_COMBOBOX_MODE), CB_GETCURSEL, 0, 0);
                /* Настройки LakeShore */
                //Применить настройки шага стабилизации
                Thermostat.TempStep = ApplySettingEditBox(hwnd, ID_EDITCONTROL_STEP, 2);
                //Применить настройки допустимого разброса
                Thermostat.TempDisp = ApplySettingEditBox(hwnd, ID_EDITCONTROL_DISPERSION, 2);
                //Применить настройки времени усреднения
                aver_time = ApplySettingEditBox(hwnd, ID_EDITCONTROL_AVERAGING_TIME);
                /* Настройки измерений */
                //Применить настройки времени измерения
                measure_time_DAQ = ApplySettingEditBox(hwnd, ID_EDITCONTROL_MEASURE_TIME);
                //Применить настройки скорости чтения DQA
                rate_DAQ = ApplySettingEditBox(hwnd, ID_EDITCONTROL_RATE);
                //Применить настройки усреднения считанных данных DAQ
                averaging_DAQ = ApplySettingEditBox(hwnd, ID_EDITCONTROL_AVERAGING);
                //Применить настройки длительности ворот
                gate_DAQ = ApplySettingEditBox(hwnd, ID_EDITCONTROL_GATE, 2);
                //Применить настройки диапазона напряжений
                index_range = SendMessage(GetDlgItem(hwnd, ID_COMBOBOX_VOLTAGE_RANGE), CB_GETCURSEL, 0, 0);
                /* Настройки генератора импульсов */
                //Применить настройки периода
                Generator.period = ApplySettingEditBox(hwnd, ID_EDITCONTROL_PERIOD, 2);
                //Вывести текущие настройки ширины импульсов
                Generator.width = ApplySettingEditBox(hwnd, ID_EDITCONTROL_WIDTHPULSE, 2);
                if(Generator.width >= Generator.period) alright = false;
                //Применить настройки верхней границы импульса в вольтах
                Generator.voltage_up = ApplySettingEditBox(hwnd, ID_EDITCONTROL_UPPERBOUNDERY, 3);
                if(Generator.voltage_up > MAX_VOLTAGE_PULSE || Generator.voltage_up < MIN_VOLTAGE_PULSE) alright = false;
                //Применить настройки нижней границы импульса в вольтах
                Generator.voltage_low = ApplySettingEditBox(hwnd, ID_EDITCONTROL_LOWERBOUNDERY, 3);
                if(Generator.voltage_low < MIN_VOLTAGE_PULSE || Generator.voltage_low > MAX_VOLTAGE_PULSE) alright = false;
                if(index_mode == ITS)
                {
                    //Применить настройки шага ITS в вольтах
                    Generator.step_voltage = ApplySettingEditBox(hwnd, ID_EDITCONTROL_STEP_VOLTAGE, 3);
                    //Применить настройки начала ITS в вольтах
                    Generator.begin_voltage = ApplySettingEditBox(hwnd, ID_EDITCONTROL_BEGIN_VOLTAGE, 3);
                    if(Generator.begin_voltage < MIN_VOLTAGE_PULSE || Generator.begin_voltage > MAX_VOLTAGE_PULSE || Generator.begin_voltage <= Generator.voltage_low) alright = false;
                    //Применить настройки конца ITS в вольтах
                    Generator.end_voltage = ApplySettingEditBox(hwnd, ID_EDITCONTROL_END_VOLTAGE, 3);
                    if(Generator.end_voltage > MAX_VOLTAGE_PULSE || Generator.end_voltage < MIN_VOLTAGE_PULSE) alright = false;
                }
                //Применить настройки к физическим устройствам и сохранить файл настроек при условии их корректности
                if(alright == true)
                {
                    ApplySettings();
                    write_settings();
                }
                else
                    MessageBox(hwnd, "Incorrect settings.\nYou should be more careful.", "Warning", MB_ICONWARNING);
            }
        break;
    }
}
