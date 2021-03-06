#include <winfunc.h>
#define RANGE_NUM                  4   /* ����� ���������� ���������� ��� ��� */

#define ID_COMBOBOX_VOLTAGE_RANGE       200
#define ID_COMBOBOX_MODE                201
#define ID_COMBOBOX_RANGE_SULA          202
#define ID_COMBOBOX_PRE_AMP_GAIN_SULA   203
#define WM_INITDLG                      WM_USER

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


BOOL SettingsWindow_OnInit(HWND hwnd, HWND, LPARAM)
{
    HWND hComboBox_range;
    HWND hComboBox_mode;
    HWND hComboBox_RangeSula;
    HWND hComboBox_PreAmpGain;
    //������� �������� ��������� ���������� ����������
    hComboBox_range = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
    167, 63, 95, 125, hwnd, (HMENU)(ID_COMBOBOX_VOLTAGE_RANGE), hInst, NULL);
    for (int i = 0; i < RANGE_NUM; i++)
        SendMessage(hComboBox_range, CB_ADDSTRING, 0, (LPARAM)range[i].data());
    SendMessage(hComboBox_range, CB_SETCURSEL, index_range.load(), 0);
    /* ��������� ������ ������������ */
    hComboBox_mode = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
    555, 180, 100, 125, hwnd, (HMENU)(ID_COMBOBOX_MODE), hInst, NULL);
        ComboBox_AddString(hComboBox_mode, "DLTS");
        ComboBox_AddString(hComboBox_mode, "ITS");
        //ComboBox_AddString(hComboBox_mode, "BITS");
        ComboBox_SetCurSel(hComboBox_mode, index_mode.load());
    /* ��������� SULA.CAPACITY_RANGE */
    hComboBox_RangeSula = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
    425, 62, 100, 125, hwnd, (HMENU)(ID_COMBOBOX_RANGE_SULA), hInst, NULL);
    stringstream buff;
    for(int i = 0; i < 5; i++)
    {
        rewrite(buff) << int_range_sula[i];
        ComboBox_AddString(hComboBox_RangeSula, buff.str().c_str());
    }
    ComboBox_SetCurSel(hComboBox_RangeSula, RANGE_SULA_index);
    /* ��������� SULA.PRE_AMP_GAIN */
    hComboBox_PreAmpGain = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
    425, 122, 100, 125, hwnd, (HMENU)(ID_COMBOBOX_PRE_AMP_GAIN_SULA), hInst, NULL);
    for(int i = 0; i < 5; i++)
    {
        rewrite(buff) << int_pre_amplifier[i];
        ComboBox_AddString(hComboBox_PreAmpGain, buff.str().c_str());
    }
    ComboBox_SetCurSel(hComboBox_PreAmpGain, PRE_AMP_GAIN_SULA_index);
    /* ����� �������� � �������� */
    PostMessage(hwnd, WM_COMMAND, WM_INITDLG, 0);
    return TRUE;
}

BOOL SettingsWindow_OnCommand(HWND hwnd, int ID, HWND, UINT codeNotify)
{
    static mode index_mode = ::index_mode.load();
    static BOOL Generator_Agilent_is_active = Generator.is_active.load();
    static HWND hCorSettingsWnd;
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
                //������� ������� ��������� ���� � ����� ����������
                rewrite(buff) << FileSaveName;
                SetDlgItemText(hwnd, ID_EDITCONTROL_FILE_SAVE_NAME, buff.str().data());
                //������� ������� ��������� ����� ����� ����������
                rewrite(buff) << FileSavePath;
                SetDlgItemText(hwnd, ID_EDITCONTROL_SAVE_PATH, buff.str().data());
                /* ��������� ���������� ��������� */
                //������� ������� ��������� �������
                rewrite(buff) << Generator.period;
                SetDlgItemText(hwnd, ID_EDITCONTROL_PERIOD, buff.str().data());
                //������� ������� ��������� ������ ���������
                rewrite(buff) << setprecision(3);
                rewrite(buff) << 1000.0 * Generator.width;
                SetDlgItemText(hwnd, ID_EDITCONTROL_WIDTHPULSE, buff.str().data());
                //������� ������� ��������� ��������� �������� � �������
                rewrite(buff) << setprecision(3);
                rewrite(buff) << Generator.amp;
                SetDlgItemText(hwnd, ID_EDITCONTROL_AMPLITUDE, buff.str().data());
                //������� ������� ��������� �������� �������� � �������
                rewrite(buff) << Generator.bias;
                SetDlgItemText(hwnd, ID_EDITCONTROL_BIAS, buff.str().data());
                //������� ������� ��������� ������ ���������� (SULA\AGILENT)
                CheckDlgButton(hwnd, ID_CHECKBOX_USE_GENERATOR, Generator.is_active.load());
                PostMessage(hwnd, WM_COMMAND, ID_CHECKBOX_USE_GENERATOR, 0);
                //������� ������� ��������� ����������� �����
                rewrite(buff) << setprecision(3) << scientific << ::dEfMass;
                SetDlgItemText(hwnd, ID_EDITCONTROL_EFFECTIVE_MASS, buff.str().data());
                //������� ������� ��������� ������� ����������
                rewrite(buff) << setprecision(0) << fixed <<::dFactorG;
                SetDlgItemText(hwnd, ID_EDITCONTROL_G, buff.str().data());
                //������� ������� ��������� ������������ ������ �������
                rewrite(buff) << setprecision(3) << scientific << ::dImpurity;
                SetDlgItemText(hwnd, ID_EDITCONTROL_IMPURITY, buff.str().data());
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
            if(!IsWindow(hCorSettingsWnd))
                hCorSettingsWnd = CreateDialog(hInst, MAKEINTRESOURCE(ID_CORRELATION_SETTINGS_WINDOW), hwnd, csdlg_proc);
            //DialogBox(hInst, MAKEINTRESOURCE(ID_CORRELATION_SETTINGS_WINDOW), hwnd, (DLGPROC)csdlg_proc);
        return TRUE;
        case ID_BUTTON_CLOSE_SETTINGS:
            /* ������� ����������� ���������� */
            Generator_Agilent_is_active = Generator.is_active.load();
            index_mode = ::index_mode.load();
            DestroyWindow(hwnd);
        return TRUE;
        /* ���������� ��������� �������� */
        case ID_CHECKBOX_USE_GENERATOR:
            {
                bool state = SendMessage(GetDlgItem(hwnd, ID_CHECKBOX_USE_GENERATOR), BM_GETCHECK, 0, 0);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_PERIOD), state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_WIDTHPULSE), state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_AMPLITUDE), state);
                EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_BIAS), state);
                Generator_Agilent_is_active = state;
            }
            return TRUE;
        case ID_COMBOBOX_MODE:
            if(codeNotify == CBN_SELCHANGE)
            {
                index_mode = static_cast<mode>(SendMessage(GetDlgItem(hwnd, ID_COMBOBOX_MODE), CB_GETCURSEL, 0, 0));
                PostMessage(hwnd, WM_COMMAND, ID_CHECKBOX_USE_GENERATOR, 0);
            }
            return TRUE;
        case ID_EDITCONTROL_SAVE_PATH:
            break;
        case ID_BUTTON_APPLY_SETTINGS:
            if(start.load())
            {
                MessageBox(hwnd, "Stop the experiment and try again.", "Warning", MB_ICONWARNING);
                return TRUE;
            }
            {
                bool alright = true;
                /* ����� ��������� */
                //��������� ��������� ���� ����� ����������
                FileSavePath = ApplySettingEditBoxString(hwnd, ID_EDITCONTROL_SAVE_PATH);
                //��������� ��������� ����� ����� ����������
                FileSaveName = ApplySettingEditBoxString(hwnd, ID_EDITCONTROL_FILE_SAVE_NAME);
                //��������� ��������� ������ ������ ���������
                //index_mode = static_cast<mode>(SendMessage(GetDlgItem(hwnd, ID_COMBOBOX_MODE), CB_GETCURSEL, 0, 0));
                /* ��������� LakeShore */
                //��������� ��������� ���� ������������
                Thermostat.TempStep = ApplySettingEditBox(hwnd, ID_EDITCONTROL_STEP, 2);
                //��������� ��������� ����������� ��������
                Thermostat.TempDisp = ApplySettingEditBox(hwnd, ID_EDITCONTROL_DISPERSION, 2);
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
                index_range = ComboBox_GetCurSel(GetDlgItem(hwnd, ID_COMBOBOX_VOLTAGE_RANGE));
                /* ��������� SULA */
                //��������� ��������� ��������� ����������
                RANGE_SULA_index = ComboBox_GetCurSel(GetDlgItem(hwnd, ID_COMBOBOX_RANGE_SULA));
                //��������� ��������� ��������� ����������
                PRE_AMP_GAIN_SULA_index = ComboBox_GetCurSel(GetDlgItem(hwnd, ID_COMBOBOX_PRE_AMP_GAIN_SULA));
                /* ��������� ���������� ��������� */
                if(Generator_Agilent_is_active)
                {
                    //��������� ��������� �������
                    Generator.period = ApplySettingEditBox(hwnd, ID_EDITCONTROL_PERIOD, 2);
                    //������� ������� ��������� ������ ���������
                    Generator.width = 0.001 * ApplySettingEditBox(hwnd, ID_EDITCONTROL_WIDTHPULSE, 2);
                    if(Generator.width >= Generator.period) alright = false;
                    //��������� ��������� ��������� �������� � �������
                    Generator.amp = ApplySettingEditBox(hwnd, ID_EDITCONTROL_AMPLITUDE, 3);
                    if(Generator.amp > MAX_VOLTAGE_PULSE || Generator.amp < MIN_VOLTAGE_PULSE) alright = false;
                    //��������� ��������� �������� �������� � �������
                    Generator.bias = ApplySettingEditBox(hwnd, ID_EDITCONTROL_BIAS, 3);
                    if(Generator.bias < MIN_VOLTAGE_PULSE || Generator.bias > MAX_VOLTAGE_PULSE) alright = false;
                }
                //��������� ��������� ����������� ����� � ������������ ������ �������
                {
                    stringstream buff;
                    buff << setprecision(3) << scientific;
                    GetDlgItemTextMod(hwnd, ID_EDITCONTROL_EFFECTIVE_MASS, buff);
                    ::dEfMass = atof(buff.str().data());
                    GetDlgItemTextMod(hwnd, ID_EDITCONTROL_IMPURITY, buff);
                    ::dImpurity = atof(buff.str().data());
                }
                //��������� ��������� ������� ����������
                ::dFactorG = ApplySettingEditBox(hwnd, ID_EDITCONTROL_G, 0);
                //��������� ��������� � ���������� ����������� � ��������� ���� �������� ��� ������� �� ������������
                if(alright == true)
                {
                    bool bfRefresh = false;
                    if(::index_mode.load() != index_mode)
                    {
                        bfRefresh = true;
                    }

                    Generator.is_active = Generator_Agilent_is_active;
                    ::index_mode.store(index_mode);
                    ApplySettings();
                    write_settings();

                    if(bfRefresh)
                        SendMessage(hMainWindow, WM_COMMAND, WM_REFRESH_DLTS, 0);
                    else
                        SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
                }
                else
                    MessageBox(hwnd, "Incorrect settings.\nYou should be more careful.", "Warning", MB_ICONWARNING);
            }
        return TRUE;
    }
    return FALSE;
}
