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
                rewrite(buff) << Thermostat.GetID();
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_LAKESHORE, buff.str().data());
                //������� ������� ��������� ����������
                rewrite(buff) << Generator.GetID();
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_GENERATOR, buff.str().data());
                //������� ������� ��������� ���� ����������� �����
                rewrite(buff) << ai_port;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_AI, buff.str().data());
                //������� ������� ��������� ����� ��� �������
                rewrite(buff) << pfi_ttl_port;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_TTL, buff.str().data());
                //������� ������� ��������� ���� ����������� ����� ���������
                rewrite(buff) << ai_port_pulse;
                SetDlgItemText(hAdvSetDlg, ID_EDITCONTROL_AI_PULSE, buff.str().data());
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
                        //��������� ��������� DAQ
                        id_DAQ = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_DAQ);
                        //��������� ��������� LakeShore
                        Thermostat.GetID() = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_LAKESHORE);
                        //��������� ��������� ����������
                        Generator.GetID() = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_GENERATOR);
                        //��������� ��������� ���� ����������� �����
                        ai_port = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_AI);
                        //��������� ��������� ����� ��� �������
                        pfi_ttl_port = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_TTL);
                        //��������� ��������� ���� ����������� �����
                        ai_port_pulse = ApplySettingEditBox(hAdvSetDlg, ID_EDITCONTROL_AI_PULSE);
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