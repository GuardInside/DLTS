#include <dlts_math.h>
#include <algorithm>
#include "facility.h"
#include "gwin.h"
#include "ini.h"
#include <gsl_bspline.h>

//��� �������
#include <random>

void AddPoint_with_interp(const vector<double> *vRelaxation, const double temp);
void AddPoint_with_approx(const vector<double> *vRelaxation, const double temp);

/* t2 = Tg + Tc = Tg + c*Tg = Tg(1 + c) */
double sin_w(double x, double Tg)
{
    double t2 = Tg*(1 + correlation_c);
    if(x >= Tg && x <= t2)
        return sin((x-Tg)*2*PI/(correlation_c*Tg));
    return 0.0;
}

double Hodgart_w(double x, double Tg)
{
    double t2 = Tg * (1 + correlation_c);
    double step_1 = Tg * (1 + 0.1 * correlation_c); /* ������ ���� ����, t1 = Tg + 0.1 Tc */
    double step_2 = step_1 + 0.15 * correlation_c * Tg;
    double a[] = {4.7, 1.45, -0.92};

    if(x >= Tg && x <= step_1)
        return a[0];
    else if(x > step_1 && x <= step_2)
        return a[1];
    else if(x > step_2 && x <= t2)
        return a[2];
    return 0.0;
}

double HiRes3_w(double x, double Tg)
{
    double t2 = Tg * (1 + correlation_c);
    double step = Tg * (1. / 3) * correlation_c;
    double t[] = {(Tg + step), (Tg + 2*step)};
    double a[] = {-1, 2, -1};
    if(x >= Tg && x <= t[0])
        return a[0];
    else if(x > t[0] && x <= t[1])
        return a[1];
    else if(x > t[1] && x <= t2)
        return a[2];
    return 0.0;
}

double HiRes4_w(double x, double Tg)
{
    double t2 = Tg * (1 + correlation_c);
    double step = Tg * 0.25 * correlation_c;
    double t[] = {Tg + step, Tg + 2*step, Tg + 3*step};
    double a[] = {-1, 3, -3, 1};
    if(x >= Tg && x <= t[0])
        return a[0];
    else if(x > t[0] && x <= t[1])
        return a[1];
    else if(x > t[1] && x <= t[2])
        return a[2];
    else if(x > t[2] && x <= t2)
        return a[3];
    return 0.0;
}

double HiRes5_w(double x, double Tg)
{
    double t2 = Tg * (1 + correlation_c);
    double step = Tg * 0.2 * correlation_c;
    double t[] = {Tg + step, Tg + 2*step, Tg + 3*step, Tg + 4*step};
    double a[] = {-1, 4, -6, 4, -1};
    if(x >= Tg && x <= t[0])
        return a[0];
    else if(x > t[0] && x <= t[1])
        return a[1];
    else if(x > t[1] && x <= t[2])
        return a[2];
    else if(x > t[2] && x <= t[3])
        return a[3];
    else if(x > t[3] && x <= t2)
        return a[4];
    return 0.0;
}
//0,875 0,21875 0,015625
double HiRes6_w(double x, double Tg)
{
    double t2 = Tg * (1 + correlation_c);
    double step =  (1./15) * Tg * correlation_c;
    double a[] = {1, -7./8, 7./32, -1./64};
    double t[] = {Tg + 1*step, Tg + 3*step, Tg + 7*step};
    if(x >= Tg && x <= t[0])
        return a[0];
    else if(x > t[0] && x <= t[1])
        return a[1];
    else if(x > t[1] && x <= t[2])
        return a[2];
    else if(x > t[2] && x <= t2)
        return a[3];
    return 0.0;
}

map<double, double> exp_shift;
double exp_w(double x, double Tg)
{
    if(exp_shift.find(Tg) == exp_shift.end())
    {
        exp_shift[Tg] = exp_w( find_middle_x0(Tg, Tg*correlation_c, ExpW), Tg );
    }

    double Tc = correlation_c*Tg;

    if(x >= Tg && x <= Tg + Tc)
        return exp(- x / ( 0.4 * Tc )) - exp_shift[Tg];
    return 0.0;
}

double lock_in(double x, double Tg)
{
    double t2 = Tg*(1 + correlation_c);
    double t_0 = 0.5 * (correlation_c * Tg) + Tg;
    if(x >= Tg && x <= t_0)
        return 1;
    if(x > t_0 && x <= t2)
        return -1;
    return 0.0;
}

double double_boxcar_width(double &Tg)
{
    return (UseAlphaBoxCar) ? correlation_alpha * Tg * correlation_c
                                   : correlation_width/1000.0;
}
double double_boxcar(double x, double Tg)
{
    double t2 = Tg*(1 + correlation_c);     //������ ������� ��������
    double dt = double_boxcar_width(Tg);    //������ ��������� � ��
    if(x >= Tg && x <= Tg+dt)
        return 1;
    if(x >= t2-dt && x <= t2)
        return -1;
    return 0.0;
}

double test_func(double x, double t1)
{
    return 1.0;
}

double divider(const double &Tc, const int &type)
{
    if(type == DoubleBoxCar)
    {
        double Tg = Tc / correlation_c;
        return 2 * double_boxcar_width(Tg);
    }
    return Tc;
}

template <typename RELAXATION>
void AddPoint(RELAXATION &f /* ����������*/, double capacity, size_t offset)
{
    double (*w) (double, double) = get_weight_function(WeightType);
    if(index_mode.load() == DLTS)
    {   /* ��������� ������ DLTS */
        for(size_t c = 0; c < CorTc.size(); ++c)          /* ����������� �� ���� ������������ */
        {
            double Tc = CorTc[c];
            double Tg = Tc / correlation_c;
            double I = Integral(Tg, Tc, w, f);

            /* ��������� �� ����� �� ������ �� ���� */
            VoltageToCapacity(&I);

            if(menu::divide)
            {
                I /= divider(Tc, WeightType);
            }

            yAxisDLTS[c].insert(yAxisDLTS[c].begin() + offset, I);
        }
    }
    else if(index_mode.load() == ITS)
    {   /* ��������� ������ ITS */
        constexpr static double Tc_delta = 0.1; /* ms */

        double Tc_begin = ( 0.001 * gate_DAQ ) * correlation_c;
        double Tc_end   = correlation_c * measure_time_DAQ / (correlation_c + 1);

        int min_i = ceil(Tc_begin / Tc_delta);
        int max_i = floor(Tc_end / Tc_delta);

        vector<double> vBuff;
        vBuff.reserve(max_i - min_i);

        for(int i = max_i, j = 0; i > min_i; --i)
        {
            PostMessage(hProgress, PBM_SETPOS, 100.0*(++j)/max_i, 0);

            double Tc = i*Tc_delta;
            double Tg = Tc / correlation_c;

            double I = Integral(Tg, Tc, w, f);

            VoltageToCapacity(&I);

            I /= divider(Tc, WeightType);

            vBuff.push_back(I);
        }
        yAxisITS.insert(yAxisITS.begin() + offset, vBuff);
        PostMessage(hProgress, PBM_SETPOS, 0, 0);

        /* ��������� ��� log �� ������� ������� */
        if(xAxisITS.empty())
        {
            xAxisITS.reserve(max_i-min_i);
            for(int i = max_i; i > min_i; --i)
            {
                xAxisITS.push_back(-log10((i*Tc_delta) ) );
            }
        }
    }
}

void AddPointToDLTS(const vector<double> *vRelaxation, const double temp, const double capacity)
{
    /* �������� �� ������������� ����� � ����� �������� ��� ������� ����� */
    size_t offset = 0;
    for(const auto& T: xAxisDLTS)
    {
        if(temp > T) offset++;
        else if(temp == T)
            return;
    }
    xAxisDLTS.insert(xAxisDLTS.begin() + offset, temp);

    if(bspline::enable)
    { /* ���������� ������������� ��� ���������� DLTS- ITS-������ */
        gsl_approx f(TimeAxis, *vRelaxation);
        vector<double> vBuff;
        vBuff.reserve(vRelaxation->size());
        for(const auto &t: TimeAxis)
        {
            vBuff.push_back( f(t) );
        }
        interp foo(TimeAxis, vBuff, gsl_interp_linear);
        AddPoint(foo, capacity, offset);
    }
    else
    { /* ���������� ����������� ��� ���������� DLTS- ITS-������ */
        interp f(TimeAxis, *vRelaxation, gsl_interp_linear);
        AddPoint(f, capacity, offset);
    }
}
