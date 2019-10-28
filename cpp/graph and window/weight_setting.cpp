#include <winfunc.h>

//Оконная функция окна графика Весовой функции
/*LRESULT CALLBACK cswin_proc(HWND hChildWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    vector<double> xAxis, yAxis;
    static int k = 0;
    char buff_c[BUFF_SIZE];
    double t1 = 0.0;
    switch(uMsg)
    {
        case WM_COMMAND:
            //Выбираем номер коррелятора
            if(wParam == WM_NEW_CORTIME)
            {
                k = (int)lParam;
                InvalidateRect(hChildWnd, NULL, FALSE);
            }
            break;
        case WM_PAINT:
            SendMessage(hWeightList, LB_GETTEXT, (WPARAM)k, (LPARAM)buff_c);
            t1 = atof(buff_c)/1000.0;
            if(t1 <= 0.0 || t1*correlation_c > measure_time_DAQ/1000.0)
                break;
            switch(CorType)
            {
                case DoubleBoxCar:
                    for(double time = 0.0; time < measure_time_DAQ; time += correlation_width/1000/2)
                    {
                        xAxis.push_back(time);
                        yAxis.push_back(double_boxcar(time/1000.0, t1));
                    }
                    break;
                case LockIn:
                    for(double time = 0.0; time < measure_time_DAQ; time += correlation_width/1000/2)
                    {
                        xAxis.push_back(time);
                        yAxis.push_back(lock_in(time/1000.0, t1));
                    }
                    break;
                case ExpW:
                    for(double time = 0.0; time < measure_time_DAQ; time += correlation_width/1000/2)
                    {
                        xAxis.push_back(time);
                        yAxis.push_back(exp_w(time/1000.0, t1));
                    }
                    break;
                case SinW:
                    for(double time = 0.0; time < measure_time_DAQ; time += correlation_width/1000/2)
                    {
                        xAxis.push_back(time);
                        yAxis.push_back(sin_w(time/1000.0, t1));
                    }
                    break;
            }
            hdc = BeginPaint(hChildWnd, &ps);
                //print_graph(xAxis, 0, yAxis, 1, hdc, hline, min_y, max_y, min_x, max_x, sx, sy, 5, 5);
            EndPaint(hChildWnd, &ps);
            break;
        default:
            return DefWindowProc (hChildWnd, uMsg, wParam, lParam);
    }
    return 0;
}*/
