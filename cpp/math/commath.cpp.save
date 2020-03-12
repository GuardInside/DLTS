#include <numeric>
#include <exception>
#include "interpolation.h"
#include "dlts_math.h"

#define tau 1.618

int sgn(double val) {
    return (0.0 < val) - (val < 0.0);
}

double GoldSerch(double a, double b, double eps, interp &Fun, int sign)
{
    //std::cout<<"\n\n\n\tМетод золотого сечения:\n";
    double x1, x2, _x, xf1, xf2;
    //int iter(0);
    x1 = a + (b - a) / (tau * tau);
    x2 = a + (b - a) / tau;
    xf1 = sign*Fun.at(x1);
    xf2 = sign*Fun.at(x2);
  P:
    //iter++;
    if(xf1 >= xf2)
    {
        a = x1;
        x1 = x2;
        xf1 = sign*Fun.at(x2);
        x2 = a + (b - a) / tau;
        xf2 = sign*Fun.at(x2);
    }
    else
    {
        b = x2;
        x2 = x1;
        xf2 = xf1;
        x1 = a + (b - a) / (tau * tau);
        xf1 = Fun.at(x1);
    }
    if(fabs(b - a) < eps)
    {
        _x = (a + b) / 2;
        //std::cout<<"Результат:\nx = "<<_x<<"\t\tF(x) = "<<Fun(_x)<<
            //"\nКоличество итераций: "<<iter;
        return _x;
    }
    else
        goto P;
}

double round(double d, int n)
{
    return int(d*pow(10,n) + 0.5)/pow(10,n);
}

void GetParam(vector<double> const &X, vector<double> const &Y, double &a, double &b)
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
    /* Коэффициенты матрицы ковариации и значение суммы квадратов не используются */
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
