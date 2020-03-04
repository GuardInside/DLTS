#include <dlts_math.h>
#include <algorithm>
#include "gwin.h"
#include "ini.h"

void AddPoint_with_interp(const vector<double> *vRelaxation, const double temp);
void AddPoint_with_approx(const vector<double> *vRelaxation, const double temp);

double exp_w(double x, double t1)
{
    double t2 = t1*correlation_c;           //������ ������� ��������
    double t_0 = (t2-t1)/2+t1;
    if(x >= t1 && x <= t_0)
        return exp(-(x-t1)/(0.4*(t2-t1)));
    if(x > t_0 && x <= t2)
        return -1*exp(-(x-t_0)/(0.4*(t2-t1)));
    return 0.0;
}

double sin_w(double x, double t1)
{
    double t2 = t1*correlation_c;           //������ ������� ��������
    if(x >= t1 && x <= t2)
        return sin((x-t1)*2*PI/(t2-t1));
    return 0.0;
}

double lock_in(double x, double t1)
{
    double t2 = t1*correlation_c;           //������ ������� ��������
    double t_0 = (t2-t1)/2+t1;
    if(x >= t1 && x <= t_0)
        return 1;
    if(x > t_0 && x <= t2)
        return -1;
    return 0.0;
}

double double_boxcar(double x, double t1)
{
    double t2 = t1*correlation_c;           //������ ������� ��������
    double dt = correlation_width/1000000.0;   //������ ��������� � ���
    if(x >= t1 && x <= t1+dt)
        return 1;
    if(x >= t2-dt && x <= t2)
        return -1;
    return 0.0;
}

double test_func(double x, double t1)
{
    return 1.0;
}

void AddPointsDLTS(const vector<double> *vRelaxation, const double temp, const double capacity)
{
    /* �������� �� ������������� ����� � ����� �������� ��� ������� ����� */
    int offset = 0;
    for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
    {
        if(temp > *it) offset++;
        else if(temp == *it)
            return;
    }
    xAxisDLTS.insert(xAxisDLTS.begin() + offset, temp); // TUS
    /* ���������� ������� ������� */
    double (*w) (double, double) = NULL;
    size_t N = 10; /* ����� ����� �������������� ����� �������� �� ��������� */
    switch(CorType)
    {
        case DoubleBoxCar:  w = double_boxcar;
            /* ��������� ����� ����� ��������; ��� ��������� � �� */
            //N =  N * ( (double)measure_time_DAQ / vRelaxation->size() ) / ( 0.001 * correlation_width );
            break;
        case LockIn:        w = lock_in;        break;
        case ExpW:          w = exp_w;          break;
        case SinW:          w = sin_w;          break;
    }
    /* ������ ����������� */
    size_t n = N*vRelaxation->size();                   //����� ����� ������������
    double T_g = (0.000001*gate_DAQ);                   //������������ ������ �������
    double dt = pow(rate_DAQ, -1);                      //��� �������������
    double a = T_g, b = T_g + (n-1)*dt;                 //������ � ������� �������
    double h = (b-a)/(n-1);                             //��� �����
    double I = 0.0;                                     //�������� ���������
    /* ��������������� ��� ������� */
    vector<double> TimeAxis;
    for(size_t i = 0; i < vRelaxation->size(); i++)
        TimeAxis.push_back(i*dt + T_g);
    try{
        if(AprEnableDLTS == false)
        {
            /* ������������� ������ */
            interp f(TimeAxis, *vRelaxation, vRelaxation->size(), gsl_interp_linear);
            for(size_t c = 0; c < CorTime.size(); c++)          //����������� �� ���� ������������
            {
                double t1 = 0.001*CorTime[c];
                for(double x_i = a; x_i <= a+h*(n-1); x_i += h) //����� ��������
                    if(w(x_i,t1) != 0.0)
                        I += h*(f.at(x_i)*w(x_i,t1) + f.at(x_i+h)*w(x_i+h, t1))/2;
                /* ��������� �� ����� �� ������ �� ���� */
                VoltageToCapacity(&I);
                I /= int_pre_amp_gain[PRE_AMP_GAIN_SULA_index] * capacity;
                yAxisDLTS[c].insert(yAxisDLTS[c].begin() + offset, I);
            }
        }
        else if(AprEnableDLTS == true)
        {
            /* �������������� ������ */
            double A = 0.0, B = 0.0, tau = 0.0;
            vector<double> vSigma(TimeAxis.size(), 1); /* ���� �� ��������� */
            get_exponent_fitt(&TimeAxis, vRelaxation, &vSigma,
                          &A, &tau, &B, NULL, NULL, NULL, NULL, NULL, ::AprIter, ::AprErr, 0, NULL);
            for(size_t c = 0; c < CorTime.size(); c++)          //����������� �� ���� ������������
            {
                double t1 = 0.001*CorTime[c];
                for(double x_i = a; x_i <= a+h*(n-1); x_i += h) //����� ��������
                    if(w(x_i,t1) != 0.0)
                    {
                        double f_i = A * exp(- x_i / tau) + B;
                        double f_i_h = A * exp(-(x_i + h) / tau) + B;
                        I += h*(f_i*w(x_i,t1) + f_i_h*w(x_i+h, t1))/2;
                    }
                /* ��������� �� ����� �� ������ �� ���� */
                VoltageToCapacity(&I);
                I /= int_pre_amp_gain[PRE_AMP_GAIN_SULA_index] * capacity;
                yAxisDLTS[c].insert(yAxisDLTS[c].begin() + offset, I);
            }
        }
    }catch(std::exception &e){ MessageBox(0, e.what(), "AddPointDLTS", 0); }
}
