#include <dlts_math.h>

double exp_w(double x, double t1)
{
    double t2 = t1*correlation_c;           //������ ������� ��������
    if(x >= t1 && x <= t2)
        return exp(-(x-t1)/(0.4*(t2-t1)));
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

void AddPointsDLTS(double temp)
{
    EnterCriticalSection(&csGlobalVariable);
    int offset = 0;
    for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
    {
        if(temp > *it) offset++;
        else if(temp == *it)
            return;
    }
    xAxisDLTS.insert(xAxisDLTS.begin() + offset, temp);
    ///���������� ������� �������
    double (*w) (double, double) = double_boxcar;
    size_t n = 10000; //���������� dt ����������
    switch(CorType)
    {
        case DoubleBoxCar:
            /* ������ ������� ����� �������� �� 50 ���������� �������������� */
            n = (measure_time_DAQ*1000)/(correlation_width/50);
        break;
        case LockIn: w = lock_in;
        break;
        case ExpW: w = exp_w;
        break;
        case SinW: w = sin_w;
        break;
    }
    double T_g = (gate_DAQ/1000000);                //��� ��������� �������������
    double dt = 1.0/rate_DAQ;
    vector<double> TimeAxis;
    for(size_t i = 0; i < Relaxation.size(); i++)
        TimeAxis.push_back(i*dt + T_g);
    interp f(TimeAxis, Relaxation, Relaxation.size(), gsl_interp_cspline);
    /* ������ ����������� */
    double I = 0.0;                                     //�������� ���������
    double a = T_g, b = T_g + Relaxation.size()*dt;     //������ � ������� �������, ������������ ������ � ������� �������
    /* ��������� ������� ��������� */
    double h = (b-a)/n;                                 //��� �����,�� �� ������� ���������
    for(size_t c = 0; c < CorTime.size(); c++)  //����������� �� ���� ������������
    {
        double t1 = CorTime[c]/1000;
        for(double x_i = a+h; x_i <= a+h*(n-1); x_i += h)          //����� ��������
        {
            if(w(x_i,t1) != 0.0)
                I += f.at(x_i) * w(x_i,t1);
        }
        I = h*( (f.at(a)*w(a,t1) + f.at(b)*w(b, t1))/2 + I );
        /* ��������� �� ����� �� ������ �� ���� */
        yAxisDLTS[c].insert(yAxisDLTS[c].begin() + offset, I);
    }
    LeaveCriticalSection(&csGlobalVariable);
}
