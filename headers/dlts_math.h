#ifndef DLTS_MATH_H_INCLUDED
#define DLTS_MATH_H_INCLUDED
#include <fstream>
#include <variable.h>
#include <math.h>
#include <vector>
#include <interpolation.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_integration.h>
using namespace std;

//�������� ��������� �������� ��������� Y = a + b*X
void GetParam(const vector <double> &X, const vector <double> &Y, double &a, double &b);
//������� �������� ����� � ������, ������� ����������
double mean(const vector<double> &temp);
//������� ������������ ����������
double AverSqFluct(const vector<double> &temp);

double exp_w(double x, double t1);

double sin_w(double x, double t1);

double lock_in(double x, double t1);

double double_boxcar(double x, double t1);

void AddPointsDLTS(double temp);
#endif // DLTS_MATH_H_INCLUDED
