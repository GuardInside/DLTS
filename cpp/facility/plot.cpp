#include "graph.h"
#include "variable.h"
#include "gwin.h"
#include "dlts_math.h"
#include "facility.h"
#include "interpolation.h"
#include "ini.h"

bool capture = false; /* ���� ������� ���� */
size_t index = 0;     /* ������ ������������ ���� */
gwin::gVector vPickData1, vPickData2; /* ������ �� ���������� ����������, ��������������� ����� */
VOID dlts_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags);
VOID dlts_OnLButtonUp(HWND hWnd, INT x, INT y, UINT keyFlags);
VOID dlts_OnMouseMove(HWND hwnd, INT x, INT y, UINT keyFlags);
VOID DvToLp(HWND hWnd, POINT *point);

VOID plotDAQ(gwin::gVector *vData1, gwin::gVector *vData2)
{
    gwin::gData(hGraph_DAQ, vData1, vData2);
}

VOID PlotRelax()
{
    if(SavedRelaxations.empty())
        return;
    gwin::gVector vData1;
    gwin::gMulVector vMulData;
    vMulData.push_back(SavedRelaxations[index_relax]);
    stringstream buffer;
    for(size_t i = 0; i < SavedRelaxations[index_relax].size(); i++)
        vData1.push_back(0.001*gate_DAQ + i*(1/rate_DAQ)*1000); /* ms */
    bool UseFitting = 0;
    ini::Settings AnalysisFile{"analysis.ini"};
    AnalysisFile.ReadBool("Fitting", "use_fitting", &UseFitting);
    if(UseFitting)
    {
        int max_iter = 0;
        double abs_error = 1e-6;
        double rel_error = 1e-6;
        double sigma = 1.0;
        AnalysisFile.ReadDouble("Fitting", "abs_error", &abs_error);
        AnalysisFile.ReadDouble("Fitting", "rel_error", &rel_error);
        AnalysisFile.ReadDouble("Fitting", "sigma", &sigma);
        AnalysisFile.ReadInt("Fitting", "max_it", &max_iter);
        /* �������������� ������ */
        vector<double> vSigma(vData1.size(), 1);
        double A = 0.0, dA = 0.0, b = 0.0, db = 0.0, tau = 0.0, dtau = 0.0, ReducedChiSqr = 0.0;
        string strStatus;
        size_t iter = 0;

        int status = get_exponent_fitt(&vData1, &SavedRelaxations[index_relax], &vSigma,
                      &A, &tau, &b, &dA, &db, &dtau, &ReducedChiSqr, &iter, max_iter, abs_error, rel_error, &strStatus);
        if(status)
            MessageBox(NULL, strStatus.data(), "Approximation warning", MB_ICONWARNING);
        gwin::gVector vApproxRelaxation;
        for(const auto &t: vData1)
            vApproxRelaxation.push_back(A * exp(-pow(tau, -1.0) * t) + b);
        vMulData.push_back(vApproxRelaxation);
    /* ����� ������������� */
    }
    if(index_mode == DLTS && !xAxisDLTS.empty())
        rewrite(buffer) << xAxisDLTS[index_relax] << " K";// << endl << iter << endl << A << endl << tau << endl << b;
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
            interp f(xAxisDLTS, yAxisDLTS[i],xAxisDLTS.size(), gsl_interp_linear);
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
                vMulData.push_back(yAxisDLTS[i]);
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
            //gwin::gMulData(hGraph_DLTS, &vData1, &vMulData);
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

VOID DvToLp(HWND hWnd, POINT *point)
{
    static CONST LONG iWndSize = 1200; // ������ ���� � ���������� ��������
    LONG iWidth, iHeight;   // ������� ���������� ������� � ��������
    RECT rt;
    GetClientRect(hWnd, &rt);
    iWidth = abs(abs(rt.right)-abs(rt.left));
    iHeight = abs(abs(rt.top)-abs(rt.bottom));
    point->x = iWndSize * point->x / iWidth - 150;  /* �������� ������ ��������� */
    point->y = iWndSize - iWndSize * point->y / iHeight - 100;
}

VOID dlts_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, INT x, INT y, UINT keyFlags)
{
    static CONST LONG MARK = 10; // ������ (������) ������� ��������� ���� � ���. ��
    POINT point{x, y};
    DvToLp(hWnd, &point); // ��������� ������� � ���. ��.
    RECT rt;
    /* �������� �������� ������� ������� */
    try{
        for(size_t i = 0; i < CorTime.size(); i++)
        {
            gwin::gPoint pt = {vPickData1.at(i), vPickData2.at(i)}; // � ��. ����������� � arb. ��������������
            gwin::gGpToLp(hWnd, &pt); // � ���. ��.
            SetRect(&rt, pt.x-MARK, pt.y-MARK, pt.x+MARK, pt.y+MARK);
            if(PtInRect(&rt, point))
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
            POINT point{x, y};
            DvToLp(hWnd, &point);
            gwin::gPoint p{point.x, point.y};
            gwin::gLpToGp(hWnd, &p);
            vPickData1.at(index) = p.x;
            PlotDLTS();
        }
    } catch(out_of_range  &e){ MessageBox(0, e.what(), "Exception in ReadITS", 0); }
      catch(...){ MessageBox(0, "Some exception.", "Exception in ReadITS", 0); }
}
