#include "graph.h"
#include "variable.h"
#include "gwin.h"
#include "dlts_math.h"
#include "facility.h"

VOID plotDAQ()
{
    gwin::gVector vData1;
    stringstream buffer;
    TryEnterCriticalSection(&csGlobalVariable);
    for(size_t i = 0; i < SignalDAQ.size(); i++)
        vData1.push_back(i*(1.0/rate_DAQ)*1000.0);
    buffer << "AI: " << ai_port + offset_ai_port;
    gwin::gAdditionalInfo(hRelax, buffer.str());
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
    //TryEnterCriticalSection(&csGlobalVariable);
    if(index_mode == ITS)
    {
        gwin::gDefaultPlot(hGraph_DLTS, "\0");
        return;
    }
    if(index_plot_DLTS == 0)
    {
        gwin::gMulVector vMulData;
        for(size_t i = 0; i < CorTime.size(); i++)
            vMulData.push_back(yAxisDLTS[i]);
        gwin::gAdditionalInfo(hGraph_DLTS, "\0");
        gwin::gMulData(hGraph_DLTS, &xAxisDLTS, &vMulData);
    }
    else if(index_plot_DLTS == 1 && !xAxisDLTS.empty())
    {
        static const double B = sqrt(3)*pow(2, 5/2)/G_CONSTANT*(MASS_ELECTRON*pow(BOLTZMANN,2)/pow(PLANCKS_CONSTANT_H,3))*pow(PI, 3/2);
        gwin::gVector vData1, vData2, vData3;
        for(size_t i = 0; i < CorTime.size(); i++)
        {
            /** Определяем положение пика **/
            size_t max_value_index = 0;
            for(size_t j = 1; j < yAxisDLTS[i].size(); j++)
            if(fabs(yAxisDLTS[i][j]) > fabs(yAxisDLTS[i][max_value_index]))
                max_value_index = j;
            double T_max = xAxisDLTS[max_value_index];
            /** Определяем, какой коррелятор выбран **/
            double t1 = CorTime[i]*0.001; // в секундах
            double t2 = t1*correlation_c;
            double tau = 0.0;
            switch(CorType)
            {
                case DoubleBoxCar:
                    tau = (t2-t1)/log(correlation_c);
                    break;
                case SinW:
                    tau = 0.4*(t2-t1); // Период синуса T_c = t2-t1
                    break;
                default:
                    {
                        gwin::gDefaultPlot(hGraph_DLTS, "The tau for the cor. isn't define.");
                        return;
                    }
            }
            vData1.push_back(pow(T_max*BOLTZMANN,-1.0));
            vData2.push_back( -log( tau*pow(T_max,2.0) ) );
        }
        double a = 0.0, b = 0.0, Energy = 0.0, CrossSection = 0.0;
        GetParam(vData1, vData2, a, b);
        Energy = (-1)*b;
        CrossSection = exp(b)/B;
        for(const auto &x: vData1)
            vData3.push_back(a+b*x);
        gwin::gMulVector vMulData;
        vMulData.push_back(vData2);
        vMulData.push_back(vData3);
        stringstream buff;
        buff << setprecision(2) << scientific << "CS: " << CrossSection << " cm^2" << endl
             << "R: " << sqrt(CrossSection/PI) << " cm" << endl
             << fixed << setprecision(3) << "E: " << Energy << " eV";
        gwin::gAdditionalInfo(hGraph_DLTS, buff.str());
        gwin::gMulData(hGraph_DLTS, &vData1, &vMulData);
    }
    //LeaveCriticalSection(&csGlobalVariable);
}

