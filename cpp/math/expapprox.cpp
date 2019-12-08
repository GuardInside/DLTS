#include "dlts_math.h"

#include <algorithm>
#include <vector>
#include <string>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>

using namespace std;

struct data
{
    size_t n;
    const vector<double> *x;  /* Экспериментальные данные */
    const vector<double> *y;
    const vector<double> *sigma;
};

int expb_f(const gsl_vector *x_, void *data, gsl_vector *f)
{
    size_t n = ((struct data*)data)->n;
    const vector<double> *y = ((struct data*)data)->y;
    const vector<double> *x = ((struct data*)data)->x;
    const vector<double> *sigma = ((struct data*)data)->sigma;

    double A = gsl_vector_get(x_, 0);
    double lambda = gsl_vector_get(x_, 1);
    double b = gsl_vector_get(x_, 2);

    for(size_t i = 0; i < n; i++)
    {
        /** Модель Yi = A * exp(-lambda * t) + b */
        double t = x->at(i);
        double Yi = A * exp(-lambda * t) + b;
        gsl_vector_set(f, i, (Yi - y->at(i))/sigma->at(i));
    }
    return GSL_SUCCESS;
}

int expb_df(const gsl_vector *x_, void *data, gsl_matrix *J)
{
    size_t n = ((struct data*)data)->n;
    const vector<double> *x = ((struct data*)data)->x;
    const vector<double> *sigma = ((struct data*)data)->sigma;

    double A = gsl_vector_get(x_, 0);
    double lambda = gsl_vector_get(x_, 1);

    for(size_t i = 0; i < n; i++)
    {
        /** Матрица Якоби J(i,j) = dfi/dxj */
        /** Где fi = (Yi - yi)/sigma[i] */
        /** Yi = A * exp(-lambda * t) + b */
        /** xj - это параметры (A,lambda,b) */
        double t = x->at(i);
        double s = sigma->at(i);
        double e = exp(-lambda * t);
        gsl_matrix_set(J, i, 0, e/s); /** Матрица размера n на 3 */
        gsl_matrix_set(J, i, 1, -t * A * e/s);
        gsl_matrix_set(J, i, 2, 1/s);
    }
    return GSL_SUCCESS;
}

int expb_fdf(const gsl_vector *x, void *data,
             gsl_vector *f, gsl_matrix *J)
{
    expb_f(x, data, f);
    expb_df(x, data, J);
    return GSL_SUCCESS;
}

int get_exponent_fitt(const vector<double> *x, const vector<double> *y, const vector<double> *sigma,
                      double *A, double *tau, double *b, double *dA, double *dtau, double *db, double *ReducedChiSqr,
                      size_t *iter, size_t max_iter,
                      double epsabs, double epsrel,
                      string *strStatusMSG)
{
    if(x->size() != y->size())
        return -1; /* Несовпадающие размеры */
    const size_t p = 3;
    const size_t n = x->size();

    struct data d = {n, x, y, sigma};

    gsl_vector *x_init = gsl_vector_calloc(p);
    gsl_vector_set(x_init, 0, -0.01);
    gsl_vector_set(x_init, 1,  0.1);
    gsl_vector_set(x_init, 2,  0.5);

    gsl_multifit_function_fdf f;
    f.f = &expb_f;
    f.df = &expb_df;
    f.fdf = &expb_fdf;
    f.n = n;
    f.p = p;
    f.params = &d;


    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc(T, n, p);
    gsl_multifit_fdfsolver_set(s, &f, x_init);

    int status;
    size_t it = 0;
    do
    {
        it++;
        status = gsl_multifit_fdfsolver_iterate(s);

        if(status != GSL_SUCCESS) break;
        status = gsl_multifit_test_delta(s->dx, s->x, epsabs, epsrel);
    }while(status == GSL_CONTINUE && it < max_iter);

    if(iter != NULL) *iter = it;

    gsl_matrix *covar = gsl_matrix_alloc(p, p);
    gsl_multifit_covar(s->J, 0.0, covar); // epsrel?
    #define FIT(i) gsl_vector_get(s->x, i)
    #define ERR(i) sqrt(gsl_matrix_get(covar, i, i))
    double chi = gsl_blas_dnrm2(s->f);
    double dof = n - p;   /* degree of freedom */
    double c = GSL_MAX_DBL(1, chi / sqrt(dof));

    if(ReducedChiSqr != NULL)
    *ReducedChiSqr =  pow(chi, 2.0) / dof;

    if(A != NULL)
    *A  = FIT(0);
    if(dA != NULL)
    *dA = c*ERR(0);

    if(tau != NULL)
    *tau = pow(FIT(1), -1.0);
    if(dtau != NULL)
    *dtau = -1 * c*ERR(1) * pow(*tau, -2.0);

    if(b != NULL)
    *b = FIT(2);
    if(db != NULL)
    *db = c*ERR(2);

    gsl_multifit_fdfsolver_free(s);
    gsl_matrix_free(covar);
    gsl_vector_free(x_init);

    if(strStatusMSG != NULL)
    *strStatusMSG = gsl_strerror(status);
    return status;
}
