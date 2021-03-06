#include <numeric>
#include <functional>
#include <algorithm>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_integration.h>
#include "dlts_math.h"

#include <fstream>

bool UseAlphaBoxCar{false};

weight_function get_weight_function(int type)
{
    static weight_function weight_functions[] =
    {
        double_boxcar, lock_in, exp_w, sin_w,
        Hodgart_w, HiRes3_w, HiRes4_w, HiRes5_w,
        HiRes6_w
    };
    return weight_functions[type];
}

/* **************************************** */
/* ������ ��� ����������������� ����������� */
/* **************************************** */

struct params_struct
{
    double Tg;
    double (*weight_f) (double, double);
};
typedef struct params_struct params_t;

double gsl_weight(double x, void *params)
{
    double Tg = ((params_t *) params)->Tg;
    double (*w) (double, double) = ((params_t *) params)->weight_f;

    return w(x, Tg);
}

double W_foo(double Tg, double Tc, double x0 /* "�����������" ����� */, weight_function weight_f)
{
    constexpr static size_t alloc_intervals = 1000;
    gsl_integration_workspace * w
        = gsl_integration_workspace_alloc (alloc_intervals);

    double result_1, result_2, abserror; /* error �� ������������ */
    params_t params = {Tg, weight_f};

    gsl_function F;
    F.function = &gsl_weight;
    F.params = &params;

    double a = Tg, b = Tg + Tc;
    gsl_integration_qags (&F, a, x0, 0, 1e-6, alloc_intervals,
                        w, &result_1, &abserror);
    gsl_integration_qags (&F, x0, b, 0, 1e-6, alloc_intervals,
                        w, &result_2, &abserror);

    gsl_integration_workspace_free (w);
    double S1 = result_1 - weight_f(x0, Tg)*(x0 - a);
    double S2 = weight_f(x0, Tg)*(b - x0) - result_2;
    return (S1 - S2) / Tc;
}

/* ����������� ������� ����� ����������������� ����������� */
double find_middle_x0(double Tg, double Tc, int type)
{

    using namespace std::placeholders;
    weight_function weight_f = get_weight_function(type);
    auto W = bind(W_foo, Tg, Tc, _1, weight_f);

    return dichotomy(Tg, Tg+Tc, 1e-9, W);//GoldSerch(Tg, Tg + Tc, 1e-9, W, MINIMUM); /* ����� �������� */
}

/* **************************************** */

double S_foo(double Tg, double Tc, double tau, weight_function weight_f)
{
    constexpr static int precision = 1e3;
    using namespace std::placeholders;
    using namespace std;

    auto sub_fp = [&](double x, double tau) -> double
    {
        return exp( - x / tau ); /* ��������������� ������� */
    };
    auto sub_f = bind(sub_fp, _1, tau); /* ��������� tau */

    return Integral(Tg, Tc, weight_f, sub_f, precision) / Tc;
}

/*double LockInEq(const double &tau, const double &Tc, const double &Tg)
{
    double a = tau / Tc;
    double b = Tg / Tc;
    return ( (a + b) / (a + b + 1) - exp( - 1 / (2*a) ));
}*/

double find_tau(double Tg, double Tc, int type)
{
    using namespace std::placeholders;
    using namespace std;
    weight_function weight_f = get_weight_function(type);

    if(type == DoubleBoxCar)
    {
        if(!UseAlphaBoxCar && correlation_c != 0.0)
        {
            return Tc / log(1 + correlation_c);
        }
    }

        //return (Tc - Tg ) / log(Tc / Tg);
    /*if(type == LockIn)
    {
        auto D = bind(LockInEq, _1, Tc, Tg);
        /*auto D_abs = [&](const double &tau)
        {
            return fabs(D(tau));
        };
        return GoldSerch(Tg, Tg + Tc, 1e-3, D_abs, MAXIMUM);*/
        /*return dichotomy(Tg, Tg+Tc, 1e-9, D);
    }*/

    auto S = bind(S_foo, Tg, Tc, _1, weight_f);
    auto S_abs = [&](double tau)
    {
        return fabs(S(tau));
    };
    return GoldSerch(0, Tg + Tc, 1e-3, S_abs, MAXIMUM); /* ����� ���������� � ��������� �� ��� */
}

void helper(double Tg, double Tc, int type) /* ��� ������� */
{
    using namespace std::placeholders;
    weight_function weight_f = get_weight_function(type);
    auto S = bind(S_foo, Tg, Tc, _1, weight_f);

    gwin::gVector vData1, vData2, vMark1, vMark2;
    double delta = 1;
    for(int i = 0; i < 200; ++i)
    {
        vData1.push_back(i*delta);
        vData2.push_back(S(i*delta));
    }
    double tau0 = find_tau(Tg, Tc, type);
    double tau1 = tau0 / lw(Tg, Tc, type);
    double tau2 = lw(Tg, Tc, type) * tau1;
    vMark1.push_back(tau0);
    vMark1.push_back(tau1);
    vMark1.push_back(tau2);
    vMark2.push_back(S(tau0));
    vMark2.push_back(S(tau1));
    vMark2.push_back(S(tau2));

    std::ofstream file{FileSavePath + "out.txt"};
    double s_limit = *((minmax_element(vData2.begin(), vData2.end())).second);
    for(auto &it: vData2)
        it /= s_limit;
    for(size_t i = 0; i < vData1.size(); ++i)
    {
        file << std::fixed << std::setprecision(4) << vData1[i] << "\t" << fabs(vData2[i]) << std::endl;
    }
    file.close();
    gwin::gCross(hRelax, &vMark1, &vMark2);
    gwin::gBand(hRelax, 0, 0, 0, 0 );
    gwin::gData(hRelax, &vData1, &vData2);

}

double get_noize_level(double Tg, double Tc, int type)
{
    constexpr static int precision = 1e3;
    weight_function weight_f = get_weight_function(type);

    auto sub_f = [&](double x) -> double
    {
        return weight_f(x, Tg); /* ��������������� ������� */
    };

    return sqrt(Integral(Tg, Tc, weight_f, sub_f, precision));
}

double get_signal_level(double Tg, double Tc, int type)
{
    using namespace std::placeholders;
    weight_function weight_f = get_weight_function(type);

    auto S = bind(S_foo, Tg, Tc, _1, weight_f);

    return S( find_tau(Tg, Tc, type) );
}

double SN(double Tg, double Tc, int type)
{
    return fabs (get_signal_level(Tg, Tc, type) / get_noize_level(Tg, Tc, type) );
}

double lw(double Tg, double Tc, int type)
{
    constexpr static double eps         = 1e-6;
    constexpr static double tau_limit   = 1e3;

    using namespace std::placeholders;
    using namespace std;

    weight_function weight_f = get_weight_function(type);
    auto S              = bind(S_foo, Tg, Tc, _1, weight_f);

    double tau0         = find_tau(Tg, Tc, type);
    double S_max        = S(tau0);
    double S_mid        = S_max / 2.;

    auto shift_S = [&](double val){ return S(val) - S_mid;};

    double tau_min      = dichotomy(0.0, tau0, eps, shift_S);
    double tau_max      = dichotomy(tau0, tau_limit, eps, shift_S);
    /* ��������  ������� ���� */
    for(double tau = tau0; tau < tau_max; tau += 1)
    {
        if(shift_S(tau)*S_max < 0 && tau < tau_max - eps)
        {
            tau_max = dichotomy(tau0, tau, eps, shift_S);
            break;
        }
    }

    /*gwin::gVector vData1, vData2, vMark1, vMark2;
    double delta = 1e-2;
    for(int i = 0; i < 10000; ++i)
    {
        vData1.push_back(i*delta);
        vData2.push_back(shift_S(i*delta));
    }
    vMark1.push_back(tau0);
    vMark1.push_back(tau_min);
    vMark1.push_back(tau_max);
    vMark2.push_back(shift_S(tau0));
    vMark2.push_back(shift_S(tau_min));
    vMark2.push_back(shift_S(tau_max));
    stringstream buff;
    buff << tau0 << "\n" << tau_min << "\n" << tau_max;

    gwin::gAdditionalInfo(hRelax, buff.str());
    gwin::gCross(hRelax, &vMark1, &vMark2);
    gwin::gBand(hRelax, 0, 0, 0, 0 );
    gwin::gData(hRelax, &vData1, &vData2);*/

    return  tau_max / tau_min;
}

int sgn(double val) {
    return (0.0 < val) - (val < 0.0);
}

double round(double d, int n)
{
    return int(d*pow(10,n) + 0.5)/pow(10,n);
}

void GetParam(std::vector<double> const &X, std::vector<double> const &Y, double &a, double &b)
{
    if(X.size() != Y.size())
        return;
    size_t size = X.size();
    double *x = new double[size];
    double *y = new double[size];
    for(size_t i = 0; i < size; i++)
    {
        x[i] = X[i];
        y[i] = Y[i];
    }
    double c0, c1;
    /* ������������ ������� ���������� � �������� ����� ��������� �� ������������ */
    double cov00, cov01, cov11, sumq;
    gsl_fit_linear(x, 1, y, 1, size, &c0, &c1, &cov00, &cov01, &cov11, &sumq);
    a = c0;
    b = c1;
    delete []x;
    delete []y;
}

double Mean(std::vector<double>::const_iterator b,
            std::vector<double>::const_iterator e)
{
    size_t range = e - b;
    if( range <= 0 ) throw math_exception("In function mean(b, e)\n\
                                           begin iterator equal or less than end iterator.");
    return std::accumulate(b, e, static_cast<double>(0.0)) / range;
}

double MeanSquareError(std::vector<double>::const_iterator b,
                       std::vector<double>::const_iterator e)
{
    size_t range = e - b;
    if( range <= 0 ) throw math_exception("In function MeanSquareError(b, e)\n\
                                           begin iterator equal or less than end iterator.");
    double MeanValue = Mean(b, e);
    double MeanFromSquares = std::accumulate(b, e, static_cast<double> (0.0),
                                [](double init, double value){ return init + pow(value, 2);}) / range;
    return round( pow(MeanFromSquares - pow(MeanValue, 2), 0.5), 2);
}
