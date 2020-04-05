#include <algorithm>

#include "graph.h"
#include "variable.h"
#include "gwin.h"
#include "dlts_math.h"
#include "facility.h"
#include "interpolation.h"
#include "ini.h"

#define WHEEL_FORWARD 120
#define WHEEL_REVERSE 65416
#define WHEEL_SCALE   2

bool capture = false; /* Флаг захвата мыши */
size_t index = 0;     /* Индекс захваченного пика */
gwin::gVector vPickData1, vPickData2; /* Вектор со значениями температур, соответствующих пикам */

VOID dlts_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags);
VOID dlts_OnLButtonUp(HWND hWnd, INT x, INT y, UINT keyFlags);
VOID dlts_OnMouseMove(HWND hWnd, INT x, INT y, UINT keyFlags);

VOID relax_OnMouseWheel(HWND hWnd, WORD key, WORD wheel, INT x, INT y, int &range_index, int &offset_index);

CRITICAL_SECTION csDAQPaint;
VOID plotDAQ(gwin::gVector *vData1, gwin::gVector *vData2)
{
    if(vData1->empty()) return;
    std::string Title, xSubscribe{"Time [ms]"}, ySubscribe{"Voltage [V]"};
    if(TryEnterCriticalSection (&csDAQPaint) == TRUE)
    {
        switch(index_w2)
        {
            case 0:
                Title = "Capacity relaxation in real time";
                break;
            case 1:
                Title = "Injection pulses in real time";
                break;
        }
        gwin::gTitle(hGraph_DAQ, Title);
        gwin::gAxisInfo(hGraph_DAQ, xSubscribe, ySubscribe);
        gwin::gData(hGraph_DAQ, vData1, vData2);
        LeaveCriticalSection(&csDAQPaint);
    }
}

VOID PlotRelax()
{
    EnterCriticalSection(&csSavedData);
    if(SavedRelaxations.empty())
    {
        LeaveCriticalSection(&csSavedData);
        return;
    }
        int limit_index_relax = SavedRelaxations.size();
        gwin::gVector vData2{SavedRelaxations[index_relax.load()]};
        double capacity = (index_mode.load() == DLTS)? SavedCapacity[index_relax.load()] : 1.0;
    LeaveCriticalSection(&csSavedData);
    gwin::gVector vData1;
    gwin::gMulVector vMulData;
    vMulData.push_back(vData2);
    for(size_t i = 0; i < vData2.size(); i++)
        vData1.push_back(0.001*gate_DAQ + i*(1/rate_DAQ)*1000); /* ms */
    stringstream buffer;
    if(AprEnableRelax)
    {
        /* Аппроксимируем кривую */
        double A = 0.0, b = 0.0, tau = 0.0;
        string strStatus;
        vector<double> vSigma(vData1.size(), 1); /* Веса пл умолчанию */
        int status = get_exponent_fitt(&vData1, &vData2, &vSigma,
                      &A, &tau, &b, NULL, NULL, NULL, NULL, NULL, AprIter, AprErr, 0, &strStatus);
        if(status)
            buffer << strStatus << endl;
        gwin::gVector vApproxRelaxation;
        for(const auto &t: vData1)
            vApproxRelaxation.push_back(A * exp(-pow(tau, -1.0) * t) + b);
        vMulData.push_back(vApproxRelaxation);
    /* Конец аппроксимации */
    }
    if(!xAxisDLTS.empty())
    {
        buffer << xAxisDLTS[index_relax.load()] << " [K]" << endl
                << round(capacity, 3) << " [pF]";
    }
    /*else if((index_mode.load() == AITS || index_mode.load() == BITS) && !itsBiasVoltages.empty())
        buffer << fixed << setprecision(2) << "T [K] = " << itsTemperature << endl
                        << setprecision(3) << "Bias [V] = " << itsBiasVoltages[index_relax.load()] << endl << "Amp [V] =  " << itsAmpVoltages[index_relax.load()];*/
    std::string Title{"Selective recording "}, xSubscribe{"Time [ms]"}, ySubscribe{"Voltage [V]"};
    gwin::gAdditionalInfo(hRelax, buffer.str());
    rewrite(buffer) << "[" << index_relax + 1 << "|" << limit_index_relax << "]";
    Title = Title + buffer.str();
    gwin::gTitle(hRelax, Title);
    gwin::gAxisInfo(hRelax, xSubscribe, ySubscribe);
    gwin::gMulData(hRelax, &vData1, &vMulData);
}

VOID PlotDLTS(gwin::gVector &xAxis, gwin::gMulVector &yAxis)
{

    if(xAxisDLTS.size() < 2 && index_mode.load() == DLTS)
    {
        gwin::gDefaultPlot(hGraph_DLTS, "You should measure the second point\nfor plotting DLTS curves\0");
        return;
    }

    if(index_w4.load() == 2)    /* CV график */
    {
        std::string Title{"Capacity recording"}, xSubscribe{"Temperature [K]"}, ySubscribe{"Capacity [pF]"};
        gwin::gTitle(hGraph_DLTS, Title);
        gwin::gAxisInfo(hGraph_DLTS, xSubscribe, ySubscribe);
        gwin::gAdditionalInfo(hGraph_DLTS, "");
        gwin::gPrecision(hGraph_DLTS, THERMO_PRECISION, 3);
        gwin::gBand(hGraph_DLTS, 0, 0, 0.0, 0.0);

        EnterCriticalSection(&csSavedData);
        gwin::gData(hGraph_DLTS, &xAxisDLTS, &SavedCapacity);
        LeaveCriticalSection(&csSavedData);
        return;
    }
    /* Точность определения пика */
    double eps = pow(10, -THERMO_PRECISION);
    if(index_mode.load() == ITS)
        eps = 1e-9;
    /* Поиск максимума методом золотого сечения */
    gwin::gVector vMinData1, vMinData2;
    double dLeftBorder = xAxis.at(0);
    double dRightBorder = xAxis.at(xAxis.size()-1);
    for(size_t i = 0; i < yAxis.size(); i++)
    {
        gwin::gVector &vData2 = yAxis[i];
        /* Определяем ориентацию пика */
        auto Pair = minmax_element(vData2.begin(), vData2.end());
        int extr_type = ( fabs(*Pair.first) >= fabs(*Pair.second) ) ? MINIMUM : MAXIMUM;
        interp f(xAxis, vData2, xAxis.size(), gsl_interp_linear);
        double dMin = 0.0;
        if(auto_peak_search.load() == true)
            dMin = GoldSerch(dLeftBorder, dRightBorder, eps, f, extr_type);
        else
        {
            if(vPickData1.size() < i + 1)
            {
                /* Попадаем сюда, если добавляем новый коррелятор с отключенным автопоиском пика */
                dMin = GoldSerch(dLeftBorder, dRightBorder, eps, f, extr_type);
                vPickData1.push_back(dMin);
            }
            else dMin = vPickData1[i];
        }
        vMinData1.push_back(dMin);
        /* Нормированное значение */
        if(normaliz_dlts.load() == true)
        {
            auto Pair = minmax_element(vData2.begin(), vData2.end());
            double MaxValue = fabs(*Pair.first) >= fabs(*Pair.second) ? fabs(*Pair.first) : fabs(*Pair.second);
            vMinData2.push_back(f.at(dMin) / MaxValue);
        }
        else
        {
            vMinData2.push_back(f.at(dMin));
        }
        /* Сохраняем значения */
        if(auto_peak_search.load() == true)
            vPickData1 =  vMinData1;
        vPickData2 =  vMinData2;
    }
    if(index_w4.load() == 0) /* DLTS кривые */
    {
        gwin::gMulVector vMulData;
        if(capture)
        {
            stringstream buffer;
            buffer << fixed << setprecision(THERMO_PRECISION);

            if(index_mode.load() == DLTS)
            {
                buffer << "T: "  << vPickData1[index] << " [K]\n"
                       << "Tc: " << CorTc[index] << " [ms]\n";
            }
            else
            {
                buffer << "tau: "  << pow(10, -vPickData1[index]) << " [ms]\n"
                       << "T: " << xAxisDLTS[index] << " [K]\n";
            }
            gwin::gAdditionalInfo(hGraph_DLTS, buffer.str());
        }
        else
        {
            gwin::gAdditionalInfo(hGraph_DLTS, "\0");
        }
        if(normaliz_dlts.load() == true)
        {
            for(size_t i = 0; i < CorTc.size(); i++)
            {
                gwin::gVector &vData2 = yAxis[i];
                auto Pair = minmax_element(vData2.begin(), vData2.end());
                double MaxValue = fabs(*Pair.first) >= fabs(*Pair.second) ? fabs(*Pair.first) : fabs(*Pair.second);
                for(auto& e: vData2)
                    e /= MaxValue;
                vMulData.push_back(vData2);
            }
            double sign = 0.0;
            for(auto &e: vPickData2)
                sign += e;
            // Сумма всех пиков больше или меньше нуля //
            if(sign > 0)
                gwin::gBand(hGraph_DLTS, 0, 0, 0.0, 1.0);
            else
                gwin::gBand(hGraph_DLTS, 0, 0, -1.0, 0.0);
            gwin::gPrecision(hGraph_DLTS, 2, 1);
        }
        else
        {
            gwin::gBand(hGraph_DLTS, 0, 0, 0.0, 0.0);
            gwin::gPrecision(hGraph_DLTS, 2, 3);
        }
        std::string Title{"DLTS curves"}, xSubscribe{"Temperature [K]"}, ySubscribe{"S [Arb.]"};
        if(index_mode.load() == ITS)
        {
            Title = "ITS curves";
            xSubscribe = "Log of emission rate [1/ms]";
        }
        Title = Title + " [" + names_wFunc[WeightType] + "]";
        gwin::gTitle(hGraph_DLTS, Title);
        gwin::gAxisInfo(hGraph_DLTS, xSubscribe, ySubscribe);
        gwin::gCross(hGraph_DLTS, &vMinData1, &vMinData2); /* Индикаторы пика */
        if(index_mode.load() == ITS)
        {
            gwin::gBand(hGraph_DLTS, floor(dLeftBorder) , ceil(dRightBorder) , 0, 0);
        }
        gwin::gMulData(hGraph_DLTS, &xAxis, &yAxis);
    }
    else if(index_w4.load() == 1) /* График Аррениуса */
    {
        if(index_mode.load() == DLTS && CorTc.size() < 2)
        {
            gwin::gDefaultPlot(hGraph_DLTS, "You should create a second correlator\nfor plotting Arrhenius graph.\0");
            return;
        }
        else if(index_mode.load() == ITS && yAxis.size() < 2)
        {
            gwin::gDefaultPlot(hGraph_DLTS, "You should measure a second temperature point\nfor plotting Arrhenius graph.\0");
            return;
        }
        double B = 2*sqrt(3.0)*pow(::dFactorG,-1)*(::dEfMass*pow(BOLTZMANN,2)*pow(PLANCKS_CONSTANT_H,-3))*pow(2*PI, 1.5);
        gwin::gVector vData1, vData2, vData3;
        for(size_t i = 0; i < yAxis.size(); i++)
        {
            /** Определяем, какой коррелятор выбран **/
            double Tc = CorTc[i]; // в ms
            double Tg = Tc / correlation_c;
            double tau = 0.0;
            double T_max = 0.0; /* Положение пика */
            if(index_mode.load() == DLTS)
            {
                tau = 0.001 * find_tau(Tg, Tc, WeightType); // в sec
                T_max = vPickData1.at(i);
            }
            else if(index_mode.load() == ITS)
            {
                tau = 0.001 * pow(10, -vPickData1.at(i));
                T_max = xAxisDLTS[i];
            }
            vData1.push_back(pow(T_max*BOLTZMANN,-1));
            vData2.push_back( -log( tau*pow(T_max, 2) ) );
        }
        double a = 0.0, b = 0.0, Energy = 0.0, CrossSection = 0.0;
        GetParam(vData1, vData2, a, b);
        Energy = (-1)*b;
        CrossSection = exp(a)*pow(B, -1);
        for(const auto &x: vData1)
            vData3.push_back(a+b*x);
        gwin::gMulVector vMulData;
        vMulData.push_back(vData2);
        vMulData.push_back(vData3);
        stringstream buff;
        buff << setprecision(2) << scientific << "CS: " << CrossSection << " [m^2]" << endl
             << "R: " << sqrt(CrossSection/PI) << " [m]" << endl
             << fixed << setprecision(0) << "E: " << 1000*Energy*(625e+16) << " [meV]";
        std::string Title{"Arrhenius plot"}, xSubscribe{"1/kT [1/J]"}, ySubscribe{"-ln(tau T^2)"};
        gwin::gTitle(hGraph_DLTS, Title);
        gwin::gAxisInfo(hGraph_DLTS, xSubscribe, ySubscribe);
        gwin::gCross(hGraph_DLTS, &vData1, &vData2); /* Экспериментальные точки */
        gwin::gAdditionalInfo(hGraph_DLTS, buff.str());
        gwin::gBand(hGraph_DLTS, 0, 0, 0.0, 0.0);
        gwin::gPrecision(hGraph_DLTS, THERMO_PRECISION, 3);
        gwin::gData(hGraph_DLTS, &vData1, &vData3);
        /* Сохраняем результаты */
        ::xAxisAr = (vector<double>)vData1;
        ::yAxisAr = (vector<vector<double>>)vMulData;
    }
}

BOOL dlts_mouse_message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, dlts_OnLButtonDown);
        HANDLE_MSG(hWnd, WM_LBUTTONUP, dlts_OnLButtonUp);
        HANDLE_MSG(hWnd, WM_MOUSEMOVE, dlts_OnMouseMove);
    }
    return FALSE;
}

VOID dlts_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags)
{
    if(vPickData1.empty() || vPickData2.empty())
        return;
    static CONST LONG MARK = 10; // Размер (радиус) области выделения пика в лог. ед
    gwin::gPoint point{(double)x, (double)y};
    gwin::gDvToLp(hWnd, &point); // Положение курсора в лог. ед.
    RECT rt;
    /* Добавить проверку полноты вектора */
    try{
        size_t n = (index_mode.load() == DLTS) ? yAxisDLTS.size() : yAxisITS.size();
        for(size_t i = 0; i < n; i++)
        {
            gwin::gPoint pt = {vPickData1.at(i), vPickData2.at(i)}; // В ед. температуры и arb. соответственно
            gwin::gGpToLp(hWnd, &pt); // В лог. ед.
            SetRect(&rt, pt.x-MARK, pt.y-MARK, pt.x+MARK, pt.y+MARK);
            if(PtInRect(&rt, POINT{(LONG)point.x, (LONG)point.y}))
            {
                auto_peak_search.store(false);
                SendMessage(hMainWindow, WM_COMMAND, WM_REFRESH_MENU, 0);
                index = i;
                capture = true;
                SetCapture(hWnd);
                SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
            }
        }
    } catch(out_of_range  &e){ MessageBox(0, e.what(), "Exception in dlts_OnLButtonDown", 0); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in dlts_OnLButtonDown", 0); }
}
VOID dlts_OnLButtonUp(HWND hWnd, INT x, INT y, UINT keyFlags)
{
    if(capture)
    {
        ReleaseCapture();
        capture = false;
        SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
    }
}
VOID dlts_OnMouseMove(HWND hWnd, INT x, INT y, UINT keyFlags)
{
    try
    {
        if(capture)
        {
            gwin::gPoint point{(double)x, (double)y};
            gwin::gDvToLp(hWnd, &point);
            gwin::gLpToGp(hWnd, &point);
            /* Проверка границ окна */
            if((index_mode.load() == DLTS && point.x >= xAxisDLTS.at(0) && point.x <= xAxisDLTS.at(xAxisDLTS.size()-1) )
               ||
               (index_mode.load() == ITS && point.x >= xAxisITS.at(0) && point.x <= xAxisITS.at(xAxisITS.size()-1)))
            {
                vPickData1.at(index) = point.x;
                SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
            }
            else
            {
                POINT CursorPosition;
                GetCursorPos(&CursorPosition);
                SetCursorPos(CursorPosition.x, CursorPosition.y);
            }
        }
    } catch(out_of_range  &e){ MessageBox(0, e.what(), "Exception in dlts_OnMouseMove", 0); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in dlts_OnMouseMove", 0); }
}

BOOL relax_mouse_message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int range_index;
    static int offset_index;
    switch(message)
    {
    case WM_MOUSEWHEEL:
        relax_OnMouseWheel(hWnd, LOWORD(wParam), HIWORD(wParam), LOWORD(lParam), HIWORD(lParam),
                           range_index, offset_index);
        return TRUE;
    }
    return FALSE;
}

BOOL daq_mouse_message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int range_index;
    static int offset_index;
    switch(message)
    {
    case WM_MOUSEWHEEL:
        if(hWnd == hGraph_DAQ && index_w2 == 1)
            return FALSE;
        relax_OnMouseWheel(hWnd, LOWORD(wParam), HIWORD(wParam), LOWORD(lParam), HIWORD(lParam),
                           range_index, offset_index);
        return TRUE;
    }
    return FALSE;
}

VOID relax_OnMouseWheel(HWND hWnd, WORD key, WORD wheel, INT x, INT y, int &range_index, int &offset_index)
{
    const int SIZE_RANGE = 10;
    const double range[SIZE_RANGE] = {10.0, 5.0, 2.0, 1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01};
    double dMinY = 0.0, dMaxY = 0.0;
    double offset = 0.0;
    /* Получаем релевантные значения */
    gwin::gBandGet(hWnd, NULL, NULL, &dMinY, &dMaxY);
    offset = range[range_index] / 10;
    /* Управление рамзером окна отображения */
    if(wheel == WHEEL_FORWARD && key == MK_CONTROL)
    {
        if(range_index < SIZE_RANGE - 1)
        {
            range_index++;
            double new_offset = range[range_index] / 10;
            offset_index = int( fabs(offset) * offset_index / new_offset + 0.5);
        }
    }
    else if (wheel == WHEEL_REVERSE && key == MK_CONTROL)
    {
        if(range_index > 0)
        {
            range_index--;
            double new_offset = range[range_index] / 10;
            offset_index = int( fabs(offset) * offset_index / new_offset - 0.5);
        }
    }
    /* Управление положением окна отображения */
    else if(wheel == WHEEL_FORWARD)
            offset_index++;
    else if (wheel == WHEEL_REVERSE)
            offset_index--;
    /* Рассчитываем положение окна */
    dMinY = -1 * range[range_index] + (range[range_index] / 10) * offset_index;
    dMaxY = range[range_index] + (range[range_index] / 10) * offset_index;
    gwin::gBand(hWnd, 0, 0, dMinY, dMaxY);
    if(hWnd == hRelax)
        PlotRelax();
}
