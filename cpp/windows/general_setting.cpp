#include <winfunc.h>
#define ID_COMBOBOX_VOLTAGE_RANGE 200
#define ID_COMBOBOX_MODE          201
#define WM_INITDLG                WM_USER

BOOL SettingsWindow_OnInit(HWND, HWND, LPARAM);
BOOL SettingsWindow_OnCommand(HWND, int, HWND, UINT);

/* ������� ������� ���� �������� */
INT_PTR CALLBACK stwin_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, SettingsWindow_OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, SettingsWindow_OnCommand);
    }
    return FALSE;
}


//BOOL SettingsWindow_OnCreate(HWND hwnd, LPCREATESTRUCT)
BOOL SettingsWindow_OnInit(HWND hwnd, HWND, LPARAM)
{
    HWND hComboBox_range, hComboBox_mode;
    //������� �������� ��������� ���������� ����������
    hComboBox_range = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
    167, 63, 95, 125, hwnd, (HMENU)(ID_COMBOBOX_VOLTAGE_RANGE), hInst, NULL);
    for (int i = 0; i < RANGE_SIZE; i++)
        SendMessage(hComboBox_range, CB_ADDSTRING, 0, (LPARAM)range[i].data());
    SendMessage(hComboBox_range, CB_SETCURSEL, index_range, 0);
    /* ��������� ������ */
    hComboBox_mode = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
    555, 120, 100, 125, hwnd, (HMENU)(ID_COMBOBOX_MODE), hInst, NULL);
        SendMessage(hComboBox_mode, CB_ADDSTRING, 0, (LPARAM)"DLTS");
        SendMessage(hComboBox_mode, CB_ADDSTRING, 0, (LPARAM)"ITS");
    SendMessage(hComboBox_mode, CB_SETCURSEL, index_mode, 0);
    /* ����� �������� � �������� */
    PostMessage(hwnd, WM_COMMAND, WM_INITDLG, 0);
    return TRUE;
}

BOOL SettingsWindow_OnCommand(HWND hwnd, int ID, HWND, UINT codeNotify)
{
    switch(ID)
    {
        case WM_INITDLG:
            {
                stringstream buff;
                buff << setprecision(2) << fixed;
                /* ����� ���������� ��� ������������� */
                //������� ������� ��������� ���� ������������
                rewrite(buff) << Thermostat.TempStep;
                SetDlgItemText(hwnd, ID_EDITCONTROL_STEP, buff.str().data());
                //������� ������� ��������� ����������� ��������
                rewrite(buff) << Thermostat.TempDisp;
                SetDlgItemText(hwnd, ID_EDITCONTROL_DISPERSION, buff.str().data());
                //������� ������� ��������� ������� ����������
                rewrite(buff) << aver_time;
                SetDlgItemText(hwnd, ID_EDITCONTROL_AVERAGING_TIME, buff.str().data());
                //������� ������� ��������� �������
                rewrite(buff) << measure_time_DAQ;
                SetDlgItemText(hwnd, ID_EDITCONTROL_MEASURE_TIME, buff.str().data());
                //������� ������� ��������� �������� ������ DQA
                rewrite(buff) << setprecision(0) << rate_DAQ << setprecision(2);
                SetDlgItemText(hwnd, ID_EDITCONTROL_RATE, buff.str().data());
                //������� ������� ��������� ���������� ��������� ������ DAQ
                rewrite(buff) << averaging_DAQ;
                SetDlgItemText(hwnd, ID_EDITCONTROL_AVERAGING, buff.str().data());
                //������� ������� ��������� ������������ �����
                rewrite(buff) << gate_DAQ;
                SetDlgItemText(hwnd, ID_EDITCONTROL_GATE, buff.str().data());
                //������� ������� ��������� ����� ����� ����������
                rewrite(buff) << FileSaveName;
                SetDlgItemText(hwnd, ID_EDITCONTROL_FILE_SAVE_NAME, buff.str().data());
                /* ��������� ���������� ��������� */
                //������� ������� ��������� �������
                rewrite(buff) << Generator.period;
                SetDlgItemText(hwnd, ID_EDITCONTROL_PERIOD, buff.str().data());
                //������� ������� ��������� ������ ���������
                rewrite(buff) << Generator.width;
                SetDlgItemText(hwnd, ID_EDITCONTROL_WIDTHPULSE, buff.str().data());
                //������� ������� ��������� ��������� �������� � �������
                rewrite(buff) << setprecision(3);
                rewrite(buff) << Generator.amplitude;
                SetDlgItemText(hwnd, ID_EDITCONTROL_UPPERBOUNDERY, buff.str().data());
                //������� ������� ��������� �������� �������� � �������
                rewrite(buff) << Generator.bias;
                SetDlgItemText(hwnd, ID_EDITCONTROL_LOWERBOUNDERY, buff.str().data());
                //������� ������� ��������� ���� ITS � �������
                rewrite(buff) << Generator.step_voltage;
                SetDlgItemText(hwnd, ID_EDITCONTROL_STEP_VOLTAGE, buff.str().data());
                //������� ������� ��������� ������ ITS � �������
                rewrite(buff) << Generator.begin_amplitude;
                SetDlgItemText(hwnd, ID_EDITCONTROL_BEGIN_VOLTAGE, buff.str().data());
                //������� ������� ��������� ����� ITS � �������
                rewrite(buff) << Generator.end_amplitude;
                SetDlgItemText(hwnd, ID_EDITCONTROL_END_VOLTAGE, buff.str().data());
                rewrite(buff) << setprecision(2);
                //������� ������� ��������� ������ ���������� (SULA\AGILENT)
                CheckDlgButton(hwnd, ID_CHECKBOX_USE_GENERATOR, Generator.is_active);
                if(!Generator.is_active)
                {
                    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_PERIOD), FALSE);
                    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_WIDTHPULSE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_UPPERBOUNDERY), FALSE);
                    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_LOWERBOUNDERY), FALSE);
                }
                /* ���������� ����� � ����������� �� ������ */
                BOOL state = TRUE;
                if(index_mode == ITS)
                    state = FALSE;
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_STEP),           state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_UPPERBOUNDERY),  state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_STEP_VOLTAGE),  !state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_BEGIN_VOLTAGE), !state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_END_VOLTAGE),   !state);
            }
        return TRUE;
        /* �������� ���� � ��������, ������ �� ������� */
        case ID_BUTTON_PID_SETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(ID_PID_SETTINGS_WINDOW), hwnd, (DLGPROC)pidlg_proc);
        return TRUE;
        case ID_BUTTON_ADVANCED_SETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(ID_ADVANCED_SETTINGS_WINDOW), hwnd, (DLGPROC)asdlg_proc);
        return TRUE;
        case ID_BUTTON_CORRELATION_SETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(ID_CORRELATION_SETTINGS_WINDOW), hwnd, (DLGPROC)csdlg_proc);
        return TRUE;
        case ID_BUTTON_CLOSE_SETTINGS:
            DestroyWindow(hwnd);
        return TRUE;
        /* ���������� ��������� �������� */
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
        return TRUE;
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
            return TRUE;
        case ID_BUTTON_APPLY_SETTINGS:
            {
                bool alright = true;
                /* �������� ���������� ��������� �������� */
                /*if(ApplySettingEditBox(hwnd, ID_EDITCONTROL_GATE, 2) == 0.0)
                {
                    MessageBox(hwnd, "Gate should be more than 0.0", "Warning", MB_ICONWARNING);
                    break;
                }*/
                /* ����� ��������� */
                //��������� ��������� ����� ����� ����������
                FileSaveName = ApplySettingEditBoxString(hwnd, ID_EDITCONTROL_FILE_SAVE_NAME);
                //��������� ��������� ������ ������ ���������
                index_mode = SendMessage(GetDlgItem(hwnd, ID_COMBOBOX_MODE), CB_GETCURSEL, 0, 0);
                /* ��������� LakeShore */
                //��������� ��������� ���� ������������
                Thermostat.TempStep = ApplySettingEditBox(hwnd, ID_EDITCONTROL_STEP, 2);
                //��������� ��������� ����������� ��������
                Thermostat.TempDisp = ApplySettingEditBox(hwnd, ID_EDITCONTROL_DISPERSION, 2);
                //��������� ��������� ������� ����������
                aver_time = ApplySettingEditBox(hwnd, ID_EDITCONTROL_AVERAGING_TIME);
                /* ��������� ��������� */
                //��������� ��������� ������� ���������
                measure_time_DAQ = ApplySettingEditBox(hwnd, ID_EDITCONTROL_MEASURE_TIME);
                //��������� ��������� �������� ������ DQA
                rate_DAQ = ApplySettingEditBox(hwnd, ID_EDITCONTROL_RATE);
                //��������� ��������� ���������� ��������� ������ DAQ
                averaging_DAQ = ApplySettingEditBox(hwnd, ID_EDITCONTROL_AVERAGING);
                //��������� ��������� ������������ �����
                gate_DAQ = ApplySettingEditBox(hwnd, ID_EDITCONTROL_GATE, 2);
                //��������� ��������� ��������� ����������
                index_range = SendMessage(GetDlgItem(hwnd, ID_COMBOBOX_VOLTAGE_RANGE), CB_GETCURSEL, 0, 0);
                /* ��������� ���������� ��������� */
                //��������� ��������� �������
                Generator.period = ApplySettingEditBox(hwnd, ID_EDITCONTROL_PERIOD, 2);
                //������� ������� ��������� ������ ���������
                Generator.width = ApplySettingEditBox(hwnd, ID_EDITCONTROL_WIDTHPULSE, 2);
                if(Generator.width >= Generator.period) alright = false;
                //��������� ��������� ������� ������� �������� � �������
                Generator.amplitude = ApplySettingEditBox(hwnd, ID_EDITCONTROL_UPPERBOUNDERY, 3);
                if(Generator.amplitude > MAX_VOLTAGE_PULSE || Generator.amplitude < MIN_VOLTAGE_PULSE) alright = false;
                //��������� ��������� ������ ������� �������� � �������
                Generator.bias = ApplySettingEditBox(hwnd, ID_EDITCONTROL_LOWERBOUNDERY, 3);
                if(Generator.bias < MIN_VOLTAGE_PULSE || Generator.bias > MAX_VOLTAGE_PULSE) alright = false;
                if(index_mode == ITS)
                {
                    //��������� ��������� ���� ITS � �������
                    Generator.step_voltage = ApplySettingEditBox(hwnd, ID_EDITCONTROL_STEP_VOLTAGE, 3);
                    //��������� ��������� ������ ITS � �������
                    Generator.begin_amplitude = ApplySettingEditBox(hwnd, ID_EDITCONTROL_BEGIN_VOLTAGE, 3);
                    if(Generator.begin_amplitude < MIN_VOLTAGE_PULSE || Generator.begin_amplitude > MAX_VOLTAGE_PULSE || Generator.begin_amplitude >= Generator.bias) alright = false;
                    //��������� ��������� ����� ITS � �������
                    Generator.end_amplitude = ApplySettingEditBox(hwnd, ID_EDITCONTROL_END_VOLTAGE, 3);
                    if(Generator.end_amplitude > MAX_VOLTAGE_PULSE || Generator.end_amplitude < MIN_VOLTAGE_PULSE) alright = false;
                }
                //��������� ��������� � ���������� ����������� � ��������� ���� �������� ��� ������� �� ������������
                if(alright == true)
                {
                    ApplySettings();
                    write_settings();
                }
                else
                    MessageBox(hwnd, "Incorrect settings.\nYou should be more careful.", "Warning", MB_ICONWARNING);
            }
        return TRUE;
    }
    return FALSE;
}
