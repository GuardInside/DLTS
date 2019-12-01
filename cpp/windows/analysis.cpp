#include "winfunc.h"
#include "ini.h"

BOOL Analysis_OnInit(HWND, HWND, LPARAM);
BOOL Analysis_OnCommand(HWND, INT, HWND, UINT);

INT_PTR CALLBACK anwin_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, Analysis_OnInit);
        HANDLE_MSG(hWnd, WM_COMMAND, Analysis_OnCommand);
    }
    return FALSE;
}

BOOL Analysis_OnInit(HWND hWnd, HWND, LPARAM)
{
    int max_iter = 0;
    double abs_error = 0.0;
    double rel_error = 0.0;
    double sigma = 0.0;
    bool isFitting = false;
    ini::File AnalysisFile{"analysis.ini"};
    AnalysisFile.ReadDouble("Fitting", "abs_error", &abs_error);
    AnalysisFile.ReadDouble("Fitting", "rel_error", &rel_error);
    AnalysisFile.ReadDouble("Fitting", "sigma", &sigma);
    AnalysisFile.ReadBool("Fitting", "use_fitting", &isFitting);
    AnalysisFile.ReadInt("Fitting", "max_it", &max_iter);
    stringstream buff;
    buff << setprecision(0);
    /* ����� ���������� ��� ������������� */
    buff << max_iter;
    SetDlgItemText(hWnd, ID_EDITCONTROL_ITERATION, buff.str().data());
    //������� ������� ��������� ���������� ������
    rewrite(buff) << scientific;
    rewrite(buff) << abs_error;
    SetDlgItemText(hWnd, ID_EDITCONTROL_ABS_ERROR, buff.str().data());
    //������� ������� ��������� ������������� ������
    rewrite(buff) << scientific;
    rewrite(buff) << rel_error;
    SetDlgItemText(hWnd, ID_EDITCONTROL_REL_ERROR, buff.str().data());
    //������� ������� ��������� ������������� ��������� sigma
    rewrite(buff) << fixed;
    rewrite(buff) << sigma;
    SetDlgItemText(hWnd, ID_EDITCONTROL_SIGMA, buff.str().data());
    //������� ������� ��������� ������������� ��������
    CheckDlgButton(hWnd, ID_CHECKBOX_USE_FITTING, isFitting);
    return TRUE;
}

BOOL Analysis_OnCommand(HWND hWnd, INT id, HWND, UINT)
{
    switch(id)
    {
        case ID_BUTTON_APPLY:
            {

                int max_iter = 0;
                double abs_error = 0.0;
                double rel_error = 0.0;
                double sigma = 0.0;
                int isFitting = 0;
                ini::File AnalysisFile{"analysis.ini"};
                /* ���������� ����� �������� */
                max_iter = ApplySettingEditBox(hWnd, ID_EDITCONTROL_ITERATION);
                /* ���������� ����������� */
                stringstream buff;
                buff << setprecision(0) << scientific;
                GetDlgItemTextMod(hWnd, ID_EDITCONTROL_ABS_ERROR, buff);
                abs_error = atof(buff.str().data());
                /* ������������� ����������� */
                GetDlgItemTextMod(hWnd, ID_EDITCONTROL_REL_ERROR, buff);
                rel_error = atof(buff.str().data());
                sigma = ApplySettingEditBox(hWnd, ID_EDITCONTROL_SIGMA);
                /* ��������� ������ ������������� */
                isFitting = SendMessage(GetDlgItem(hWnd, ID_CHECKBOX_USE_FITTING), BM_GETCHECK, 0, 0);
                /* ��������� ��������� */
                AnalysisFile.WriteInt("Fitting", "max_it", max_iter);
                AnalysisFile.WriteDoubleSc("Fitting", "abs_error", abs_error, 0);
                AnalysisFile.WriteDoubleSc("Fitting", "rel_error", rel_error, 0);
                AnalysisFile.WriteDoubleFix("Fitting", "sigma", sigma, 0);
                AnalysisFile.WriteBool("Fitting", "use_fitting", isFitting);
                /* ��������� �� �������� ���������� �������� */
                HANDLE hThreadSuccess = (HANDLE)_beginthreadex(NULL, 0, dlg_success, NULL, 0, NULL);
                CloseHandle(hThreadSuccess);
                /* ������������� � ��������� ������� */
                PlotRelax();
                RefreshDLTS();
            }
            return TRUE;
        case ID_BUTTON_CLOSE:
            DestroyWindow(hWnd);
            return TRUE;
    }
    return FALSE;
}