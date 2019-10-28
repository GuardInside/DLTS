#include <graph.h>
#include <winfunc.h>

LRESULT CALLBACK graph_proc(HWND, UINT, WPARAM, LPARAM);

BOOL ArrheniusWindow_OnCreate(HWND, LPCREATESTRUCT);
VOID ArrheniusWindow_OnCommand(HWND, int, HWND, UINT);

/* Оконная функция окна с графиком Аррениуса*/
LRESULT CALLBACK arwin_proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hwnd, WM_CREATE, ArrheniusWindow_OnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND, ArrheniusWindow_OnCommand);
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}

BOOL ArrheniusWindow_OnCreate(HWND hwnd, LPCREATESTRUCT)
{
    CreateClass(hInst, "ArrheniusGraph", graph_proc);
    hGraph_Arrhenius = CreateWindow("ArrheniusGraph", NULL, WS_CHILD | WS_DLGFRAME | WS_VISIBLE, 10, 16, 385, 330, hwnd, 0, hInst, NULL);
    return TRUE;
}

void ArrheniusWindow_OnCommand(HWND hwnd, int ID, HWND, UINT)
{
    switch(ID)
    {
        case ID_BUTTON_CLOSE:
            DestroyWindow(hwnd);
        break;
        case REFRESH_INFO:
            stringstream buff;
            //Вывести значение энергии связи
            rewrite(buff) << "E = " << setprecision(2) << fixed << Energy << " эВ";
            SetDlgItemText(hwnd, ID_STR_ARRHENIUS_BINDING_ENERGY, buff.str().data());
            //Вывести значение сечения захвата
            rewrite(buff) << "S = " << scientific << CrossSection << " см^2";
            SetDlgItemText(hwnd, ID_STR_ARRHENIUS_CROSS_SECTION, buff.str().data());
        break;
    }
}

//Оконная функция окна графика Аррениуса
LRESULT CALLBACK graph_proc(HWND hChildWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    static HPEN hline;
    static int sx, sy;
    static double a = 0.0, b = 0.0;
    /* B - табличное значение */
    static const double B = sqrt(3)*pow(2, 5/2)/G_CONSTANT*(MASS_ELECTRON*pow(BOLTZMANN,2)/pow(PLANCKS_CONSTANT_H,3))*pow(PI, 3/2);
    static double max_y, min_y, min_x, max_x;
    static stringstream buff;
    static vector<double> xAxis, yAxis;
    switch(uMsg)
    {
        case WM_CREATE:
            hline = CreatePen(PS_SOLID, 1, RGB(150, 0, 200));
            buff << setprecision(4);
            break;
        case WM_SIZE:
            sx = LOWORD(lParam);
            sy = HIWORD(lParam);
            InvalidateRect(hChildWnd, NULL, TRUE);
            break;
        case WM_PAINT:
            if(xAxisDLTS.empty())
                break;
            vector<double>().swap(xAxis);
            vector<double>().swap(yAxis);
            static stringstream bu; //отладка
            for(size_t i = 0; i < CorTime.size(); i++)
            {
                size_t max_value_index = 0;
                for(size_t j = 1; j < yAxisDLTS[i].size(); j++)
                if(fabs(yAxisDLTS[i][j]) > fabs(yAxisDLTS[i][max_value_index]))
                    max_value_index = j;
                double t1 = CorTime[i]/1000.0; // в секундах
                double t2 = t1*correlation_c;
                double tau = 0.0;
                switch(CorType)
                {
                    case DoubleBoxCar:
                        tau = (t2-t1)/log(correlation_c);
                    break;
                    /* Период синуса T_c = t2-t1 */
                    case SinW:
                        tau = 0.4*(t2-t1);
                        break;
                }
                double T_max = xAxisDLTS[max_value_index];
                xAxis.push_back(pow(T_max*BOLTZMANN,-1.0));
                yAxis.push_back( -log( tau*pow(T_max,2.0) ) );
            }
            GetParam(xAxis, yAxis, a, b);
            Energy = (-1)*b;
            CrossSection = exp(b)/B;
            SendMessage(GetParent(hChildWnd), WM_COMMAND, REFRESH_INFO, 0);
            hdc = BeginPaint(hChildWnd, &ps);
                print_graph_arhenius(xAxis, 3, yAxis, 2, hdc, hline, min_y, max_y, min_x, max_x, sx, sy, 3, 5, a, b);
            EndPaint(hChildWnd, &ps);
            break;
        default:
            return DefWindowProc (hChildWnd, uMsg, wParam, lParam);
    }
    return 0;
}

//ScaleX и ScaleY указывают количество рисок на осях
void print_graph_arhenius(vector<double> &xAxis, int x_prec, vector<double> &yAxis, int y_prec, HDC &hdc, HPEN &hline,
                       double &min_y, double &max_y, double &min_x, double &max_x, int sx, int sy, const size_t scaleX, const size_t scaleY, const double a, const double b)
{
    static const HPEN hAxis = (HPEN)GetStockObject(BLACK_PEN);
    HPEN hOldPen;
    if(yAxis.size() != xAxis.size() || yAxis.empty())
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
    set_min_max(xAxis, min_x, max_x);
    set_min_max(yAxis, min_y, max_y);
    hx = (max_x - min_x)/scaleX;
    hy = (max_y - min_y)/scaleY;
    hOldPen = (HPEN)SelectObject(memBit, (HPEN)hAxis);
    //Размечаем масштаб оси X
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
    //Размечаем масштаб оси Y
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
    HBRUSH hBrush = CreateSolidBrush(RGB(255,0,0));
    SelectObject(memBit, hBrush);
    ///Рисуем бары
    for(i = 0; i < xAxis.size(); i++)
    {
        x = int((xAxis[i] - min_x)*GRAPHWIDTH/(max_x - min_x) + 0.5);
        y = int((yAxis[i] - min_y)*GRAPHWIDTH/(max_y - min_y) + 0.5);
        Ellipse(memBit, x-GRAPHWIDTH/40, y+GRAPHWIDTH/40, x+GRAPHWIDTH/40, y-GRAPHWIDTH/40);
    }
    ///Рисуем прямую
    y = int((a+b*min_x - min_y)*GRAPHWIDTH/(max_y - min_y) + 0.5);
    MoveToEx(memBit, 0, y, NULL);
    x = int((max_x - min_x)*GRAPHWIDTH/(max_x - min_x) + 0.5);
    y = int((a+b*max_x - min_y)*GRAPHWIDTH/(max_y - min_y) + 0.5);
    LineTo(memBit, x, y);

    DeleteObject(hBrush);
    transform_back(memBit, sx, sy);
    transform_back(hdc, sx, sy);
    BitBlt(hdc, 0, 0, GRAPHSIZE, GRAPHSIZE, memBit, 0, 0, SRCCOPY);
    DeleteObject(hBitmap); //Удаляем рисунок
    DeleteDC(memBit);
}
