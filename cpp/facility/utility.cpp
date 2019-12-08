#include <facility.h>

#include "graph.h"
#include "ini.h"
#include "variable.h"

void VoltageToCapacity(double *value)
{
    static const double _SULA_CONST = 0.2;
    *value = _SULA_CONST*(*value)*int_range_sula[RANGE_SULA_index] / int_pre_amp_gain[PRE_AMP_GAIN_SULA_index];
}

void ClearMemmory()
{
    /* �������� ��������� �������� */
    auto_peak_search = true;
    ClearMemmoryDLTS();
    itsBiasVoltages.clear();
    itsAmpVoltages.clear();
    gwin::gMulVector().swap(SavedRelaxations);
    gwin::gDefaultPlot(hRelax, "\0");
    gwin::gDefaultPlot(hGraph_DLTS, "\0");
}

void ClearMemmoryDLTS()
{
    if(yAxisDLTS != nullptr)
            delete []yAxisDLTS;
    yAxisDLTS = new vector<double>[CorTime.size()]; ///����� ���� ��������� � ������ ������������
    xAxisDLTS.clear();
}

void RefreshDLTS()
{
    int progress = 0;
    if(xAxisDLTS.empty())
        return;
    gwin::gVector xAxis = xAxisDLTS;
    ClearMemmoryDLTS();
    for(size_t i = 0; i < xAxis.size(); i++)
    {
        AddPointsDLTS(&SavedRelaxations[i], xAxis[i]);
        /* ���������� ���������� � ���� ������ */
        progress = 99 * i/(xAxis.size());
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    auto_peak_search = true;
    PlotDLTS();
}

void SaveRelaxSignal(double MeanTemp, const vector<double> *vData, double dBias, double dAmp)
{
    string ext;
    if(index_mode == DLTS) ext = ".dlts";
    else if(index_mode == ITS) ext = ".its";
    string FullPath = FileSavePath + FileSaveName + ext;
    ofstream file;
    if(bfNewfile)
    {
        file.open(FullPath.data());
        bfNewfile = false;
    }
    else
        file.open(FullPath.data(), std::ofstream::app);
    if(!file.is_open())
    {
        MessageBox(NULL, "Save file isn't being found.", "Error", MB_ICONERROR);
        return;
    }

    file << setiosflags(ios::fixed) << setprecision(0);
    ifstream ifile(FullPath.data());
    if(ifile.peek() == EOF)
    {
        /* ��������� ���������, ���� ���� ������ ������� */
        file<< averaging_DAQ << " "
            << measure_time_DAQ << " "
            << rate_DAQ << " "
            << gate_DAQ << endl << setprecision(VOLTAGE_PRECISION)
            << Generator.amplitude << " "
            << Generator.bias << endl;
        if(index_mode == ITS) file << setprecision(THERMO_PRECISION) << MeanTemp << endl;
    }
    else file << endl;
    if(index_mode == DLTS) file << setprecision(THERMO_PRECISION) << MeanTemp << endl;
    else if(index_mode == ITS) /* ������ ���������� ��������, � �� ������������� �������� */
        file << setprecision(VOLTAGE_PRECISION) << dBias << " " << dAmp << endl;
    file << setprecision(VOLTAGE_PRECISION);
    UINT32 uSamples = rate_DAQ*measure_time_DAQ*0.001;
    for(uInt32 i = 0; i < uSamples; i++)
    {
        file << vData->at(i);
        if(i != uSamples-1)
            file << endl;
    }
    ifile.close();
    file.close();
}

/* ���������� ���������� ��������� ����� */
string GetExtensionFile(string str)
{
    for(size_t i = 0; i < str.length(); i++)
        if(str[i] == '.')
            return string(str, i);
    return string("Incorrect format");
}
