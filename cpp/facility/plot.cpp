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

bool capture = false; /* ���� ������� ���� */
size_t index = 0;     /* ������ ������������ ���� */
gwin::gVector vPickData1, vPickData2; /* ������ �� ���������� ����������, ��������������� ����� */

VOID dlts_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags);
VOID dlts_OnLButtonUp(HWND hWnd, INT x, INT y, UINT keyFlags);
VOID dlts_OnMouseMove(HWND hWnd, INT x, INT y, UINT keyFlags);

VOID relax_OnMouseWheel(HWND hWnd, WORD key, WORD wheel, INT x, INT y, int &range_index, int &offset_index);

VOID plotDAQ(gwin::gVector *vData1, gwin::gVector *vData2)
{
    if(vData1->empty()) return;

    gwin::gData(hGraph_DAQ, vData1, vData2);
}

VOID PlotRelax()
{
    if(SavedRelaxations.empty())
        return;
    gwin::gVector vData2{SavedRelaxations[index_relax]};
    /*for(auto &Y: vData2)
        Y = CONST_02_SULA * Y * RANGE_SULA / PRE_AMP_GAIN_SULA;*/
    gwin::gVector vData1;
    gwin::gMulVector vMulData;
    vMulData.push_back(vData2);
    stringstream buffer;
    for(size_t i = 0; i < vData2.size(); i++)
        vData1.push_back(0.001*gate_DAQ + i*(1/rate_DAQ)*1000); /* ms */
    if(AprEnableRelax)
    {
        /* �������������� ������ */
        double A = 0.0, b = 0.0, tau = 0.0;
        string strStatus;
        vector<double> vSigma(vData1.size(), 1); /* ���� �� ��������� */
        int status = get_exponent_fitt(&vData1, &vData2, &vSigma,
                      &A, &tau, &b, NULL, NULL, NULL, NULL, NULL, AprIter, AprErr, 0, &strStatus);
        if(status)
            MessageBox(NULL, strStatus.data(), "Approximation warning", MB_ICONWARNING);
        gwin::gVector vApproxRelaxation;
        for(const auto &t: vData1)
            vApproxRelaxation.push_back(A * exp(-pow(tau, -1.0) * t) + b);
        vMulData.push_back(vApproxRelaxation);
    /* ����� ������������� */
    }
    if(index_mode == DLTS && !xAxisDLTS.empty())
        rewrite(buffer) << xAxisDLTS[index_relax] << " K";
    else if(index_mode == ITS && !itsBiasVoltages.empty())
        rewrite(buffer) << fixed << setprecision(2) << "T [K] = " << itsTemperature << endl
                        << setprecision(3) << "Bias [V] = " << itsBiasVoltages[index_relax] << endl << "Amp [V] =  " << itsAmpVoltages[index_relax];
    gwin::gAdditionalInfo(hRelax, buffer.str());
    gwin::gMulData(hRelax, &vData1, &vMulData);
}

VOID PlotDLTS()
{
    if(index_mode == ITS)
    {
        gwin::gDefaultPlot(hGraph_DLTS, "\0");
        return;
    }
    if(xAxisDLTS.size() < 2)
    {
        if(xAxisDLTS.size() == 1)
            gwin::gDefaultPlot(hGraph_DLTS, "You should measure the second point.");
        return;
    }
    if(index_plot_DLTS == 0 || index_plot_DLTS == 1)
    {
        /* �������� ����������� ���� */
        static const double eps = pow(10, -THERMO_PRECISION);
        /* ����� ��������� ������� �������� ������� */
        gwin::gVector vMinData1, vMinData2;
        double dLeftBorder = xAxisDLTS.at(0);
        double dRightBorder = xAxisDLTS.at(xAxisDLTS.size()-1);
        for(size_t i = 0; i < CorTime.size(); i++)
        {
            gwin::gVector vData2{yAxisDLTS[i]};
            interp f(xAxisDLTS, vData2,xAxisDLTS.size(), gsl_interp_linear);
            double dMin = 0.0;
            if(auto_peak_search == true)
                dMin = GoldSerch(dLeftBorder, dRightBorder, eps, f);
            else
            {
                if(vPickData1.size() < i + 1)
                {
                    /* �������� ����, ���� ��������� ����� ���������� � ����������� ����������� ���� */
                    dMin = GoldSerch(dLeftBorder, dRightBorder, eps, f);
                    vPickData1.push_back(dMin);
                }
                else dMin = vPickData1.at(i);
            }
            vMinData1.push_back(dMin);
            vMinData2.push_back(f.at(dMin));
            /* ��������� �������� */
            if(auto_peak_search == true)
                vPickData1 = vMinData1;
            vPickData2 = vMinData2;
        }
        if(index_plot_DLTS == 0) /* DLTS ������ */
        {
            gwin::gMulVector vMulData;
            for(size_t i = 0; i < CorTime.size(); i++)
            {
                gwin::gVector vData2{yAxisDLTS[i]};
                vMulData.push_back(vData2);
            }
            if(capture)
            {
                stringstream buffer;
                buffer << fixed << setprecision(THERMO_PRECISION);
                buffer << vPickData1.at(index) << " K" << endl;
                gwin::gAdditionalInfo(hGraph_DLTS, buffer.str());
            }
            else
            {
                gwin::gAdditionalInfo(hGraph_DLTS, "\0");
            }
            gwin::gCross(hGraph_DLTS, &vMinData1, &vMinData2); /* ���������� ���� */
            gwin::gMulData(hGraph_DLTS, &xAxisDLTS, &vMulData);
        }
        else if(index_plot_DLTS == 1) /* ������ ��������� */
        {
            double B = sqrt(3.0)*pow(2.0, 2.5)*pow(::dFactorG,-1)*(::dEfMass*pow(BOLTZMANN,2)*pow(PLANCKS_CONSTANT_H,-3))*pow(PI, 1.5);
            gwin::gVector vData1, vData2, vData3;
            for(size_t i = 0; i < CorTime.size(); i++)
            {
                double T_max = vMinData1.at(i); /* ��������� ���� */
                /** ����������, ����� ���������� ������ **/
                double t1 = CorTime[i]*0.001; // � ��������
                double t2 = t1*correlation_c;
                double tau = 0.0;
                switch(CorType)
                {
                    case DoubleBoxCar:
                        tau = (t2-t1)/log(correlation_c);
                        break;
                    case SinW:
                        tau = 0.4*(t2-t1); // ������ ������ T_c = t2-t1
                        break;
                    default:
                        {
                            gwin::gDefaultPlot(hGraph_DLTS, "The tau for the cor. isn't define.");
                            return;
                        }
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
            buff << setprecision(2) << scientific << "CS: " << CrossSection << " m^2" << endl
                 << "R: " << sqrt(CrossSection/PI) << " m" << endl
                 << fixed << setprecision(3) << "E: " << Energy*(625e+16) << " eV";
            gwin::gCross(hGraph_DLTS, &vData1, &vData2); /* ����������������� ����� */
            gwin::gAdditionalInfo(hGraph_DLTS, buff.str());
            gwin::gData(hGraph_DLTS, &vData1, &vData3);
            /* ��������� ���������� */
            ::xAxisAr = (vector<double>)vData1;
            ::yAxisAr = (vector<vector<double>>)vMulData;
        }
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
    static CONST LONG MARK = 10; // ������ (������) ������� ��������� ���� � ���. ��
    gwin::gPoint point{(double)x, (double)y};
    gwin::gDvToLp(hWnd, &point); // ��������� ������� � ���. ��.
    RECT rt;
    /* �������� �������� ������� ������� */
    try{
        for(size_t i = 0; i < CorTime.size(); i++)
        {
            gwin::gPoint pt = {vPickData1.at(i), vPickData2.at(i)}; // � ��. ����������� � arb. ��������������
            gwin::gGpToLp(hWnd, &pt); // � ���. ��.
            SetRect(&rt, pt.x-MARK, pt.y-MARK, pt.x+MARK, pt.y+MARK);
            if(PtInRect(&rt, POINT{(LONG)point.x, (LONG)point.y}))
            {
                auto_peak_search = false;
                index = i;
                capture = true;
                SetCapture(hWnd);
                PlotDLTS();
            }
        }
    } catch(out_of_range  &e){ MessageBox(0, e.what(), "Exception in ReadITS", 0); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in ReadITS", 0); }
}
VOID dlts_OnLButtonUp(HWND hWnd, INT x, INT y, UINT keyFlags)
{
    if(capture)
    {
        ReleaseCapture();
        capture = false;
        PlotDLTS();
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
            vPickData1.at(index) = point.x;
            PlotDLTS();
        }
    } catch(out_of_range  &e){ MessageBox(0, e.what(), "Exception in ReadITS", 0); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in ReadITS", 0); }
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
    /* �������� ����������� �������� */
    gwin::gBandGet(hWnd, NULL, NULL, &dMinY, &dMaxY);
    offset = range[range_index] / 10;
    /* ���������� �������� ���� ����������� */
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
    /* ���������� ���������� ���� ����������� */
    else if(wheel == WHEEL_FORWARD)
            offset_index++;
    else if (wheel == WHEEL_REVERSE)
            offset_index--;
    /* ������������ ��������� ���� */
    dMinY = -1 * range[range_index] + (range[range_index] / 10) * offset_index;
    dMaxY = range[range_index] + (range[range_index] / 10) * offset_index;
    gwin::gBand(hWnd, 0, 0, dMinY, dMaxY);
    if(hWnd == hRelax)
        PlotRelax();
}
