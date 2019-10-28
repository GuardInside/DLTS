#ifndef INTERPOLATION_H_INCLUDED
#define INTERPOLATION_H_INCLUDED
#include <gsl/gsl_interp.h>
#include <vector>
using namespace std;

/* ����������� ����� ����� ��� ���������� ������������ */
#define MIN_SIZE_INTERP     3

class interp
{
    private:
        gsl_interp_accel *acc;  //���������� �������
        gsl_interp *inter_poly; //��������� �� �����������
        size_t size;            //������ �����
        double *x;              //��������� �� ������� �����
        double *y;
    public:
        interp(const vector<double> &xAxis, const vector<double> &yAxis, size_t s, const gsl_interp_type *type);
        double at(double x_0);
        ~interp();
};


#endif // INTERPOLATION_H_INCLUDED