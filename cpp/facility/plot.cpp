#include "graph.h"
#include "variable.h"
#include "gwin.h"
#include "dlts_math.h"
#include "facility.h"
#include "interpolation.h"

VOID plotDAQ(gwin::gVector *vData1, gwin::gVector *vData2)
{
    gwin::gData(hGraph_DAQ, vData1, vData2);
}

VOID PlotRelax()
{
    gwin::gVector vData1;
    stringstream buffer;
    for(size_t i = 0; i < SavedRelaxations[index_relax].size(); i++)
        vData1.push_back(0.001*gate_DAQ + i*(1/rate_DAQ)*1000); /* ms */
    if(index_mode == DLTS && !xAxisDLTS.empty())
        rewrite(buffer) << xAxisDLTS[index_relax] << " K";
    else if(index_mode == ITS && !itsBiasVoltages.empty())
        rewrite(buffer) << fixed << setprecision(2) << "T [K] = " << itsTemperature << endl
                        << setprecision(3) << "Bias [V] = " << itsBiasVoltages[index_relax] << endl << "Amp [V] =  " << itsAmpVoltages[index_relax];
    gwin::gAdditionalInfo(hRelax, buffer.str());
    gwin::gData(hRelax, &vData1, &SavedRelaxations[index_relax]);
}

VOID PlotDLTS()
{
    if(index_mode == ITS)
    {
        gwin::gDefaultPlot(hGraph_DLTS, "\0");
        return;
    }
    if(xAxisDLTS.size() < 2)
    {
        if(xAxisDLTS.size() == 1)
            gwin::gDefaultPlot(hGraph_DLTS, "You should measure the second point.");
        return;
    }
    if(index_plot_DLTS == 0 || index_plot_DLTS == 1)
    {
        /* Точность определения пика */
        static const double eps = 0.01;
        /* Поиск максимума методом золотого сечения */
        gwin::gVector vMinData1, vMinData2;
        double dRightBorder = ::dRightBorderGold;
        double dLeftBorder = ::dLeftBorderGold;
        /* Если границы заданы как нулевые */
        if(dRightBorder == 0.0 && dLeftBorder == 0.0 ||
           dLeftBorder < xAxisDLTS.at(0) || dRightBorder > xAxisDLTS.at(xAxisDLTS.size()-1))
        {
            /* Устанавливаем границы по умолчанию */
            dLeftBorder = xAxisDLTS.at(0);
            dRightBorder = xAxisDLTS.at(xAxisDLTS.size()-1);
        }
        for(size_t i = 0; i < CorTime.size(); i++)
        {
            interp f(xAxisDLTS, yAxisDLTS[i],xAxisDLTS.size(), gsl_interp_linear);
            double dMin = GoldSerch(dLeftBorder, dRightBorder, eps, f);
            vMinData1.push_back(dMin);
            vMinData2.push_back(f.at(dMin));
        }
        if(index_plot_DLTS == 0) /* DLTS кривые */
        {
            gwin::gMulVector vMulData;
            for(size_t i = 0; i < CorTime.size(); i++)
                vMulData.push_back(yAxisDLTS[i]);
            gwin::gAdditionalInfo(hGraph_DLTS, "\0");
            gwin::gCross(hGraph_DLTS, &vMinData1, &vMinData2); /* Индикаторы пика */
            gwin::gMulData(hGraph_DLTS, &xAxisDLTS, &vMulData);
        }
        else if(index_plot_DLTS == 1) /* График Аррениуса */
        {
            double B = sqrt(3.0)*pow(2.0, 2.5)*pow(::dFactorG,-1)*(::dEfMass*pow(BOLTZMANN,2)*pow(PLANCKS_CONSTANT_H,-3))*pow(PI, 1.5);
            gwin::gVector vData1, vData2, vData3;
            for(size_t i = 0; i < CorTime.size(); i++)
            {
                double T_max = vMinData1.at(i); /* Положение пика */
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
                vData1.push_back(pow(T_max*BOLTZMANN,-1));
                vData2.push_back( -log( tau*pow(T_max, 2) ) );
            }
            double a = 0.0, b = 0.0, Energy = 0.0, CrossSection = 0.0;
            GetParam(vData1, vData2, a, b);
            Energy = (-1)*b;
            CrossSection = exp(a)*pow(B, -1);
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
            /* Сохраняем результаты */
            ::xAxisAr = (vector<double>)vData1;
            ::yAxisAr = (vector<vector<double>>)vMulData;
        }
    }
}

