#include <sstream>
#include <string>
#include <fstream>
#include <exception>

#include "facility.h"
#include "variable.h"
#include "gwin.h"
#include "ini.h"

using namespace std;

UINT SaveSettings();
UINT SaveRelax();
UINT SaveDLTS();
UINT SaveITS();
UINT SaveArrhenius();
UINT SaveCT();

char LoadFileName[BUFF_SIZE];
char SaveFileName[BUFF_SIZE];

VOID DownloadWindow()
{
    OPENFILENAME file = {0};
        file.lStructSize = sizeof(OPENFILENAME);
        file.hwndOwner = NULL;
        file.lpstrFilter = "All files(*.*)\0*.*\0DLTS file(*.dlts)\0*.dlts\0ITS file(*.its)\0*.its\0\0";
        file.lpstrFile = LoadFileName;
        file.nMaxFile = BUFF_SIZE;
        file.lpstrInitialDir = ".\\save\\";
        file.lpstrDefExt = NULL;
        file.Flags = OFN_HIDEREADONLY+OFN_NOCHANGEDIR;
    if(!GetOpenFileName(&file)) return; /* Отмена выбора файла */
    /* Установка новго имени файла для сохранений */
    FileSavePath.clear();
    FileSaveName.clear();
    for(WORD i = 0; i < file.nFileOffset; i++)
        FileSavePath.push_back(LoadFileName[i]);
    for(WORD i = file.nFileOffset; i < (file.nFileExtension-1); i++)
        FileSaveName.push_back(LoadFileName[i]);
    string ext;
    for(WORD i = file.nFileExtension - 1; i < strlen(LoadFileName); i++)
        ext.push_back(LoadFileName[i]);
    /* Установка мода */
    if(ext != FileSaveExt)
    {
        MessageBox(NULL, "You should chose file with .dlts", "Warning", MB_ICONWARNING);
        return;
    }
    CloseHandle((HANDLE)_beginthreadex(NULL, 0, LoadFile, NULL, 0, NULL));
}

UINT CALLBACK LoadFile(PVOID)
{
    string strName = FileSavePath + FileSaveName + FileSaveExt;

    ifstream file;
    file.open(strName);
    if(!file.is_open()) return FALSE;
    /** Создаем событие-флаг **/
    /* Чтобы запись релаксаций не начиналась раньше конца загрузки */
    ResetEvent(hDownloadEvent);
    double temp = 0.0, capacity = 1.0;
    /** Загружаем настройки **/
    string placeholder;
    file >> placeholder >> measure_time_DAQ >> placeholder
         >> placeholder >> gate_DAQ >> placeholder
         >> placeholder >> rate_DAQ >> placeholder
         >> placeholder >> RANGE_SULA_index >> placeholder
         >> placeholder >> PRE_AMP_GAIN_SULA_index >> placeholder;
    if(ApplySettings() != 0)
        return FALSE;
    file >> placeholder >> Generator.bias >> placeholder
         >> placeholder >> Generator.amp >> placeholder
         >> placeholder >> Generator.width >> placeholder;
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
        file >> temp >> capacity;
        /* Релаксация с такой температурой уже есть в памяти */
        if(!xAxisDLTS.empty() && temp == xAxisDLTS.back())
            continue;
        vector <double> vRelaxation;
        UINT uSamples = rate_DAQ*measure_time_DAQ*0.001;
        vRelaxation.reserve(uSamples);
        double buff = 0.0;
        for(size_t i = 0; i < uSamples; i++)
        {
            file >> buff;
            vRelaxation.push_back(buff);
        }
        int offset = 0;
        for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
            if(temp > *it) offset++;
        AddPointToDLTS(&vRelaxation, temp, capacity); //Передаем значение температуры
        EnterCriticalSection(&csSavedData);
            SavedRelaxations.insert(SavedRelaxations.begin()+offset, vRelaxation);
            SavedCapacity.insert(SavedCapacity.begin()+offset, capacity);
            index_relax.store(offset);
        LeaveCriticalSection(&csSavedData);
        SendMessage(hProgress, PBM_SETPOS, 100.0*i/uQuantity, 0);
        i++;
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    file.close();
    SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_RELAX, 0);
    SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
    /** Устанавливаем флаг окончания загрузки **/
    SetEvent(hDownloadEvent);
    return TRUE;
}

void SaveRelaxSignalToFile(double MeanTemp, const vector<double> *vData, double dBias, double dAmp, double capacity)
{
    string FullPath = FileSavePath + FileSaveName + FileSaveExt;
    ofstream file;

    ifstream ifile(FullPath.data());
    if(bfNewfile.load() == true || ifile.peek() == EOF)
    {
        file.open(FullPath.data());
        bfNewfile.store(false);
        FileOpeningTest(file);
        /* Сохраняем настройки, если файл открыт впервые */
        file << fixed << setprecision(0)
             << "Time: " << measure_time_DAQ << " [ms]\n"
             << "Gate: " << gate_DAQ << " [mcs]\n"
             << "Rate: " << rate_DAQ << " [Hz]\n"
             << "Capa: " << RANGE_SULA_index << " [0|10pF,1|30pF,2|100pF,3|300pF,4|1nF]\n"
             << "Pre-amp: " << RANGE_SULA_index << " [0|1,1|3,2|10,3|30,4|100]\n"
             << setprecision(VOLTAGE_PRECISION)
             << "Bias: " << Generator.bias << " [V]\n"
             << "Amp: " << Generator.amp << " [V]\n"
             << "Width: " << Generator.width << " [ms]" << endl;
    }
    else
    {
        file.open(FullPath.data(), std::ofstream::app);
        FileOpeningTest(file);
        file << fixed << setprecision(0) << endl;
    }

    file << setprecision(THERMO_PRECISION)
         << MeanTemp << " "
         << setprecision(VOLTAGE_PRECISION)
         << capacity << endl;
    UINT32 uSamples = rate_DAQ*measure_time_DAQ*0.001;
    for(uInt32 i = 0; i < uSamples; i++)
    {
        file << (*vData)[i];
        if(i != uSamples-1) file << endl;
    }

    ifile.close();
    file.close();
}

VOID SaveWindow(SAVE_MODE _mode)
{
    static SAVE_MODE mode;
    mode = _mode;
    string strTitle;
    switch(mode)
    {
        case SAVE_SETTINGS:
            strTitle = "Save settings";
            break;
        case SAVE_RELAXATIONS:
            strTitle = "Save relaxations";
            break;
        case SAVE_DLTS:
            strTitle = "Save DLTS";
            break;
        case SAVE_ARRHENIUS:
            strTitle = "Save Arrhenius plot";
            break;
        case SAVE_CT:
            strTitle = "Save CT plot";
            break;
    }
    strcpy(SaveFileName, FileSaveName.data()); /* Имя файла по умолчанию */
    const string ext = "txt", filter = ".txt\0";
    OPENFILENAME file = {0};
        file.lStructSize = sizeof(OPENFILENAME);
        file.hwndOwner = NULL;
        file.lpstrFilter = filter.data();
        file.lpstrFile = SaveFileName;
        file.nMaxFile = BUFF_SIZE;
        file.lpstrInitialDir = ".\\save\\";
        file.lpstrDefExt = ext.data();
        file.Flags = OFN_NOCHANGEDIR;
        file.lpstrTitle = strTitle.c_str();
    if(!GetSaveFileName(&file))
        return;
    /* Установка новго имени файла для сохранений */
    HANDLE thSave = (HANDLE)_beginthreadex(NULL, 0, SaveFile, (PVOID)&mode, 0, NULL);
    CloseHandle(thSave);
}

UINT CALLBACK SaveFile(PVOID _mode)
{
    SAVE_MODE mode =  *(SAVE_MODE*)_mode;
    if(SavedRelaxations.empty() && mode != SAVE_SETTINGS)
        return -1;
    //MessageBox(0,to_string(*(SAVE_MODE*)_mode).c_str(),"",0);
    switch(mode)
    {
        case SAVE_SETTINGS:
            return SaveSettings();
        case SAVE_RELAXATIONS:
            return SaveRelax();
        case SAVE_DLTS:
            if(index_mode.load() == DLTS)
                return SaveDLTS();
            else
                return SaveITS();
        case SAVE_ARRHENIUS:
            return SaveArrhenius();
        case SAVE_CT:
            return SaveCT();
        default:
            MessageBox(NULL, "SaveFile thread", "Error", MB_ICONERROR);
            return -1;
    }
}
UINT SaveSettings()
{
    ini::File iniFile{SaveFileName};
    iniFile.Redir(std::string(""));

    iniFile.WriteString("DAQ", "Range [V]", range[index_range.load()] );
    iniFile.WriteInt("DAQ", "Averaging [amount]", averaging_DAQ);
    iniFile.WriteInt("DAQ", "Rate [Hz]", rate_DAQ);
    iniFile.WriteInt("DAQ", "Time [ms]", measure_time_DAQ);
    iniFile.WriteDoubleFix("DAQ", "gate [µs]", gate_DAQ, TIME_PRECISION);
    /* Сохраняем настройки блока SULA */
    iniFile.WriteInt("SULA", "Capacity range [pF]", int_range_sula[RANGE_SULA_index]);
    iniFile.WriteInt("SULA", "Pre-amplifier", int_pre_amplifier[PRE_AMP_GAIN_SULA_index]);
    /* Сохраняем настройки блока генератора импульсов */
    if(Generator.is_active.load())
    {
        iniFile.WriteDoubleFix("Generator", "Period", Generator.period, TIME_PRECISION);
        iniFile.WriteDoubleFix("Generator", "Width", Generator.width, TIME_PRECISION);
    }
    /* Сохраняем настройки блока ITS */
    /*switch(index_mode.load())
    {
        case DLTS:*/
            iniFile.WriteDoubleFix("Generator", "Amplitude", Generator.amp, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "Bias", Generator.bias, VOLTAGE_PRECISION);
            /*break;
        case AITS:
            iniFile.WriteDoubleFix("Generator", "Step voltage", Generator.step_amp, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "Begin amplitude", Generator.begin_amp, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "End amplitude", Generator.end_amp, VOLTAGE_PRECISION);
            break;
        case BITS:
            iniFile.WriteDoubleFix("Generator", "Step voltage", Generator.step_bias, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "Begin bias", Generator.begin_bias, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "End bias", Generator.end_bias, VOLTAGE_PRECISION);
            break;
    }*/
    return 0;
}

UINT SaveArrhenius()
{
    if(xAxisAr.size() == 0)
    {
        MessageBox(NULL, "You should load the Arrhenius plot.", "Information", MB_OK | MB_ICONEXCLAMATION);
        return -2;
    }
    ofstream file;
    file.open(SaveFileName, ios_base::trunc);
    int progress = 0;
    size_t uSample = xAxisAr.size();
    /* Шапка таблицы */
    file << "X\tY\tY_MSQ" << endl << fixed;
    /* Таблица */
    for(size_t i = 0; i < uSample; i++)
    {
        file << setprecision(2) << xAxisAr.at(i)
             << setprecision(2)
             << '\t' << yAxisAr[i]
             << '\t' << yAxisArMSQ[i]
             << endl;
        progress = 99*i/(uSample);
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    file.close();
    return 0;
}

UINT SaveRelax()
{
    ofstream file;
    file.open(SaveFileName, ios_base::trunc);
    int progress = 0;
    double dt = 1000.0/rate_DAQ; /* ms */
    /* Шапка таблицы */
    file << fixed << setprecision(THERMO_PRECISION);
    file << "[ms]";
    EnterCriticalSection(&csSavedData);
    const vector<vector<double>> &y = SavedRelaxations;
        size_t uSample = y[0].size();
        file << "\t" << xAxisDLTS[0];
        for(size_t j = 1; j < y.size(); j++)
            file << "\t" << xAxisDLTS[j];
        file << endl;
        for(size_t i = 0; i < uSample; i++)
        {
            file << setprecision(TIME_PRECISION) << (0.001*gate_DAQ + i*dt)
                 << setprecision(VOLTAGE_PRECISION);
            for(size_t j = 0; j < y.size(); j++)
            {
                file << '\t' << y[j][i];
            }
            file << endl;
            progress = 99*i/(uSample);
            SendMessage(hProgress, PBM_SETPOS, progress, 0);
        }
    LeaveCriticalSection(&csSavedData);
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    file.close();
    return 0;
}

UINT SaveDLTS()
{
    ofstream file;
    file.open(SaveFileName, ios_base::trunc);
    int progress = 0;
    size_t uSample = xAxisDLTS.size();
    file << fixed;
    const vector<vector<double>> &y = yAxisDLTS;
    file << "T [K]";
    for(const auto Tc: CorTc)
        file << '\t' << Tc;
    for(size_t i = 0; i < uSample; i++)
    {
        file << endl << setprecision(THERMO_PRECISION) << xAxisDLTS.at(i) << setprecision(10);
        for(size_t j = 0; j < CorTc.size(); j++)
        {
            file << '\t' << y[j][i];
        }
        progress = 99*i/(uSample);
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    file.close();
    return 0;
}

UINT SaveITS()
{
    if(xAxisITS.empty() || yAxisITS.empty() || xAxisDLTS.empty())
        return -1;
    ofstream file;
    file.open(SaveFileName, ios_base::trunc);
    size_t  nCurves     = xAxisDLTS.size();
    size_t  n           = yAxisITS[0].size();
    file << fixed << setprecision(THERMO_PRECISION) << "log[ms^-1]\t";
    for(size_t i = 0; i < nCurves; i++)
    {
        file << xAxisDLTS[i] << "\t";
    }
    file << setprecision(10);
    for(size_t i = 0; i < n; i++)
    {
        file <<  endl << xAxisITS[i] << "\t";
        for(size_t j = 0; j < nCurves; j++)
        {
            file << yAxisITS[j][i] << "\t";
        }
        SendMessage(hProgress, PBM_SETPOS, 100*i/n, 0);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    file.close();
    return 0;
}

UINT SaveCT()
{
    ofstream file;
    file.open(SaveFileName, ios_base::trunc);
    size_t uSample = SavedCapacity.size();
    file << fixed << "T [K]\tCapacity [pF]";
    for(size_t i = 0; i < uSample; i++)
    {
        file << setprecision(THERMO_PRECISION) << xAxisDLTS[i] << setprecision(3)
            << '\t' << SavedCapacity[i] << endl;
        SendMessage(hProgress, PBM_SETPOS, 100*(i+1)/(uSample), 0);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    file.close();
    return 0;
}
