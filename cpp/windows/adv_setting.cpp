#include <winfunc.h>

//������� ������� ����������� ���� ����������� ��������
BOOL CALLBACK asdlg_proc(HWND hAdvSetDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static stringstream buff;
    switch(uMsg)
        {
            case WM_INITDIALOG:
            {
                //����� ���������� ��� �������������
                buff << setprecision(2) << fixed;
                //������� ������� ��������� DAQ
                rewrite(buff) << id_DAQ;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_DAQ, buff.str().data());
                //������� ������� ��������� LakeShore
                rewrite(buff) << Thermostat.gpib;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_LAKESHORE, buff.str().data());
                //������� ������� ��������� ����������
                rewrite(buff) << Generator.gpib;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_GENERATOR, buff.str().data());
                //������� ������� ��������� ���� ����������� �����
                rewrite(buff) << ai_port_measurement;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_AI, buff.str().data());
                //������� ������� ��������� ����� ��� �������
                rewrite(buff) << pfi_ttl_port;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_TTL, buff.str().data());
                //������� ������� ��������� ���� ����������� ����� ���������
                rewrite(buff) << ai_port_pulse;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_AI_PULSE, buff.str().data());
                //������� ������� ��������� ������ ������ ����������
                rewrite(buff) << Generator.curr_channel;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_OUT_CHANNEL, buff.str().data());
                //������� ������� ��������� ���� ����������� ����� �������
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
                        //��������� ��������� DAQ
                        id_DAQ = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_DAQ);
                        //��������� ��������� LakeShore
                        Thermostat.gpib = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_LAKESHORE);
                        //��������� ��������� ����������
                        Generator.gpib = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_GENERATOR);
                        //��������� ��������� ���� ����������� �����
                        ai_port_measurement = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_AI);
                        //��������� ��������� ����� ��� �������
                        pfi_ttl_port = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_TTL);
                        //��������� ��������� ���� ����������� ����� ���������
                        ai_port_pulse = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_AI_PULSE);
                        //������� ������� ��������� ������ ������ ����������
                        Generator.curr_channel = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_OUT_CHANNEL);
                        //��������� ��������� ���� ����������� ����� �������
                        ai_port_capacity = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_AI_CAPACITY);
                        //��������� ��������� � ���������� ����������� � ��������� ���� ��������
                        ApplySettings();
                        write_settings();
                    break;
                }
                return TRUE;
            }
        }
        return FALSE;
}
