#include <facility.h>
#include "graph.h"

void ClearMemmory()
{
    ClearMemmoryDLTS();
    vector<double> ().swap(itsLowVoltages);
    vector<double> ().swap(itsUpVoltages);
    gwin::gMulVector().swap(SavedRelaxations);
    gwin::gVector().swap(Relaxation);
}

void ClearMemmoryDLTS()
{
    if(yAxisDLTS != nullptr)
            delete []yAxisDLTS;
    yAxisDLTS = new vector<double>[CorTime.size()]; ///Число осей совпадает с числом корреляторов
    vector<double> ().swap(xAxisDLTS);
}

void RefreshDLTS()
{
    int progress = 0;
    if(xAxisDLTS.empty())
        return;
    gwin::gVector xAxis = xAxisDLTS;
    ClearMemmoryDLTS();
    UINT32 uSamples = rate_DAQ*measure_time_DAQ*0.001;
    for(size_t i = 0; i < xAxis.size(); i++)
    {
        vector <double>().swap(Relaxation);
        for(size_t j = 0; j < uSamples; j++)
            Relaxation.push_back(SavedRelaxations[i][j]);
        AddPointsDLTS(xAxis[i]);
        /* Обновление информации о ходе записи */
        progress = 99 * i/(xAxis.size());
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    vector <double>().swap(Relaxation);
    for(size_t i = 0; i < uSamples; i++)
        Relaxation.push_back(SavedRelaxations[index_relax][i]);
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    PlotDLTS();
}

void SaveRelaxSignal(double MeanTemp, const vector<double> vData, double dVoltMin, double dVoltMax)
{
    //EnterCriticalSection(&csGlobalVariable);
    string SavePath = Save + FileSaveName;
    if(index_mode == DLTS) SavePath += ".dlts";
    else if(index_mode == ITS) SavePath += ".its";
    ofstream file;
    if(bfNewfile)
    {
        file.open(SavePath.data());
        bfNewfile = false;
        bfAppfile = true;
    }
    else if(bfAppfile)
        file.open(SavePath.data(), std::ofstream::app);
    if(!file.is_open())
    {
        MessageBox(NULL, "Save file isn't being found.", "Error", MB_OK);
        return;
    }
    //Сохраняем настройки, если файл открыт впервые
    file << setiosflags(ios::fixed) << setprecision(0);
    ifstream ifile(SavePath.data());
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
    else if(index_mode == ITS) /* Должен записывать истинное, а не установленное значение */
        file << setprecision(3) << dVoltMin << " " << dVoltMax << endl;
    file << setprecision(6);
    UINT32 uSamples = rate_DAQ*measure_time_DAQ*0.001;
    for(uInt32 i = 0; i < uSamples; i++)
    {
        file << vData.at(i);
        if(i != uSamples-1)
            file << endl;
    }
    //LeaveCriticalSection(&csGlobalVariable);
    ifile.close();
    file.close();
}

/* Возвращает расширение открытого файла */
string GetExtensionFile(string str)
{
    for(int i = 0; i < str.length(); i++)
        if(str[i] == '.')
            return string(str, i);
    return string("Incorrect format");
}
