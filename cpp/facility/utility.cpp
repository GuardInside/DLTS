#include <facility.h>
#include "graph.h"

void ClearMemmory()
{
    ClearAxisDLTS();
    gwin::gMulVector().swap(SavedRelaxations);
    gwin::gVector().swap(Relaxation);
}

void ClearAxisDLTS()
{
    //EnterCriticalSection(&csGlobalVariable);
    if(yAxisDLTS != nullptr)
            delete []yAxisDLTS;
    yAxisDLTS = new vector<double>[CorTime.size()]; ///����� ���� ��������� � ������ ������������
    vector<double> ().swap(xAxisDLTS);
    vector<double> ().swap(itsLowVoltages);
    vector<double> ().swap(itsUpVoltages);
    //LeaveCriticalSection(&csGlobalVariable);
}

void RefreshDLTS()
{
    int progress = 0;
    if(xAxisDLTS.empty())
        return;
    gwin::gVector xAxis = xAxisDLTS;
    ClearAxisDLTS();
    UINT32 uSamples = rate_DAQ*measure_time_DAQ*0.001;
    for(size_t i = 0; i < xAxis.size(); i++)
    {
        vector <double>().swap(Relaxation);
        for(size_t j = 0; j < uSamples; j++)
            Relaxation.push_back(SavedRelaxations[i][j]);
        AddPointsDLTS(xAxis[i]);
        /* ���������� ���������� � ���� ������ */
        progress = 99 * i/(xAxis.size());
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    vector <double>().swap(Relaxation);
    for(size_t i = 0; i < uSamples; i++)
        Relaxation.push_back(SavedRelaxations[index_relax][i]);
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    PlotDLTS();
}

//����������� SetPoint �� �������� StepTemp � ������� ���� ������������
void prepare_next_set_point()
{
    stringstream buff;
    Thermostat.SetPoint += Thermostat.TempStep;
    buff << "SETP " << Thermostat.SetPoint << "K";
    Thermostat.Write(buff);
}

void SaveRelaxSignal(double MeanTemp, const vector<double> vData, double dVoltMin, double dVoltMax)
{
    EnterCriticalSection(&csGlobalVariable);
    string SavePath = Save + FileSaveName;
    if(index_mode == DLTS) SavePath += ".dlts";
    else if(index_mode == ITS) SavePath += ".its";
    ofstream file(SavePath.data(), std::ofstream::app);
    ifstream ifile(SavePath.data());
    if(!file.is_open())
    {
        MessageBox(NULL, "SaveFile wasn't find.", "Error", MB_OK);
        return;
    }
    //��������� ���������, ���� ���� ������ �������
    file << setiosflags(ios::fixed) << setprecision(0);
    if(ifile.peek() == EOF)
    {
        file<< averaging_DAQ << " "
            << measure_time_DAQ << " "
            << rate_DAQ << " "
            << gate_DAQ << endl << setprecision(3)
            << Generator.step_voltage << " "
            << Generator.begin_voltage << " "
            << Generator.end_voltage << endl;
        if(index_mode == ITS) file << setprecision(2) << MeanTemp << endl;
    }
    else file << endl;
    if(index_mode == DLTS) file << setprecision(2) << MeanTemp << endl;
    else if(index_mode == ITS) /* ������ ���������� ��������, � �� ������������� �������� */
        file << setprecision(3) << dVoltMin << " " << dVoltMax << endl;
    file << setprecision(6);
    UINT32 uSamples = rate_DAQ*measure_time_DAQ*0.001;
    for(uInt32 i = 0; i < uSamples; i++)
    {
        file << vData.at(i);
        if(i != uSamples-1)
            file << endl;
    }
    LeaveCriticalSection(&csGlobalVariable);
    ifile.close();
    file.close();
}

/* ���������� ���������� ��������� ����� */
string GetExtensionFile(char *str, int length)
{
    for(int i = 0; i < length; i++)
        if(*(str+i) == '.')
            return string(str+i, length-i);
    return string("Incorrect format");
}
