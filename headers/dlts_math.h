#ifndef DLTS_MATH_H_INCLUDED
#define DLTS_MATH_H_INCLUDED
#include <fstream>
#include <cmath>
#include <vector>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_integration.h>
#include "variable.h"
#include "interpolation.h"

/* ����� �������� */
double GoldSerch(double a, double b, double eps, interp &Fun);
/* ��������� �� n �������� ���� ����� ������� */
double round(double d, int n);
/* �������� ��������� �������� ��������� Y = a + b*X */
void GetParam(const std::vector <double> &X, const std::vector <double> &Y, double &a, double &b);
/* ������� �������� ����� � ������, ������� ���������� */
double mean(const std::vector<double> &temp);
/* ������� ������������ ���������� */
double AverSqFluct(const std::vector<double> &temp);

double double_boxcar(double x, double t1);
double lock_in(double x, double t1);
double sin_w(double x, double t1);
double exp_w(double x, double t1);
#endif // DLTS_MATH_H_INCLUDED
