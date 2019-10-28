#include <dlts_math.h>

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
double mean(const vector<double> &temp)
{
    if(temp.empty())
        return 0.0;
    size_t i = 0;
    double result = 0.0;
    for(vector<double>::const_reverse_iterator it = temp.crbegin(); it != temp.crend() && i < 1000*aver_time/REFRESH_TIME; it++)
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
    for(vector<double>::const_reverse_iterator it = temp.crbegin(); it != temp.crend() && i < 1000*aver_time/REFRESH_TIME; it++)
    {
        result += pow(*it, 2.0);
        i++;
    }
    result /= i;
    result = result - pow(m, 2.0);
    return pow(result, 0.5);
}
