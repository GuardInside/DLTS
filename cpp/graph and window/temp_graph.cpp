#include <graph.h>
#include <winfunc.h>

//ScaleX и ScaleY указывают количество рисок на осях
void print_graph(const vector<double> &xAxis, int x_prec, const vector<double> &yAxis, int y_prec, HDC &hdc, HPEN &hline,
                       double &min_y, double &max_y, double &min_x, double &max_x, int sx, int sy, const size_t scaleX, const size_t scaleY)
{
    static stringstream buff;
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
    buff << fixed;
    double  z = 0.0, hx = 0.0, hy = 0.0;
    int     x = 0, y = 0;
    size_t  i = 0;
    set_min_max(xAxis, min_x, max_x);
    set_min_max(yAxis, min_y, max_y);
    hx = (max_x - min_x)/scaleX;
    hy = (max_y - min_y)/scaleY;
    SelectObject(memBit, (HPEN)GetStockObject(BLACK_PEN));
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

    SelectObject(memBit, (HPEN)hline);
    ///Рисуем график
    y = int((yAxis[0] - min_y)*GRAPHWIDTH/(max_y - min_y));
    MoveToEx(memBit, 0, y, NULL);
    for(i = 1; i < xAxis.size(); i++)
    {
        x = int((xAxis[i] - min_x)*GRAPHWIDTH/(max_x - min_x) + 0.5);
        y = int((yAxis[i] - min_y)*GRAPHWIDTH/(max_y - min_y) + 0.5);
        LineTo(memBit, x, y);
    }
    transform_back(memBit, sx, sy);
    transform_back(hdc, sx, sy);
    BitBlt(hdc, 0, 0, GRAPHSIZE, GRAPHSIZE, memBit, 0, 0, SRCCOPY);
    DeleteObject(hBitmap); //Удаляем рисунок
    DeleteDC(memBit);
}
