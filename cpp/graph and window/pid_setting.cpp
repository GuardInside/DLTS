#include <winfunc.h>

//������� ������� ����������� ���� �������� PID
BOOL CALLBACK pidlg_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static stringstream buff;
    static HWND hRange[10];
    switch(uMsg)
        {
            case WM_INITDIALOG:
            {
                //����� ���������� ��� �������������
                buff << setprecision(2) << fixed;
                //������� ������� ��������� ���� ���
                for(int i = 0; i < QUANTITY_ZONE; i++)
                {
                    //������� ������� ��������� ������� �������
                    rewrite(buff) << ZoneTable.upper_boundary[i];
                    SetDlgItemText(hDlg, ZoneTable.GetIDRes(i, "UPPER_BOUNDARY"), buff.str().data());
                    //������� ������� ��������� �������� P
                    rewrite(buff) << ZoneTable.P[i];
                    SetDlgItemText(hDlg, ZoneTable.GetIDRes(i, "P"), buff.str().data());
                    //������� ������� ��������� �������� I
                    rewrite(buff) << ZoneTable.I[i];
                    SetDlgItemText(hDlg, ZoneTable.GetIDRes(i, "I"), buff.str().data());
                    //������� ������� ��������� �������� D
                    rewrite(buff) << ZoneTable.D[i];
                    SetDlgItemText(hDlg, ZoneTable.GetIDRes(i, "D"), buff.str().data());
                    //������� ������� ��������� ������ �������
                    hRange[i] = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                                            435, 349-(i*32.5), 80, 120, hDlg, NULL, hInst, NULL);
                    for (int j = 0; j < RANGE_HEATING_SIZE; j++)
                        SendMessage(hRange[i], CB_ADDSTRING, 0, (LPARAM)strHeatingRange[j].data());
                    SendMessage(hRange[i], CB_SETCURSEL, ZoneTable.range[i], 0);
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
                        //��������� ��������� ���
                        for(int i = 0; i < QUANTITY_ZONE; i++)
                        {
                            ZoneTable.upper_boundary[i] = ApplySettingEditBox(hDlg, ZoneTable.GetIDRes(i, "UPPER_BOUNDARY"));
                            ZoneTable.P[i] = ApplySettingEditBox(hDlg, ZoneTable.GetIDRes(i, "P"));
                            ZoneTable.I[i] = ApplySettingEditBox(hDlg, ZoneTable.GetIDRes(i, "I"));
                            ZoneTable.D[i] = ApplySettingEditBox(hDlg, ZoneTable.GetIDRes(i, "D"));
                            /* ��������� ������� �������� ���� */
                            ZoneTable.range[i] = SendMessage(hRange[i], CB_GETCURSEL, 0, 0);
                        }
                        SetZones();
                        write_settings();
                    break;
                }
                return TRUE;
            }
        }
        return FALSE;
}