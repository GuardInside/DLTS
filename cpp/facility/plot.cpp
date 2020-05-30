#include <algorithm>
#include <gsl_bspline.h>
#include <memory>

#include "variable.h"
#include "gwin.h"
#include "dlts_math.h"
#include "facility.h"
#include "ini.h"

#define WHEEL_FORWARD 120
#define WHEEL_REVERSE 65416
#define WHEEL_SCALE   2

bool capture = false; /* Флаг захвата мыши */
size_t index = 0;     /* Индекс захваченного пика */
vector<double> vPickData1, vPickData2; /* Вектор со значениями температур, соответствующих пикам */

VOID dlts_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags);
VOID dlts_OnLButtonUp(HWND hWnd, INT x, INT y, UINT keyFlags);
VOID dlts_OnMouseMove(HWND hWnd, INT x, INT y, UINT keyFlags);

VOID relax_OnMouseWheel(HWND hWnd, WORD key, WORD wheel, INT x, INT y, int &range_index, int &offset_index);

/* ********************************* */
/* Правое верхнее окно с релаксацими */
/* ********************************* */

CRITICAL_SECTION csDAQPaint;
VOID plotDAQ(gwin::gVector *vData1, gwin::gVector *vData2)
{
    if(vData1->empty() || vData2->empty()) return;
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

/* ******************************* */
/* Левое нижнее окно с релаксацими */
/* ******************************* */

VOID PlotRelax()
{
    EnterCriticalSection(&csSavedData);
    if(SavedRelaxations.empty())
    {
        LeaveCriticalSection(&csSavedData);
        return;
    }
    int limit_index_relax = SavedRelaxations.size();
    const size_t n = SavedRelaxations[index_relax.load()].size();

    double capacity = SavedCapacity[index_relax.load()];

    stringstream buffer;
    if(!xAxisDLTS.empty())
    {
        buffer << "T: " << xAxisDLTS[index_relax.load()] << " [K]\n"
               << "C: " << round(capacity, 3) << " [pF]" << endl;
    }

    std::string Title{"Selective recording "},
                xSubscribe{"Time [ms]"},
                ySubscribe{"Voltage [V]"};
    gwin::gAdditionalInfo(hRelax, buffer.str());

    rewrite(buffer) << "[" << index_relax + 1 << "|" << limit_index_relax << "]";
    Title = Title + buffer.str();
    gwin::gTitle(hRelax, Title);
    gwin::gAxisInfo(hRelax, xSubscribe, ySubscribe);

    if(bspline::enable)
    {
        gwin::gVector vBuff;
        gsl_approx approx(TimeAxis, SavedRelaxations[index_relax.load()], bspline::ncoeffs, bspline::order);
        for(size_t i = 0; i < n; ++i)
            vBuff.push_back(approx(TimeAxis[i]));
        gwin::gData(hRelax, &TimeAxis, &vBuff);
    }
    else
    {
        gwin::gData(hRelax, &TimeAxis, &SavedRelaxations[index_relax.load()]);
    }
    LeaveCriticalSection(&csSavedData);
}

/* ** */
/* CV */
/* ** */

VOID PlotCV()
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

/* *********** */
/* DLTS/ITS/AR */
/* *********** */

VOID PlotDLTS(const gwin::gVector &xAxis, const gwin::gMulVector &yAxis)
{
    if(xAxis.empty() || yAxis.empty())
        return;
    if(xAxis.size() < 2 && index_mode.load() == DLTS)
    {
        gwin::gDefaultPlot(hGraph_DLTS, "You should measure the second point\nfor plotting DLTS curves\0");
        return;
    }
    if(index_w4.load() == 2 && !xAxisDLTS.empty() && !SavedCapacity.empty())    /* CV график */
    {
        PlotCV();
    }
    /* Точность определения пика */
    double eps = pow(10, -THERMO_PRECISION);
    if(index_mode.load() == ITS)
        eps = 1e-6; /* Высокая точность определения для ITS */
    /* Поиск максимума методом золотого сечения */
    double a = xAxis.at(0); // Левый край
    double b = xAxis.at(xAxis.size()-1); // Правый край
    size_t n = yAxis.size(); //Количество кривых
    for(size_t i = 0; i < n; i++)
    {
        const gwin::gVector &vData2 = yAxis[i];

        interp f (xAxis, vData2, gsl_interp_linear);
        auto f_abs = [&](double arg){ return fabs(f(arg)); };

        if(menu::automatic == true)
        {
            if(vPickData1.size() < i + 1)
            {
                double tau = GoldSerch(a, b, eps, f_abs, MAXIMUM);
                vPickData1.push_back( tau );
                vPickData2.push_back( f(tau) );
            }
            else
            {
                double tau = GoldSerch(a, b, eps, f_abs, MAXIMUM);
                vPickData1[i] = tau;
                vPickData2[i] = f(tau);
            }
        }
        else if(vPickData1.size() > i)
        {
            vPickData2[i] = f(vPickData1[i]);
        }
    }
    if(index_w4.load() == 0) /* DLTS кривые */
    {
        //gwin::gMulVector vMulData;
        if(capture)
        {
            stringstream buffer;
            buffer << fixed << setprecision(THERMO_PRECISION);

            if(index_mode.load() == DLTS)
            {
                double Tc = CorTc[index];
                double Tpeak = vPickData1.at(index);
                buffer << "T: "  << Tpeak << " [K]\n"
                       << "Tc: " << Tc << " [ms]\n";
                /*if(WeightType == DoubleBoxCar)
                { // Рассчитываем и выводим концентрацию ловушек
                    double Tg = Tc / correlation_c;
                    double tau = find_tau(Tg, Tc, WeightType); //мсек
                    interp C (xAxis, SavedCapacity, gsl_interp_linear);
                    double Smax = vPickData2[index];
                    double c = correlation_c;

                    double beta = c * pow( 1.0 + c, - (1.0 / c + 1.0) );
                    double N = 2 * ( ::dImpurity / (C(Tpeak)) ) * (Smax*beta);
                    if(!menu::divide)
                    {
                        N /= divider(Tc, WeightType);
                    }
                    buffer << scientific << "N: " << fabs(N) << "[cm^-3]" << endl;
                }*/
            }
            else if(index_mode.load() == ITS)
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
        gwin::gBand(hGraph_DLTS, 0, 0, 0.0, 0.0);
        gwin::gPrecision(hGraph_DLTS, THERMO_PRECISION, 3);
        std::string Title{"DLTS curves"}, xSubscribe{"Temperature [K]"}, ySubscribe{"S [pF*ms]"};
        if(index_mode.load() == ITS)
        {
            Title = "ITS curves";
            xSubscribe = "Log of emission rate [1/ms]";
        }
        if(menu::divide || index_mode.load() == ITS)
        {
            ySubscribe = "S [pF]";
        }
        Title = Title + " [" + names_wFunc[WeightType] + "]";
        gwin::gTitle(hGraph_DLTS, Title);
        gwin::gAxisInfo(hGraph_DLTS, xSubscribe, ySubscribe);
        gwin::gCross(hGraph_DLTS, &vPickData1, &vPickData2); /* Индикаторы пика */
        if(index_mode.load() == ITS)
        {
            gwin::gPrecision(hGraph_DLTS, 2, 3);
            gwin::gBand(hGraph_DLTS, floor(a) , ceil(b) , 0, 0);
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

        double B = ::dEfMass * pow(::dFactorG,-1) * B_CONSTANT;
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
                tau = 0.001 * (pow(10, -vPickData1.at(i)));
                T_max = xAxisDLTS[i];
            }
            vData1.push_back(pow(T_max*BOLTZMANN_SI,-1));
            vData2.push_back(log( tau*pow(T_max, 2) ) );
        }
        double a = 0.0, b = 0.0, Energy = 0.0, CrossSection = 0.0;
        GetParam(vData1, vData2, a, b);
        Energy = b * 625e19;  //мэВ
        CrossSection = (exp(-a) / B) * 1e4; //см^2

        for(size_t i = 0; i < vData1.size(); ++i)
        {   /* Формируем оси */
            const double x = vData1[i];
            const double shift = 2*log(BOLTZMANN_SI * x);
            vData3.push_back( (a+b*x) - shift );
            vData2[i] = vData2[i] - shift;
            vData1[i] = x / 625e16;
        }
        stringstream buff;
        buff << setprecision(2) << scientific << "CS: " << CrossSection << " [cm^2]" << endl
             << "R: " << sqrt(CrossSection/PI) << " [cm]" << endl
             << fixed << setprecision(0) << "E: " << Energy << " [meV]" << endl;
        std::string Title{"Arrhenius plot"}, xSubscribe{"1/kT [1/eV]"}, ySubscribe{"ln(e)"};
        gwin::gTitle(hGraph_DLTS, Title);
        gwin::gBand(hGraph_DLTS, 0, 0, 0.0, 0.0);
        gwin::gAxisInfo(hGraph_DLTS, xSubscribe, ySubscribe);
        gwin::gCross(hGraph_DLTS, &vData1, &vData2); /* Экспериментальные точки */
        gwin::gAdditionalInfo(hGraph_DLTS, buff.str(), gwin::ALIGMENT::RIGHT);
        gwin::gPrecision(hGraph_DLTS, THERMO_PRECISION, 3);
        gwin::gData(hGraph_DLTS, &vData1, &vData3);
        /* Сохраняем результаты */
        ::xAxisAr       = move(vData1);
        ::yAxisAr       = move(vData2);
        ::yAxisArMSQ    = move(vData3);
    }
}

BOOL dlts_mouse_message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, dlts_OnLButtonDown);
        //HANDLE_MSG(hWnd, WM_RBUTTONDOWN, dlts_OnRButtonDown);
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
        size_t n = 0;
        switch(index_mode.load())
        {
            case DLTS:
                n = yAxisDLTS.size();
                break;
            case ITS:
                n = yAxisITS.size();
                break;
        }
        for(size_t i = 0; i < n; i++)
        {
            gwin::gPoint pt = {vPickData1.at(i), vPickData2.at(i)}; // В ед. температуры и arb. соответственно
            gwin::gGpToLp(hWnd, &pt); // В лог. ед.
            SetRect(&rt, pt.x-MARK, pt.y-MARK, pt.x+MARK, pt.y+MARK);
            /*stringstream buff;
            buff << "pt" << pt.x << " and " << pt.y << endl;
            buff << point.x << " " << point.y;
            MessageBox(0,buff.str().c_str(),"",0);*/
            menu::automatic = false;
            if(PtInRect(&rt, POINT{(LONG)point.x, (LONG)point.y}))
            {

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
               (index_mode.load() == ITS && point.x >= xAxisITS.at(0) && point.x <= xAxisITS.at(xAxisITS.size()-1)) )
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
