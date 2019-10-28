#include <facility.h>

void ClearAxisDLTS()
{
    EnterCriticalSection(&csGlobalVariable);
    if(yAxisDLTS != nullptr)
            delete []yAxisDLTS;
    yAxisDLTS = new vector<double>[CorTime.size()]; ///����� ���� ��������� � ������ ������������
    vector<double> ().swap(xAxisDLTS);
    vector<double> ().swap(itsLowVoltages);
    vector<double> ().swap(itsUpVoltages);
    LeaveCriticalSection(&csGlobalVariable);
}

void RefreshDLTS()
{
    int progress = 0;
    if(xAxisDLTS.empty())
        return;
    gwin::gVector xAxis = xAxisDLTS;
    ClearAxisDLTS();
    for(size_t i = 0; i < xAxis.size(); i++)
    {
        vector <double>().swap(Relaxation);
        for(size_t j = 0; j < samples_DAQ; j++)
            Relaxation.push_back(SavedRelaxations[i][j]);
        AddPointsDLTS(xAxis[i]);
        /* ���������� ���������� � ���� ������ */
        progress = 99 * i/(xAxis.size());
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    vector <double>().swap(Relaxation);
    for(size_t i = 0; i < samples_DAQ; i++)
        Relaxation.push_back(SavedRelaxations[index_relax][i]);
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    gwin::gMulVector vMulData2;
    for(int i = 0; i < CorTime.size(); i++)
        vMulData2.push_back(yAxisDLTS[i]);
    gwin::gMulData(hGraph_DLTS, &xAxis, &vMulData2);
}

//����������� SetPoint �� �������� StepTemp � ������� ���� ������������
void prepare_next_set_point()
{
    stringstream buff;
    Thermostat.SetPoint += Thermostat.TempStep;
    buff << "SETP " << Thermostat.SetPoint << "K";
    Thermostat.Write(buff);
}

void SaveRelaxSignal(double MeanTemp, const vector<double> vData)
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
        file << setprecision(3) << Generator.voltage_low_measured << " " << Generator.voltage_up_measured << endl;
    file << setprecision(6);
    for(uInt32 i = 0; i < samples_DAQ; i++)
    {
        file << vData.at(i);
        if(i != samples_DAQ-1)
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
