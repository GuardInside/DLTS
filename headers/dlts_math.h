#ifndef DLTS_MATH_H_INCLUDED
#define DLTS_MATH_H_INCLUDED
#include <cmath>
#include <complex>
#include <vector>
#include <string>
#include <map>
#include <exception>
#include <functional>
#include <gsl/gsl_interp.h>
//Delete this//
#include <windows.h>
#include <sstream>
//#include <functional>
#include "gwin.h"
#include "variable.h"

class interp
{
    private:
        gsl_interp_accel *acc;  //Ускоритель доступа
        gsl_interp *inter_poly; //Указатель на интерполянт
        size_t size;            //Размер сетки
        double *x;              //Указатели на массивы сетки
        double *y;
    public:
        interp(const std::vector<double> &xAxis, const std::vector<double> &yAxis, const gsl_interp_type *type);
        double at(const double &x0);
        double operator()(const double &x0);
        complex<double> operator()(const complex<double> &x0);
        ~interp();
};

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

/* Экспоненцианальная аппроксимация */
int get_exponent_fitt(const std::vector<double> *x, const std::vector<double> *y, const std::vector<double> *sigma,
                      double *A, double *tau, double *b, double *dA, double *dtau, double *db, double *ReducedChiSqr,
                      size_t *iter, size_t max_iter,
                      double epsabs, double epsrel,
                      std::string *strStatusMSG);
/* Округлить до n значащих цифр после запятой */
double round(double d, int n);
/* Получить параметры линейной регрессии Y = a + b*X */
void GetParam(std::vector<double> const &X, std::vector<double> const &Y, double &a, double &b);
/* Среднее значение */
double Mean(std::vector<double>::const_iterator b,
            std::vector<double>::const_iterator e);
/* Средняя квадратичная флуктуация */
double MeanSquareError(std::vector<double>::const_iterator b,
                       std::vector<double>::const_iterator e);
double MeanSquareErrorOfTemp(std::vector<double>::const_iterator b,
                             std::vector<double>::const_iterator e);

typedef enum {DoubleBoxCar, LockIn, ExpW, SinW, Hodgart, HiRes3, HiRes4, HiRes5, HiRes6} weight_t;
typedef double(*weight_function)(double, double);
/* x и t1 должны измеряться в одинаковых единицах */
extern bool UseAlphaBoxCar;    //Ts = alpha * Tc
double divider(const double &Tc, const int &type);
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

void helper(double Tg, double Tc, int type);

/* Поиск постоянной времени релаксации в точке DLTS-максимума */
double find_tau(double Tg /* measure time, ms*/, double Tc /* ms */, int type /* тип коррелятора */);

/* Поиск средней точки x0: если x0 поместить на ось абсцисс, то получим весовую функцию */
double find_middle_x0(double Tg, double Tc, int type);

/* Поиск минимума (sign == 1) или максимума (sign == -1) */
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

template <typename F, typename W>
double Integral(double Tg, double Tc,
                W &w, F &f,
                int precision =  1e5)
{

    const int &n    = precision;
    double &a       = Tg;
    double b        = a + Tc;
    //double k        = Tc / 5.0; // для возрастания точности с ростом промежутка
    double h        = (b - a) / (n);
    double result   = 0.5 * (f(a)*w(a,Tg) + f(b)*w(b,Tg));

    for(int i = 1; i < n; ++i) //Метод трапеций
    {
        double weight = w(a + i*h, Tg);
        if( weight != 0.0)
        {
            result += f(a + i*h)*weight;
        }
    }

    return h*result;
}


/* Вычисляет интеграл, нормированный на промежуток, где f != 0 */
/*template <typename F>
double NIntegral(const double &Tg, const double &Tc,
                double(*w)(double,double), F &f,
                const int precision =  1e5)
{
    return Integral(Tg, Tc, w, f, true);
}*/

/*template <typename F>
double IntegralRomberg(double Tg, double Tc,
                       double(*w)(double,double), F &f,
                       size_t max_steps = 1e3, double acc = 1e-6)
{
    double R1[max_steps], R2[max_steps]; // buffers
    double *Rp = &R1[0], *Rc = &R2[0]; // Rp is previous row, Rc is current row

    double &a       = Tg;
    double b        = a + Tc;
    double h        = (b-a);
    Rp[0] = (f(a) + f(b))*h*.5;

    for (size_t i = 1; i < max_steps; ++i)
    {
      h /= 2.;
      double c = 0;
      size_t ep = 1 << (i-1); //2^(n-1)
      for (size_t j = 1; j <= ep; ++j) {
         c += f(a+(2*j-1)*h) * w(a+(2*j-1)*h, Tg);
      }
      Rc[0] = h*c + .5*Rp[0]; //R(i,0)

      for (size_t j = 1; j <= i; ++j) {
         double n_k = pow(4, j);
         Rc[j] = (n_k*Rc[j-1] - Rp[j-1])/(n_k-1); // compute R(i,j)
      }

      if (i > 1 && fabs(Rp[i-1]-Rc[i]) < acc) {
         return Rc[i-1];
      }

      // swap Rn and Rc as we only need the last row
      double *rt = Rp;
      Rp = Rc;
      Rc = rt;
    }
    return Rp[max_steps-1]; // return our best guess
}*/

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

/*----------------------------------------------------------------
  Laplace Inversion Source Code
  Copyright © 2010 James R. Craig, University of Waterloo
----------------------------------------------------------------*/


#define MAX_LAPORDER 60

typedef complex<double> cmplex;
template <typename FUNC>
double LaplaceInversion(FUNC &F,
                        const double &t,
                        const double tolerance)
{
    using namespace std;
    //--Variable declaration---------------------------------
    int    i,n,m,r;          //counters & intermediate array indices
    int    M(40);            //order of Taylor Expansion (must be less than MAX_LAPORDER)
    double DeHoogFactor(4.0);//DeHoog time factor
    double T;                //Period of DeHoog Inversion formula
    double gamma;            //Integration limit parameter
    cmplex h2M,R2M,z,dz,s;   //Temporary variables

    static cmplex Fctrl [2*MAX_LAPORDER+1];
    static cmplex e     [2*MAX_LAPORDER][MAX_LAPORDER];
    static cmplex q     [2*MAX_LAPORDER][MAX_LAPORDER];
    static cmplex d     [2*MAX_LAPORDER+1];
    static cmplex A     [2*MAX_LAPORDER+2];
    static cmplex B     [2*MAX_LAPORDER+2];

    //Calculate period and integration limits------------------------------------
    T    =DeHoogFactor*t;
    gamma=-0.5*log(tolerance)/T;

    //Calculate F(s) at evalution points gamma+IM*i*PI/T for i=0 to 2*M-1--------
    //This is likely the most time consuming portion of the DeHoog algorithm
    Fctrl[0]=0.5*F(gamma);
    for (i=1; i<=2*M;i++)
    {
    s=cmplex(gamma,i*PI/T);
    Fctrl[i]=F(s);
    }

    //Evaluate e and q ----------------------------------------------------------
    //eqn 20 of De Hoog et al 1982
    for (i=0;i<2*M;i++)
    {
    e[i][0]=0.0;
    q[i][1]=Fctrl[i+1]/Fctrl[i];
    }
    e[2*M][0]=0.0;

    for (r=1;r<=M-1;r++) //one minor correction - does not work for r<=M, as suggested in paper
    {
    for (i=2*(M-r);i>=0;i--)
    {
      if ((i<2*(M-r)) && (r>1)){
      q[i][r]=q[i+1][r-1]*e[i+1][r-1]/e[i  ][r-1];
      }
      e[i][r]=q[i+1][r  ]-q[i  ][r  ]+e[i+1][r-1];
    }
    }

    //Populate d vector-----------------------------------------------------------
    d[0]=Fctrl[0];
    for (m=1;m<=M;m++)
    {
    d[2*m-1]=-q[0][m];
    d[2*m  ]=-e[0][m];
    }

    //Evaluate A, B---------------------------------------------------------------
    //Eqn. 21 in De Hoog et al.
    z =cmplex(cos(PI*t/T),sin(PI*t/T));

    A[0]= 0.0; B[0]=1.0; //A_{-1},B_{-1} in De Hoog
    A[1]=d[0]; B[1]=1.0;
    for (n=2;n<=2*M+1; n++)
    {
    dz  =d[n-1]*z;
    A[n]=A[n-1]+dz*A[n-2];
    B[n]=B[n-1]+dz*B[n-2];
    }

    //Eqn. 23 in De Hoog et al.
    h2M=0.5*(1.0+z*(d[2*M-1]-d[2*M]));
    R2M=-h2M*(1.0-sqrt(1.0+(z*d[2*M]/h2M/h2M)));

    //Eqn. 24 in De Hoog et al.
    A[2*M+1]=A[2*M]+R2M*A[2*M-1];
    B[2*M+1]=B[2*M]+R2M*B[2*M-1];

    //Final result: A[2*M]/B[2*M]=sum [F(gamma+itheta)*exp(itheta)]-------------
    return 1.0/T*exp(gamma*t)*(A[2*M+1]/B[2*M+1]).real();
}




#endif // DLTS_MATH_H_INCLUDED
