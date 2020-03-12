#include "winfunc.h"
#include "variable.h"

#define WM_REPAINT WM_USER

BOOL CorWindow_OnInit(HWND, HWND, WPARAM);
VOID CorWindow_OnCommand(HWND, INT, HWND, UINT);

HWND hWeightGraph, hEdit, hWeightList;

BOOL CALLBACK csdlg_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, CorWindow_OnInit);
        HANDLE_MSG(hWnd, WM_COMMAND, CorWindow_OnCommand); //Останется ли сообщение с стеке?
    }
    return FALSE;
}

BOOL CorWindow_OnInit(HWND hWnd, HWND, WPARAM)
{
    stringstream buff;
    hWeightGraph = gwin::gCreateWindow(hInst, hWnd, WS_CHILD | WS_DLGFRAME | WS_VISIBLE);
    gwin::gPosition(hWeightGraph, 13, 25);
    gwin::gSize(hWeightGraph, 328, 230);
    gwin::gPrecision(hWeightGraph, 0, 1);
    //Получаем описатели элементов управления
    hWeightList = GetDlgItem(hWnd, ID_LIST_CORRELATION);
    hEdit = GetDlgItem(hWnd, ID_EDITCONTROL_TIME_CORRELATION);
    //Вывод информации при инициализации
    buff << setprecision(2) << fixed;
    //Вывести текущие настройки ширины импульсов
    rewrite(buff) << correlation_width;
    SetDlgItemText(hWnd, ID_EDITCONTROL_WIDTH, buff.str().data());
    //Вывести текущие настройки константы отношения
    rewrite(buff) << correlation_c;
    SetDlgItemText(hWnd, ID_EDITCONTROL_C, buff.str().data());
    //Вывести текущие настройки времен корреляторов
    for (size_t i = 0; i < CorTime.size(); i++)
    {
        rewrite(buff) << CorTime[i];
        SendMessage(hWeightList, LB_ADDSTRING, 0, (LPARAM)buff.str().data());
    }
    if(CorType == DoubleBoxCar)
    {
        EnableWindow(GetDlgItem(hWnd, ID_BUTTON_PREVIOUS_WEIGHT_FUNC), FALSE);
        EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), TRUE);
    }
    else
        EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), FALSE);
    if(CorType == SinW)
        EnableWindow(GetDlgItem(hWnd, ID_BUTTON_NEXT_WEIGHT_FUNC), FALSE);
    rewrite(buff) << names_wFunc[CorType];
    SetDlgItemText(hWnd, ID_STR_WEIGHT_FUNCTION_NAME, buff.str().data());
    SendMessage(hWeightList, LB_SETCURSEL, 0, 0);
    SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
    return TRUE;
}

VOID CorWindow_OnCommand(HWND hWnd, INT id, HWND, UINT)
{
    stringstream buff;
    static UINT CorType = ::CorType;
    switch(id)
    {
        case ID_BUTTON_PREVIOUS_WEIGHT_FUNC:
            CorType--;
            rewrite(buff) << names_wFunc[CorType];
            SetDlgItemText(hWnd, ID_STR_WEIGHT_FUNCTION_NAME, buff.str().data());
            if(CorType == DoubleBoxCar)
            {
                EnableWindow(GetDlgItem(hWnd, ID_BUTTON_PREVIOUS_WEIGHT_FUNC), FALSE);
                EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), TRUE);
            }
            EnableWindow(GetDlgItem(hWnd, ID_BUTTON_NEXT_WEIGHT_FUNC), TRUE);
            SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
            break;
        case ID_BUTTON_NEXT_WEIGHT_FUNC:
            CorType++;
            rewrite(buff) << names_wFunc[CorType];
            SetDlgItemText(hWnd, ID_STR_WEIGHT_FUNCTION_NAME, buff.str().data());
            if(CorType == SinW)
                EnableWindow(GetDlgItem(hWnd, ID_BUTTON_NEXT_WEIGHT_FUNC), FALSE);
            if(CorType != DoubleBoxCar)
                EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), FALSE);
            EnableWindow(GetDlgItem(hWnd, ID_BUTTON_PREVIOUS_WEIGHT_FUNC), TRUE);
            SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
            break;
        case ID_BUTTON_PUSH_CORRELATION:
            {
                double time = ApplySettingEditBox(hWnd, ID_EDITCONTROL_TIME_CORRELATION, 2);
                if(time < 0.00)
                {
                    MessageBox(hWnd, "The value must be positive and greater than zero.", "Information", MB_ICONINFORMATION);
                    break;
                }
                if(time*correlation_c > measure_time_DAQ) //Все в мс
                {
                    MessageBox(hWnd, "The value \"t_1*C\" must be less than \"T_measure-width\".", "Information", MB_ICONINFORMATION);
                    break;
                }
                rewrite(buff) << time;
                SendMessage(hWeightList, LB_ADDSTRING, 0, (LPARAM)buff.str().data());
                SetFocus(hEdit);
            }
            break;
        case ID_BUTTON_PULL_CORRELATION: //Отсчет с нуля
                SendMessage(hWeightList, LB_DELETESTRING , (WPARAM)SendMessage(hWeightList, LB_GETCURSEL, 0, 0), 0);
            break;
        case ID_LIST_CORRELATION:
        case WM_REPAINT:
            {
                char cstrBuff[BUFF_SIZE];
                gwin::gVector xAxis, yAxis;
                SendMessage(hWeightList, LB_GETTEXT, (WPARAM)SendMessage(hWeightList, LB_GETCURSEL, 0, 0), (LPARAM)cstrBuff);
                double t1 = atof(cstrBuff)/1000.0;
                double (*w) (double, double) = double_boxcar;
                switch(CorType)
                {
                    case LockIn: w = lock_in;
                    break;
                    case ExpW: w = exp_w;
                    break;
                    case SinW: w = sin_w;
                    break;
                }
                double dt = 1000.0 * pow(rate_DAQ, -1);
                for(double time = 0.0; time < measure_time_DAQ; time += dt)
                {
                    xAxis.push_back(time);
                    yAxis.push_back(w(0.001 * time, t1));
                }
                gwin::gData(hWeightGraph, &xAxis, &yAxis);
            }
            break;
        case ID_BUTTON_CLOSE_SETTINGS:
            EndDialog(hWnd, 0);
            break;
         case ID_BUTTON_APPLY_SETTINGS:
            if(start.load())
            {
                MessageBox(hWnd, "Stop the experiment and try again.", "Warning", MB_ICONWARNING);
                break;
            }
            {
                ::CorType = CorType;
                char cstrBuff[BUFF_SIZE];
                double time = 0.0;
                //Применить настройки ширины импульсов
                correlation_width = ApplySettingEditBox(hWnd, ID_EDITCONTROL_WIDTH, 2);
                //Применить настройки константы отношения
                correlation_c = ApplySettingEditBox(hWnd, ID_EDITCONTROL_C, 2);
                //Применить настройки времен корреляторов
                CorTime.clear();
                for (int i = 0; i < SendMessage(hWeightList, LB_GETCOUNT, 0, 0); i++)
                {
                    SendMessage(hWeightList, LB_GETTEXT, (WPARAM)i, (LPARAM)cstrBuff);
                    time = atof(cstrBuff);
                    CorTime.push_back(time);
                }
                sort(CorTime.begin(), CorTime.end());
                if(yAxisDLTS != nullptr)
                    delete []yAxisDLTS;
                yAxisDLTS = new vector<double>[CorTime.size()]; ///Число осей совпадает с числом корреляторов
                write_settings();
                RefreshDLTS();
            }
            SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
            break;
    }
}

