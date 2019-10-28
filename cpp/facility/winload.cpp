#include <facility.h>
#include "variable.h"
#include "gwin.h"
#include "graph.h"

unsigned __stdcall DownloadFile(void*)
{
    static char load_file[BUFF_SIZE] = "";
    static OPENFILENAME file;
    double itsUpper_boundary = 0.0, itsLower_boundary = 0.0;
    ifstream LoadFile;
    double buff = 0.0, temp = 0.0;
        file.lStructSize = sizeof(OPENFILENAME);
        file.hwndOwner = NULL;
        file.lpstrFilter = "All files(*.*)\0*.*\0DLTS file(*.dlts)\0*.dlts\0ITS file(*.its)\0*.its\0\0";
        file.lpstrFile = load_file;
        file.nMaxFile = BUFF_SIZE;
        file.lpstrInitialDir = ".\\save\\";
        file.lpstrDefExt = NULL;
        file.Flags = OFN_HIDEREADONLY+OFN_NOCHANGEDIR;
    if(!GetOpenFileName(&file))
        return -1;
    loading = true; /* вспомогательный флаг */
    LoadFile.open(load_file);
    EnterCriticalSection(&csGlobalVariable);
    string ext = GetExtensionFile(load_file, strlen(load_file));
    LoadFile >> averaging_DAQ >> measure_time_DAQ >> rate_DAQ >> gate_DAQ
             >> Generator.step_voltage >> Generator.begin_voltage >> Generator.end_voltage;
    samples_DAQ = (measure_time_DAQ/1000.0)*rate_DAQ;
    if(ext == ".dlts")
        index_mode = DLTS;
    else if(ext == ".its")
    {
        index_mode = ITS;
        LoadFile >> itsTemperature;
    }
    int progress = 0;
    ClearAxisDLTS(); //Очищаем DLTS-кривые
    vector<vector<double>>().swap(SavedRelaxations); //Очищаем Сохраненные ранее релаксации
    while(!LoadFile.eof())
    {
        if(index_mode == DLTS)
            LoadFile >> temp;
        else if(index_mode == ITS)
        {
            LoadFile >> itsLower_boundary >> itsUpper_boundary;
            itsLowVoltages.push_back(itsLower_boundary);
            itsUpVoltages.push_back(itsUpper_boundary);
        }
        vector <double>().swap(Relaxation);
        for(size_t i = 0; i < samples_DAQ; i++)
        {
            LoadFile >> buff;
            Relaxation.push_back(buff);
        }
        progress++;
        if(progress == 99) progress = 0;
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
        if(index_mode == DLTS)
            AddPointsDLTS(temp); //Передаем значение температуры
        SavedRelaxations.push_back(Relaxation);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    //Индекс используется для просмотра снятых ранее релаксаций
    index_relax = SavedRelaxations.size() - 1; //без единицы, потому что индекс
    LeaveCriticalSection(&csGlobalVariable);
    LoadFile.close();
    PlotRelax();
    PlotDLTS();
    return 0;
}

unsigned __stdcall SaveFile(void*)
{
    char save_file[BUFF_SIZE];
    strcpy(save_file, FileSaveName.data());
    static OPENFILENAME file;
    ofstream SaveFile;
    string ext, filter;
    if(index_mode == DLTS)
    {
        filter = "DLTS file(*.dlts)\0*.dlts\0\0";
        ext = "dlts";
    }
    if(index_mode == ITS)
    {
        filter = "ITS file(*.its)\0*.its\0\0";
        ext = "its";
    }
        file.lStructSize = sizeof(OPENFILENAME);
        file.hwndOwner = NULL;
        file.lpstrFilter = filter.data();
        file.lpstrFile = save_file;
        file.nMaxFile = BUFF_SIZE;
        file.lpstrInitialDir = ".\\save\\";
        file.lpstrDefExt = ext.data();
        file.Flags = OFN_NOCHANGEDIR;
    if(!GetSaveFileName(&file))
        return -1;
    EnterCriticalSection(&csGlobalVariable);
    SaveFile.open(save_file);
    SaveFile << setiosflags(ios::fixed) << setprecision(0);
    SaveFile << averaging_DAQ << " "
             << measure_time_DAQ << " "
             << rate_DAQ << " "
             << gate_DAQ << endl << setprecision(3)
             << Generator.step_voltage << " "
             << Generator.begin_voltage << " "
             << Generator.end_voltage << endl;
    if(index_mode == ITS)
        SaveFile << setprecision(2) << itsTemperature << endl;
    int progress = 0;
    for(size_t i = 0; i < SavedRelaxations.size(); i++)
    {
        if(index_mode == DLTS)
            SaveFile << setprecision(2) << xAxisDLTS[i] << endl;
        else if(index_mode == ITS)
            SaveFile << setprecision(3) << itsLowVoltages[i] << " " << itsUpVoltages[i] << endl;
        SaveFile << setprecision(6);
        for(size_t j = 0; j < SavedRelaxations[i].size(); j++)
        {
            SaveFile << SavedRelaxations[i][j];
            if(j != SavedRelaxations[i].size() - 1)
                SaveFile << endl;
        }
        if(i != SavedRelaxations.size() - 1)
            SaveFile << endl;
        progress = 99*i/(SavedRelaxations.size());
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    LeaveCriticalSection(&csGlobalVariable);
    SaveFile.close();
    return 0;
}
