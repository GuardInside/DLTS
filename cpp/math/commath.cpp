#include <dlts_math.h>
#include "interpolation.h"

/*int sgn(double val) {
    return (0.0 < val) - (val < 0.0);
}*/

#define tay 1.618
double GoldSerch(double a, double b, double eps, interp &Fun)
{
    //std::cout<<"\n\n\n\tМетод золотого сечения:\n";
    double x1, x2, _x, xf1, xf2;
    //int iter(0);
    x1 = a + (b - a) / (tay * tay);
    x2 = a + (b - a) / tay;
    xf1 = Fun.at(x1);
    xf2 = Fun.at(x2);
  P:
    //iter++;
    if(xf1 >= xf2)
    {
        a = x1;
        x1 = x2;
        xf1 = Fun.at(x2);
        x2 = a + (b - a) / tay;
        xf2 = Fun.at(x2);
    }
    else
    {
        b = x2;
        x2 = x1;
        xf2 = xf1;
        x1 = a + (b - a) / (tay * tay);
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

//Получить параметры линейной регрессии Y = a + b*X
void GetParam(const vector <double> &X, const vector <double> &Y, double &a, double &b)
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
    double c0, c1, cov00, cov01, cov11, sumq;
    gsl_fit_linear(x, 1, y, 1, size, &c0, &c1, &cov00, &cov01, &cov11, &sumq);
    a = c0;
    b = c1;
    delete []x;
    delete []y;
}

//Среднее значение чесел в наборе, начиная последнего
double mean(const std::vector<double> &temp)
{
    if(temp.empty())
        return 0.0;
    size_t i = 0;
    double result = 0.0;
    for(vector<double>::const_reverse_iterator it = temp.crbegin(); it != temp.crend() && i < 1000*AVERAGING_TIME/REFRESH_TIME_THERMOSTAT; it++)
    {
        result += *it;
        i++;
    }
    return result/i;
}

//Средняя квадратичная флуктуация
double AverSqFluct(const vector<double> &temp)
{
    if(temp.empty())
        return 0.0;
    size_t i = 0;
    double result = 0.0, m = mean(temp);
    for(vector<double>::const_reverse_iterator it = temp.crbegin(); it != temp.crend() && i < 1000*AVERAGING_TIME/REFRESH_TIME_THERMOSTAT; it++)
    {
        result += pow(*it, 2.0);
        i++;
    }
    result /= i;
    result = result - pow(m, 2.0);
    return pow(result, 0.5) > pow(10,-THERMO_PRECISION) ? pow(result, 0.5) : 0.00;
}
