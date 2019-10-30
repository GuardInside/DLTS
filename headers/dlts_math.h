#ifndef DLTS_MATH_H_INCLUDED
#define DLTS_MATH_H_INCLUDED
#include <fstream>
#include <cmath>
#include <vector>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_integration.h>
#include "variable.h"
#include "interpolation.h"

/* Округлить до n значащих цифр после запятой */
double round(double d, int n);
/* Получить параметры линейной регрессии Y = a + b*X */
void GetParam(const std::vector <double> &X, const std::vector <double> &Y, double &a, double &b);
/* Среднее значение чесел в наборе, начиная последнего */
double mean(const std::vector<double> &temp);
/* Средняя квадратичная флуктуация */
double AverSqFluct(const std::vector<double> &temp);

double double_boxcar(double x, double t1);
double lock_in(double x, double t1);
double sin_w(double x, double t1);
double exp_w(double x, double t1);
#endif // DLTS_MATH_H_INCLUDED
