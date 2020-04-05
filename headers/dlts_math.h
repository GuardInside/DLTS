#ifndef DLTS_MATH_H_INCLUDED
#define DLTS_MATH_H_INCLUDED
#include <cmath>
#include <vector>
#include <string>
#include <map>
#include <exception>
#include <functional>
#include "interpolation.h"
//Delete this//
#include <windows.h>
#include <sstream>
#include "gwin.h"
#include "variable.h"

int sgn(double val);

class math_exception: public std::exception
{
    public:
        math_exception(const std::string _message) : exception(), message{_message} {}
        const char* what() const noexcept override
        {
            return static_cast<const char*>(message.c_str());
        }
    private:
        std::string message;
};

/* ������������������ ������������� */
int get_exponent_fitt(const std::vector<double> *x, const std::vector<double> *y, const std::vector<double> *sigma,
                      double *A, double *tau, double *b, double *dA, double *dtau, double *db, double *ReducedChiSqr,
                      size_t *iter, size_t max_iter,
                      double epsabs, double epsrel,
                      std::string *strStatusMSG);
/* ��������� �� n �������� ���� ����� ������� */
double round(double d, int n);
/* �������� ��������� �������� ��������� Y = a + b*X */
void GetParam(std::vector<double> const &X, std::vector<double> const &Y, double &a, double &b);
/* ������� �������� */
double Mean(std::vector<double>::const_iterator b,
            std::vector<double>::const_iterator e);
/* ������� ������������ ���������� */
double MeanSquareError(std::vector<double>::const_iterator b,
                       std::vector<double>::const_iterator e);
double MeanSquareErrorOfTemp(std::vector<double>::const_iterator b,
                             std::vector<double>::const_iterator e);

typedef enum {DoubleBoxCar, LockIn, ExpW, SinW, Hodgart, HiRes3, HiRes4, HiRes5, HiRes6} weight_t;
typedef double(*weight_function)(double, double);
/* x � t1 ������ ���������� � ���������� �������� */
extern bool UseAlphaBoxCar;    //Ts = alpha * Tc
double double_boxcar_width(double &Tg);
double double_boxcar(double x, double Tg);

double lock_in(double x, double Tg);
double sin_w(double x, double Tg);

extern std::map<double, double> exp_shift;
double exp_w(double x, double Tg);

double Hodgart_w(double x, double Tg);
double HiRes3_w(double x, double Tg);
double HiRes4_w(double x, double Tg);
double HiRes5_w(double x, double Tg);
double HiRes6_w(double x, double Tg);

double get_noize_level(double Tg, double Tc, int type);
double get_signal_level(double Tg, double Tc, int type);
double SN(double Tg, double Tc, int type);
double lw(double Tg, double Tc, int type);
weight_function get_weight_function(int type);

double helper(double Tg, double Tc, int type);

/* ����� ���������� ������� ���������� � ����� DLTS-��������� */
double find_tau(double Tg /* measure time, ms*/, double Tc /* ms */, int type /* ��� ����������� */);

/* ����� ������� ����� x0: ���� x0 ��������� �� ��� �������, �� ������� ������� ������� */
double find_middle_x0(double Tg, double Tc, int type);

/* ����� �������� (sign == 1) ��� ��������� (sign == -1) */
#define MAXIMUM     -1
#define MINIMUM      1
#define tau         1.618
template <typename func_t>
double GoldSerch(double a, double b, double eps, func_t &Fun, int sign = 1)
{
    if(a > b)
        std::swap(a, b);
    double x1, x2, _x, xf1, xf2;
    //int iter(0);
    x1 = a + (b - a) / (tau * tau);
    x2 = a + (b - a) / tau;
    xf1 = sign*Fun(x1);
    xf2 = sign*Fun(x2);
  P:
    //iter++;
    if(xf1 >= xf2)
    {
        a = x1;
        x1 = x2;
        xf1 = sign*Fun(x2);
        x2 = a + (b - a) / tau;
        xf2 = sign*Fun(x2);
    }
    else
    {
        b = x2;
        x2 = x1;
        xf2 = xf1;
        x1 = a + (b - a) / (tau * tau);
        xf1 = sign*Fun(x1);
    }
    if(fabs(b - a) < eps)
    {
        _x = (a + b) / 2;
        return _x;
    }
    else
        goto P;
}
#undef tau

template <typename F>
double Integral(double Tg, double Tc,
                double(*w)(double,double), F &f,
                const int precision =  1e5,
                std::vector<double> *pts = nullptr)
{
    const int &n    = precision;
    double &a       = Tg;
    double b        = a + Tc;
    double h        = (b - a) / n;
    double result   = 0.5 * (f(a)*w(a,Tg) + f(b)*w(b,Tc));

    for(int i = 1; i < n; ++i) //����� ��������
    {
        double weight = w(a + i*h, Tg);
        if( weight != 0.0)
            {
                result += f(a + i*h)*weight;
            }
    }
    return h*result;
}

template <typename func_t>
double dichotomy(double infinum, double supremum, double eps, func_t &Fun) {
   double x;
   while (supremum - infinum > eps) {
      x = (infinum + supremum) / 2;
      if (Fun(supremum) * Fun(x) < 0)
         infinum = x;
      else
         supremum = x;
   }
   return (infinum + supremum) / 2;
}

#endif // DLTS_MATH_H_INCLUDED
