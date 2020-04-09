#include "winfunc.h"
#include "variable.h"
#include "dlts_math.h"

#define WM_REPAINT WM_USER

BOOL CorWindow_OnInit(HWND, HWND, WPARAM);
BOOL CorWindow_OnCommand(HWND, INT, HWND, UINT);

HWND hWeightGraph, hEdit, hWeightList;

/* Имена весовых функций */
const string names_wFunc[] = {"Double box-car", "Lock-in", "Exponent", "Sine",
                              "Hodgart", "HiRes-3", "HiRes-4", "HiRes-5", "HiRes-6"};

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
    gwin::gPosition(hWeightGraph, 10, 18);
    gwin::gSize(hWeightGraph, 400, 276);
    //gwin::gPrecision(hWeightGraph, 0, 1);
    gwin::gTitle(hWeightGraph, "Weight function");
    //Получаем описатели элементов управления
    hWeightList = GetDlgItem(hWnd, ID_LIST_CORRELATION);
    hEdit = GetDlgItem(hWnd, ID_EDITCONTROL_TIME_CORRELATION);
    //Вывод информации при инициализации
    buff << setprecision(2) << fixed;
    //Вывести текущие настройки ширины импульсов box-car
    rewrite(buff) << correlation_width;
    SetDlgItemText(hWnd, ID_EDITCONTROL_WIDTH, buff.str().data());
    //Вывести текущие настройки константы alpha: Ts = alpa * Tc
    rewrite(buff) << correlation_alpha;
    SetDlgItemText(hWnd, ID_EDITCONTROL_ALPHA, buff.str().data());
    CheckDlgButton(hWnd, ID_CHECKBOX_USE_ALPHA, UseAlphaBoxCar);
    PostMessage(hWnd, WM_COMMAND, ID_CHECKBOX_USE_ALPHA, 0);
    //Вывести текущие настройки константы отношения Tc/Tg
    rewrite(buff) << correlation_c;
    SetDlgItemText(hWnd, ID_EDITCONTROL_C, buff.str().data());
    //Вывести текущие настройки использования альфа-коэффициента boxcar dt = alpha Tc
    CheckDlgButton(hWnd, ID_CHECKBOX_USE_ALPHA, UseAlphaBoxCar);
    PostMessage(hWnd, WM_COMMAND, ID_CHECKBOX_USE_ALPHA, 0);
    //Вывести текущие настройки времен корреляторов
    for (size_t i = 0; i < CorTc.size(); i++)
    {
        rewrite(buff) << CorTc[i];
        //ComboBox_AddString(hWeightList, buff.str().c_str());
        SendMessage(hWeightList, LB_ADDSTRING, 0, (LPARAM)buff.str().data());
    }
    if(::WeightType == DoubleBoxCar)
    {
        EnableWindow(GetDlgItem(hWnd, ID_BUTTON_PREVIOUS_WEIGHT_FUNC), FALSE);
        EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), TRUE);
    }
    else
        EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), FALSE);

    if(::WeightType == HiRes6)
        EnableWindow(GetDlgItem(hWnd, ID_BUTTON_NEXT_WEIGHT_FUNC), FALSE);

    rewrite(buff) << names_wFunc[::WeightType];
    SetDlgItemText(hWnd, ID_STR_WEIGHT_FUNCTION_NAME, buff.str().data());
    SendMessage(hWeightList, LB_SETCURSEL, 0, 0);
    //SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
    return TRUE;
}

BOOL CorWindow_OnCommand(HWND hWnd, INT id, HWND, UINT what)
{
    stringstream buff;
    static int WeightType = ::WeightType;
    static BOOL UseAlpha = UseAlphaBoxCar;
    switch(id)
    {
        case ID_CHECKBOX_USE_ALPHA:
                if(WeightType == DoubleBoxCar)
                {
                    bool state = SendMessage(GetDlgItem(hWnd, ID_CHECKBOX_USE_ALPHA), BM_GETCHECK, 0, 0);
                    EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_ALPHA), state);
                    EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), !state);
                    UseAlpha = state;
                }
                else
                {
                    EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), FALSE);
                    EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_ALPHA), FALSE);
                    EnableWindow(GetDlgItem(hWnd, ID_CHECKBOX_USE_ALPHA), FALSE);
                }
                SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
            return TRUE;
        case ID_BUTTON_PREVIOUS_WEIGHT_FUNC:
            WeightType--;
            rewrite(buff) << names_wFunc[WeightType];
            SetDlgItemText(hWnd, ID_STR_WEIGHT_FUNCTION_NAME, buff.str().data());
            if(WeightType == DoubleBoxCar)
            {
                EnableWindow(GetDlgItem(hWnd, ID_BUTTON_PREVIOUS_WEIGHT_FUNC), FALSE);

                EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), !UseAlpha);
                EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_ALPHA), UseAlpha);
                EnableWindow(GetDlgItem(hWnd, ID_CHECKBOX_USE_ALPHA), TRUE);
            }
            EnableWindow(GetDlgItem(hWnd, ID_BUTTON_NEXT_WEIGHT_FUNC), TRUE);
            SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
            return TRUE;
        case ID_BUTTON_NEXT_WEIGHT_FUNC:
            WeightType++;
            rewrite(buff) << names_wFunc[WeightType];
            SetDlgItemText(hWnd, ID_STR_WEIGHT_FUNCTION_NAME, buff.str().data());
            if(WeightType == HiRes6)
                EnableWindow(GetDlgItem(hWnd, ID_BUTTON_NEXT_WEIGHT_FUNC), FALSE);
            if(WeightType != DoubleBoxCar)
            {
                EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_WIDTH), FALSE);
                EnableWindow(GetDlgItem(hWnd, ID_EDITCONTROL_ALPHA), FALSE);
                EnableWindow(GetDlgItem(hWnd, ID_CHECKBOX_USE_ALPHA), FALSE);
            }
            EnableWindow(GetDlgItem(hWnd, ID_BUTTON_PREVIOUS_WEIGHT_FUNC), TRUE);
            SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
            return TRUE;
        case ID_BUTTON_PUSH_CORRELATION:
            {
                double Tc = ApplySettingEditBox(hWnd, ID_EDITCONTROL_TIME_CORRELATION, 2);
                double Tg = Tc / correlation_c;
                rewrite(buff) << fixed << setprecision(2) << Tc;
                if(Tc <= 0.00)
                {
                    MessageBox(hWnd, "The value must be positive and greater than zero.", "Information", MB_ICONINFORMATION);
                    return TRUE;
                }
                if(Tc + Tg > measure_time_DAQ) //Все в мс
                {
                    MessageBox(hWnd, "The value \"Tc + Tg\" must be less than \"T_measure-width\".", "Information", MB_ICONINFORMATION);
                    return TRUE;
                }
                if(ListBox_FindString(hWeightList, -1, buff.str().data()) != LB_ERR)
                {
                    MessageBox(hWnd, "The value \"Tc\" is already exist.", "Information", MB_ICONINFORMATION);
                    return TRUE;
                }

                int pos = ListBox_GetCount(hWeightList); /* По умолчанию в конец списка */
                char szBuff[BUFF_SIZE];
                do
                {
                    ListBox_GetText(hWeightList, --pos, szBuff);
                }
                while( Tc < atof(szBuff) && pos != LB_ERR);
                ListBox_InsertString(hWeightList, pos + 1, buff.str().data());
                ListBox_SetCurSel(hWeightList, ListBox_FindString(hWeightList, -1, buff.str().data()));
                SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
                SetFocus(hEdit);            /* устоановить фокус ввода */
                Edit_SetSel(hEdit, 0, -1);  /* Выделить содержимое */
            }
            return TRUE;
        case ID_BUTTON_PULL_CORRELATION: //Отсчет с нуля
            {
                int pos = SendMessage(hWeightList, LB_GETCURSEL, 0, 0);
                SendMessage(hWeightList, LB_DELETESTRING , (WPARAM)pos, 0);
                if(--pos < 0) pos = 0;
                SendMessage(hWeightList, LB_SETCURSEL, (WPARAM)pos, 0);
                SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
            }
            return TRUE;
        case ID_LIST_CORRELATION:
            if(what == LBN_SELCHANGE)
                SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
            return TRUE;
            /* without break */
        case WM_REPAINT:
            {
                bool tmp = UseAlphaBoxCar;
                UseAlphaBoxCar = UseAlpha;


                int pos = SendMessage(hWeightList, LB_GETCURSEL, 0, 0);
                SendMessage(hWeightList, LB_SETCURSEL, pos, 0);
                if(pos < 0)
                {
                    gwin::gDefaultPlot(hWeightGraph, "Set value in \"Tc edit box\" and\n click on \"Add\" button for\nadding a correlator.");
                    return TRUE;
                }
                char cstrBuff[BUFF_SIZE];
                SendMessage(hWeightList, LB_GETTEXT, (WPARAM)pos, (LPARAM)cstrBuff);

                double Tc = atof(cstrBuff);
                double Tg =  Tc / correlation_c;
                double dt = 1000.0 * pow(rate_DAQ, -1);
                const size_t points = (measure_time_DAQ + 0.001*gate_DAQ) * 1e-3 * rate_DAQ;

                gwin::gVector xAxis, yAxis;
                xAxis.reserve( points );
                yAxis.reserve( points );

                if(WeightType == DoubleBoxCar && correlation_width < 1000*10*dt)
                {
                    for(size_t i = 0; i < points; ++i)
                        xAxis.push_back(i*dt);
                    yAxis.assign(points, 0.0);
                    yAxis[size_t(Tg / dt)] = 1.0;
                    yAxis[size_t( (Tg+Tc) / dt)] = -1.0;
                }
                else
                {
                    double (*w) (double, double) = get_weight_function(WeightType);
                    for(size_t i = 0; i < points; ++i)
                    {
                        xAxis.push_back(i*dt);
                        yAxis.push_back(w(i*dt, Tg));
                    }
                }

                std::stringstream info;
                //helper(Tg, Tc, WeightType);

                corinfo cinfo = get_corinfo(WeightType, Tc);
                info << std::fixed << std::setprecision(3)
                //<< "Tc: " << Tc << endl << "Tg:" << Tg << endl << "type:" << WeightType << endl
                     << "peak-tau: " << cinfo.tau0 << " [ms]"
                     //<< "\nA0: " << get_signal_level(Tg, Tc, WeightType)
                     //<< "\nN: " << get_noize_level(Tg, Tc, WeightType)
                     << "\nSN: " << cinfo.SN
                     << "\nlw: " << lw(Tg, Tc, WeightType) << std::endl;

                gwin::gAdditionalInfo(hWeightGraph, info.str());
                gwin::gAxisInfo(hWeightGraph, "Time [ms]", "W [Arb]");
                gwin::gData(hWeightGraph, &xAxis, &yAxis);
                UseAlphaBoxCar = tmp;
            }
            return TRUE;
        case ID_BUTTON_CLOSE_SETTINGS:
            WeightType = ::WeightType;
            UseAlpha = UseAlphaBoxCar;
            DestroyWindow(hWnd);
            return TRUE;
         case ID_BUTTON_APPLY_SETTINGS:
            if(start.load())
            {
                MessageBox(hWnd, "Stop the experiment and try again.", "Warning", MB_ICONWARNING);
                return TRUE;
            }
            {

                //Применить настройки ширины импульсов
                correlation_width = ApplySettingEditBox(hWnd, ID_EDITCONTROL_WIDTH, 2);
                //Применить настройки константы отношения
                correlation_c = ApplySettingEditBox(hWnd, ID_EDITCONTROL_C, 2);
                if(correlation_c == 0)
                {
                    MessageBox(hWnd, "You cant set the fraction [Tc/Tg] = 0\n", "Warning", MB_ICONWARNING);
                    return TRUE;
                }
                correlation_alpha = ApplySettingEditBox(hWnd, ID_EDITCONTROL_ALPHA, 2);
                //Применить настройки времен корреляторов
                UseAlphaBoxCar = UseAlpha;
                ::WeightType = WeightType;
                CorTc.clear();
                CorInfo.clear();
                char cstrBuff[BUFF_SIZE];
                for (int i = 0; i < SendMessage(hWeightList, LB_GETCOUNT, 0, 0); i++)
                {
                    SendMessage(hWeightList, LB_GETTEXT, (WPARAM)i, (LPARAM)cstrBuff);
                    double Tc = atof(cstrBuff);
                    CorTc.push_back(Tc);
                }
                write_settings();
                //RefreshDLTS();
                SendMessage(hMainWindow, WM_COMMAND, WM_REFRESH_DLTS, 0);
            }
            SendMessage(hWnd, WM_COMMAND, WM_REPAINT, 0);
            return TRUE;
    }
     return FALSE;
}

