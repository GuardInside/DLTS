#include <interpolation.h>

interp::interp(const vector<double> &xAxis, const vector<double> &yAxis, size_t s, const gsl_interp_type *type): size(s)
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

interp::~interp()
{
    gsl_interp_free(inter_poly);
    gsl_interp_accel_free(acc);
    delete []x;
    delete []y;
}


    /* ������� */
    /*
    gwin::gVector vData1, vData2;
    gwin::gMulVector vMulData2;
    for(double t = *TimeAxis.begin(); t < *(TimeAxis.end()-1); t += 0.5*dt)
    {
        vData1.push_back(t);
        vData2.push_back(f.at(t));
    }
    stringstream buff;
    buff << fixed << setprecision(3);
    HWND hWnd = gwin::gCreateWindow(hInst, HWND_DESKTOP, WS_VISIBLE | WS_OVERLAPPED);
    gwin::gPosition(hWnd, 0, 0);
    gwin::gSize(hWnd, 500, 500);
    rewrite(buff) << "Before interp" << endl << "P: " << TimeAxis.size();
    gwin::gAdditionalInfo(hWnd, buff.str().data());
    gwin::gData(hWnd, &TimeAxis, vRelaxation);
    HWND hWnd1 = gwin::gCreateWindow(hInst, HWND_DESKTOP, WS_VISIBLE | WS_OVERLAPPED);
    gwin::gPosition(hWnd1, 500, 0);
    gwin::gSize(hWnd1, 500, 500);
    rewrite(buff) << "After interp" << endl << "P: " << vData1.size();
    gwin::gAdditionalInfo(hWnd1, buff.str().data());
    gwin::gData(hWnd1, &vData1, &vData2);
    MessageBox(0,"","",0); */
    /* ����� ������� */
