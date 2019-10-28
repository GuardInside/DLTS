#include <winfunc.h>

//������� ������� ����������� ���� ��������� ����� save-�����
BOOL CALLBACK rndlg_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static stringstream buff;
    switch(uMsg)
        {
            case WM_INITDIALOG:
            {
                //������� ������� ��������� ����� ����� ����������
                rewrite(buff) << FileSaveName;
                SetDlgItemText(hwnd, ID_EDITCONTROL_NEW_NAME, buff.str().data());
            }
            return TRUE;
            case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case ID_BUTTON_CONTINUE:
                        //��������� ��������� ����� ����� ����������
                        FileSaveName = ApplySettingEditBoxString(hwnd, ID_EDITCONTROL_NEW_NAME);
                        //C�������� ���� ��������
                        write_settings();
                        EndDialog(hwnd, 0);
                    break;
                }
                return TRUE;
            }
        }
        return FALSE;
}
