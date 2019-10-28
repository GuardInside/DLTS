#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED
#include <windows.h>
#include <sstream>
#include <iomanip>
#include <interpolation.h>
#include <facility.h>

using namespace std;

extern const int indent; //Отступ для вывода масштаба
extern const int GRAPHSIZE;
extern const int GRAPHWIDTH;

int scale(double val);
void set_min_max(const vector<double> &data, double &min_y, double &max_y);
void transform(HDC &hdc, const int sx, const int sy);
void transform_back(HDC &hdc, const int sx, const int sy);

void print_graph_arhenius(vector<double> &xAxis, int x_prec, vector<double> &yAxis, int y_prec, HDC &hdc, HPEN &hline,
                       double &min_y, double &max_y, double &min_x, double &max_x, int sx, int sy, const size_t scaleX, const size_t scaleY, const double a, const double b);

VOID plotDAQ();
VOID PlotRelax();
VOID PlotDLTS();
#endif // GRAPH_H_INCLUDED
