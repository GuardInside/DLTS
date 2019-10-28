#include <graph.h>
#include <winfunc.h>

//������� ������� ���� DLTS
/*LRESULT CALLBACK dlwin_proc(HWND hDLTS, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    static int sx, sy;
    static double max_y, min_y, min_x, max_x;
    switch(uMsg)
    {
        case WM_SIZE:
            sx = LOWORD(lParam);
            sy = HIWORD(lParam);
            break;
        case WM_PAINT:
            TryEnterCriticalSection(&csDLTS);
            hdc = BeginPaint(hDLTS, &ps);
                print_graph_dlts(hdc, min_y, max_y, min_x, max_x, sx, sy, 5, 5);
            EndPaint(hDLTS, &ps);
            LeaveCriticalSection(&csDLTS);
            break;
        default:
            return DefWindowProc (hDLTS, uMsg, wParam, lParam);
    }
    return 0;
}*/

//������ DLTS-������
/*void print_graph_dlts(HDC &hdc, double &min_y, double &max_y, double &min_x, double &max_x, int sx, int sy, const size_t scaleX, const size_t scaleY)
{
    const vector<double> &xAxis = xAxisDLTS;
    int x_prec = 2, y_prec = 2;
    if(xAxis.size() < MIN_SIZE_INTERP || yAxisDLST[0].size() != xAxis.size() || yAxisDLST[0].empty())
        return;
    static const HPEN hAxis = (HPEN)GetStockObject(BLACK_PEN);
    HPEN line;
    HDC memBit;
    HBITMAP hBitmap;
    int maxX, maxY;
    maxX = GetSystemMetrics(SM_CXSCREEN);
    maxY = GetSystemMetrics(SM_CYSCREEN);
        memBit = CreateCompatibleDC(hdc);                   //������� ���������� ������ ��������
        hBitmap = CreateCompatibleBitmap(hdc, maxX, maxY);  //������� ������
        SelectObject(memBit, hBitmap);                      //���������� ������ �� ����������
        PatBlt(memBit, 0, 0, maxX, maxY, WHITENESS);        //������ �������������
        transform(memBit, sx, sy);
    static stringstream buff;
    buff << fixed;
    double  z = 0.0, hx = 0.0, hy = 0.0;
    int     x = 0, y = 0;
    size_t  i = 0;
    set_min_max(xAxis, min_x, max_x);
    max_y = min_y = yAxisDLST[0][0];
    for(size_t i = 0; i < CorTime.size(); i++) //�������� �������� � �������
    {
        for(size_t j = 0; j < yAxisDLST[i].size(); j++)
        {
            if(max_y < yAxisDLST[i][j]) max_y = yAxisDLST[i][j];
            if(min_y > yAxisDLST[i][j]) min_y = yAxisDLST[i][j];
        }
    }
    hx = (max_x - min_x)/scaleX;
    hy = (max_y - min_y)/scaleY;
    SelectObject(memBit, (HPEN)hAxis);
    ///��������� ������� ��� X
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
    MoveToEx(memBit, 0, 0, NULL);
    LineTo(memBit, GRAPHWIDTH, 0);
    ///��������� ������� ��� Y
    SetTextAlign(memBit, TA_RIGHT | TA_BOTTOM);
    int scale_z = scale(max(fabs(max_y), fabs(min_y)));
    rewrite(buff) << setprecision(0) << "e-" << log10(scale_z) << setprecision(y_prec);
    TextOut(memBit, 60, scaleY*hy*GRAPHWIDTH/(max_y - min_y)+0.5, buff.str().data(), buff.str().length());
    for (z = min_y, i = 0; i <= scaleY; z += hy, i++)
    {
        y = int((z - min_y)*GRAPHWIDTH/(max_y - min_y) + 0.5);
        rewrite(buff) << z*scale_z;
        TextOut(memBit, -10, y, buff.str().data(), buff.str().length());
        MoveToEx(memBit, -10, y, NULL);
        LineTo(memBit, 10, y);
    }
    MoveToEx(memBit, 0, 0, NULL);
    LineTo(memBit, 0, GRAPHWIDTH);
    for(size_t i = 0; i < CorTime.size(); i++)
    {
        ///������ ������
        line = CreatePen(PS_SOLID, 1, RGB(250.0*(i+1.0)/CorTime.size(), 0, 0));
        SelectObject(memBit, line);
        interp *data_interp = new interp(xAxis, yAxisDLST[i], xAxis.size(), gsl_interp_cspline);
        double step = 0.01;
        y = int((yAxisDLST[i][0] - min_y)*GRAPHWIDTH/(max_y - min_y));
        MoveToEx(memBit, 0, y, NULL);
        for(double x_0 = min_x; x_0 < max_x; x_0 += step)
        {
            x = int((x_0 - min_x)*GRAPHWIDTH/(max_x - min_x) + 0.5);
            y = int((data_interp->at(x_0) - min_y)*GRAPHWIDTH/(max_y - min_y) + 0.5);
            LineTo(memBit, x, y);
        }
        delete data_interp;
        DeleteObject(line);
    }
    transform_back(memBit, sx, sy);
    transform_back(hdc, sx, sy);
    BitBlt(hdc, 0, 0, GRAPHSIZE, GRAPHSIZE, memBit, 0, 0, SRCCOPY);
    DeleteObject(hBitmap); //������� �������
    DeleteDC(memBit);
}*/