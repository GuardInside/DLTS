#include "dlts_math.h"

interp::interp(const std::vector<double> &xAxis, const std::vector<double> &yAxis, const gsl_interp_type *type): size(xAxis.size())
{
    x = new double[size];
    y = new double[size];
    for(size_t i = 0; i < size; i++)
    {
        x[i] = xAxis[i];
        y[i] = yAxis[i];
    }
    acc = gsl_interp_accel_alloc();
    inter_poly = gsl_interp_alloc(type, size);
    gsl_interp_init(inter_poly, x, y, size);
}

double interp::at(double x_0)
{
    return gsl_interp_eval(inter_poly, x, y, x_0, acc);
}

double interp::operator()(double x_0)
{
    return at(x_0);
}

interp::~interp()
{
    gsl_interp_free(inter_poly);
    gsl_interp_accel_free(acc);
    delete []x;
    delete []y;
}