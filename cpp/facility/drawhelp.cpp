#include "graph.h"
#include "variable.h"
#include "gwin.h"


const int indent =          25; //Отступ для вывода масштаба
const int GRAPHSIZE =       1200;
const int GRAPHWIDTH =      1000;

int scale(double val)
{
    if(val == 0.0)
        return 1;
    int sc = 1;
    while(val < 10.0)
    {
        val *= 10.0;
        sc *= 10;
    }
    return sc;
}

void set_min_max(const vector<double> &data, double &min_y, double &max_y)
{
    max_y = min_y = data[0];
    for(const auto &itir: data)
    {
        if(max_y < itir) max_y = itir;
        if(min_y > itir) min_y = itir;
    }
}

void transform(HDC &hdc, const int sx, const int sy)
{
    SetMapMode(hdc, MM_ANISOTROPIC);
    SetWindowExtEx(hdc, GRAPHSIZE, -GRAPHSIZE, NULL);
    SetViewportExtEx(hdc, sx, sy, NULL);
    SetViewportOrgEx(hdc, 2*indent+int(indent*0.1), sy-indent/1.5, NULL);
}

void transform_back(HDC &hdc, const int sx, const int sy)
{
    SetMapMode(hdc, MM_ANISOTROPIC);
    SetWindowExtEx(hdc, GRAPHSIZE, -GRAPHSIZE, NULL);
    SetViewportExtEx(hdc, sx, sy, NULL);
    SetViewportOrgEx(hdc, 0, sy, NULL);
}

VOID plotDAQ()
{
    gwin::gVector vData1;
    EnterCriticalSection(&csGlobalVariable);
    for(size_t i = 0; i < SignalDAQ.size(); i++)
        vData1.push_back(i*(1.0/rate_DAQ)*1000.0);
    gwin::gData(hGraph_DAQ, &vData1, &SignalDAQ);
    LeaveCriticalSection(&csGlobalVariable);
}

VOID PlotRelax()
{
    TryEnterCriticalSection(&csGlobalVariable);
    gwin::gVector vData1;
    stringstream buffer;
    for(size_t i = 0; i < Relaxation.size(); i++)
        vData1.push_back(i*(1/rate_DAQ)*1000);
    if(index_mode == DLTS && !xAxisDLTS.empty())
        rewrite(buffer) << xAxisDLTS[index_relax] << " K";
    else if(index_mode == ITS && !itsLowVoltages.empty())
        rewrite(buffer) << fixed << setprecision(2) << itsTemperature << " K\n"
                        << setprecision(3) << "[" << itsLowVoltages[index_relax] << "," << itsUpVoltages[index_relax] << "] V";
    gwin::gAdditionalInfo(hRelax, buffer.str());
    gwin::gData(hRelax, &vData1, &SavedRelaxations[index_relax]);
    LeaveCriticalSection(&csGlobalVariable);
}

VOID PlotDLTS()
{
    EnterCriticalSection(&csGlobalVariable);
    if(index_mode == ITS) return;
    gwin::gMulVector vMulData2;
    for(int i = 0; i < CorTime.size(); i++)
        vMulData2.push_back(yAxisDLTS[i]);
    gwin::gMulData(hGraph_DLTS, &xAxisDLTS, &vMulData2);
    LeaveCriticalSection(&csGlobalVariable);
}

