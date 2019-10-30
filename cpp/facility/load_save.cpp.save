#include "facility.h"
#include "variable.h"
#include "gwin.h"
#include "graph.h"

unsigned __stdcall DownloadFile(void*)
{
    static char load_file[BUFF_SIZE] = "";
    static OPENFILENAME file;
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
    LoadFile(load_file);
    return 0;
}

BOOL LoadFile(string strName)
{
    ifstream file;
    file.open(strName);
    if(!file.is_open())
        return FALSE;
    double itsUpper_boundary = 0.0, itsLower_boundary = 0.0;
    double buff = 0.0, temp = 0.0;
    string ext = GetExtensionFile(strName);
    file >> averaging_DAQ >> measure_time_DAQ >> rate_DAQ >> gate_DAQ
             >> Generator.step_voltage >> Generator.begin_voltage >> Generator.end_voltage;
    if(ext == ".dlts")
        index_mode = DLTS;
    else if(ext == ".its")
    {
        index_mode = ITS;
        file >> itsTemperature;
    }
    /** Оценка размера загружаемого файла **/
    UINT uSamples = rate_DAQ*measure_time_DAQ*0.001, uQuantity = 0;
    ifstream LoadFile2;
    LoadFile2.open(strName.data());
    LoadFile2.seekg (0, LoadFile2.end);
    uQuantity = (UINT)((LoadFile2.tellg()/8.5)/uSamples);
    LoadFile2.close();
    ClearMemmory();
    /** Загрузка **/
    int i = 0;
    while(!file.eof())
    {
        if(index_mode == DLTS)
            file >> temp;
        else if(index_mode == ITS)
        {
            file >> itsLower_boundary >> itsUpper_boundary;
            itsLowVoltages.push_back(itsLower_boundary);
            itsUpVoltages.push_back(itsUpper_boundary);
        }
        if(index_mode == DLTS && !xAxisDLTS.empty() && temp == xAxisDLTS.back())
            continue;
        vector <double>().swap(Relaxation);
        UINT uSamples = rate_DAQ*measure_time_DAQ*0.001;
        for(size_t i = 0; i < uSamples; i++)
        {
            file >> buff;
            Relaxation.push_back(buff);
        }
        if(index_mode == DLTS)
        {
            int offset = 0;
            for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
                if(temp > *it) offset++;
            AddPointsDLTS(temp); //Передаем значение температуры
            SavedRelaxations.insert(SavedRelaxations.begin()+offset, Relaxation);
            index_relax = offset;
        }
        if(index_mode == ITS)
        {
            SavedRelaxations.push_back(Relaxation);
            index_relax = SavedRelaxations.size() - 1; //без единицы, потому что индекс
        }
        SendMessage(hProgress, PBM_SETPOS, 100.0*i/uQuantity, 0);
        i++;
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    //Индекс используется для просмотра снятых ранее релаксаций
    file.close();
    PlotRelax();
    PlotDLTS();
    return TRUE;
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
