#include <graph.h>
#include <winfunc.h>

//Оконная функция дочернего окна графика обработанной релаксации
/*LRESULT CALLBACK rewin_proc(HWND hRelax, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    static HPEN hline;
    static int sx, sy;
    static double max_y, min_y, min_x, max_x;
    static vector<double> xAxis;
    static stringstream buff;
    switch(uMsg)
    {
        case WM_CREATE:
            buff << setprecision(2) << fixed;
            hline = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            break;
        case WM_SIZE:
            sx = LOWORD(lParam);
            sy = HIWORD(lParam);
            break;
        case WM_PAINT:
            TryEnterCriticalSection(&csRelaxation);
            xAxis = vector<double>();
                for(size_t i = 0; i < Relaxation.size(); i++)
                    xAxis.push_back(i*(1/rate_DAQ)*1000);
            hdc = BeginPaint(hRelax, &ps);
                print_graph_interp(xAxis, 0, Relaxation, 3, hdc, hline, min_y, max_y, min_x, max_x, sx, sy, 5, 5);
                if(index_mode == DLTS && !xAxisDLTS.empty())
                {
                    rewrite(buff) << xAxisDLTS[index_relax] << " K";
                    TextOut(hdc, GRAPHWIDTH, GRAPHSIZE, buff.str().data(), buff.str().size());
                }
                else if(index_mode == ITS && !itsLowVoltages.empty())
                {
                    rewrite(buff) << itsTemperature << " K";
                    TextOut(hdc, GRAPHWIDTH, GRAPHSIZE, buff.str().data(), buff.str().size());
                    buff << setprecision(3);
                    rewrite(buff) << itsLowVoltages[index_relax] << " to " << itsUpVoltages[index_relax] << " V";
                    TextOut(hdc, GRAPHWIDTH-202, GRAPHSIZE-100, buff.str().data(), buff.str().size());
                    buff << setprecision(2);
                }
            EndPaint(hRelax, &ps);
            LeaveCriticalSection(&csRelaxation);
            break;
        default:
            return DefWindowProc (hRelax, uMsg, wParam, lParam);
    }
    return 0;
}*/
//Рисуем интерполированную кривую
/*void print_graph_interp(vector<double> &xAxis, int x_prec, vector<double> &yAxis, int y_prec, HDC &hdc, HPEN &hline,
                       double &min_y, double &max_y, double &min_x, double &max_x, int sx, int sy, const size_t scaleX, const size_t scaleY)
{
    static const HPEN hAxis = (HPEN)GetStockObject(BLACK_PEN);
    static HPEN hOldPen;
    if(xAxis.size() < MIN_SIZE_INTERP || yAxis.size() != xAxis.size() || yAxis.empty())
        return;
    HDC memBit;
    HBITMAP hBitmap;
    int maxX, maxY;
    maxX = GetSystemMetrics(SM_CXSCREEN);
    maxY = GetSystemMetrics(SM_CYSCREEN);
        memBit = CreateCompatibleDC(hdc);                   //Создаем дескриптор экрана пустышку
        hBitmap = CreateCompatibleBitmap(hdc, maxX, maxY);  //Создаем буффер
        SelectObject(memBit, hBitmap);                      //Закрепляем буффер за контекстом
        PatBlt(memBit, 0, 0, maxX, maxY, WHITENESS);        //Рисуем прямоугольник
        transform(memBit, sx, sy);
    static stringstream buff;
    buff << fixed;
    double  z = 0.0, hx = 0.0, hy = 0.0;
    int     x = 0, y = 0;
    size_t  i = 0;
    interp data_interp(xAxis, yAxis, yAxis.size(), gsl_interp_cspline); ///Создаем интерполянт
    set_min_max(xAxis, min_x, max_x);
    set_min_max(yAxis, min_y, max_y);
    hx = (max_x - min_x)/scaleX;
    hy = (max_y - min_y)/scaleY;
    hOldPen = (HPEN)SelectObject(memBit, (HPEN)hAxis);
    ///Размечаем масштаб оси X
    buff << setprecision(x_prec);
    MoveToEx(memBit, 0, 0, NULL);
    LineTo(memBit, GRAPHWIDTH, 0);
    SetTextAlign(memBit, TA_RIGHT | TA_TOP);
    for(z = min_x, i = 0; i <= scaleX; z += hx, i++)
    {
        x = int((z - min_x)*GRAPHWIDTH/(max_x - min_x) + 0.5);
        rewrite(buff) << z;
        TextOut(memBit, x, 0, buff.str().data(), buff.str().length());
        MoveToEx(memBit, x, -10, NULL);
        LineTo(memBit, x, 10);
    }
    ///Размечаем масштаб оси Y
    buff << setprecision(y_prec);
    MoveToEx(memBit, 0, 0, NULL);
    LineTo(memBit, GRAPHWIDTH, 0);
    SetTextAlign(memBit, TA_RIGHT | TA_BOTTOM);
    for (z = min_y, i = 0; i <= scaleY; z += hy, i++)
    {
        y = int((z - min_y)*GRAPHWIDTH/(max_y - min_y) + 0.5);
        rewrite(buff) << z;
        TextOut(memBit, -10, y, buff.str().data(), buff.str().length());
        MoveToEx(memBit, -10, y, NULL);
        LineTo(memBit, 10, y);
    }
    MoveToEx(memBit, 0, 0, NULL);
    LineTo(memBit, 0, GRAPHWIDTH);
    SelectObject(memBit, (HPEN)hOldPen);
    ///Рисуем график
    SelectObject(memBit, hline);
    y = int((yAxis[0] - min_y)*GRAPHWIDTH/(max_y - min_y));
    MoveToEx(memBit, 0, y, NULL);
    double step = 0.1;
    for(double x_0 = min_x; x_0 <= max_x; x_0 += step)
    {
        x = int((x_0 - min_x)*GRAPHWIDTH/(max_x - min_x) + 0.5);
        y = int((data_interp.at(x_0) - min_y)*GRAPHWIDTH/(max_y - min_y) + 0.5);
        LineTo(memBit, x, y);
    }
    transform_back(memBit, sx, sy);
    transform_back(hdc, sx, sy);
    BitBlt(hdc, 0, 0, GRAPHSIZE, GRAPHSIZE, memBit, 0, 0, SRCCOPY);
    DeleteObject(hBitmap); //Удаляем рисунок
    DeleteDC(memBit);
}*/
