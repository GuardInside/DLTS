#include <sstream>
#include <string>
#include <exception>

#include "facility.h"
#include "variable.h"
#include "gwin.h"
#include "graph.h"
#include "ini.h"

using namespace std;

UINT SaveSettings();
UINT SaveRelax();
UINT SaveDLTS();
UINT SaveArrhenius();

static char LoadFileName[BUFF_SIZE];
static char SaveFileName[BUFF_SIZE];

VOID DownloadWindow()
{
    OPENFILENAME file = {0};
        file.lStructSize = sizeof(OPENFILENAME);
        file.hwndOwner = NULL;
        file.lpstrFilter = "All files(*.*)\0*.*\0DLTS file(*.dlts)\0*.dlts\0ITS file(*.aits)\0*.its\0\0";
        file.lpstrFile = LoadFileName;
        file.nMaxFile = BUFF_SIZE;
        file.lpstrInitialDir = ".\\save\\";
        file.lpstrDefExt = NULL;
        file.Flags = OFN_HIDEREADONLY+OFN_NOCHANGEDIR;
    if(!GetOpenFileName(&file)) return; /* ������ ������ ����� */
    /* ��������� ����� ����� ����� ��� ���������� */
    FileSavePath.clear();
    FileSaveName.clear();
    for(WORD i = 0; i < file.nFileOffset; i++)
        FileSavePath.push_back(LoadFileName[i]);
    for(WORD i = file.nFileOffset; i < (file.nFileExtension-1); i++)
        FileSaveName.push_back(LoadFileName[i]);
    string ext;
    for(WORD i = file.nFileExtension; i < strlen(LoadFileName); i++)
        ext.push_back(LoadFileName[i]);
    /* ��������� ���� */
    if(ext == "dlts")
        index_mode.store(DLTS);
    else if(ext == "aits")
        index_mode.store(AITS);
    else
    {
        MessageBox(NULL, "You should chose file with .dlts or .aits extension.", "Warning", MB_ICONWARNING);
        return;
    }
    HANDLE thDownload = (HANDLE)_beginthreadex(NULL, 0, LoadFile, NULL, 0, NULL);
    CloseHandle(thDownload);
}

UINT CALLBACK LoadFile(PVOID)
{
    string ext;
    if(index_mode.load() == DLTS) ext = "dlts";
    else if(index_mode.load() == AITS) ext = "aits";
    string strName = FileSavePath + FileSaveName + '.' + ext;

    ifstream file;
    file.open(strName);
    if(!file.is_open()) return FALSE;
    /** ������� �������-���� **/
    /* ����� ������ ���������� �� ���������� ������ ����� �������� */
    ResetEvent(hDownloadEvent);
    double temp = 0.0, capacity = 1.0;
    /** ��������� ��������� **/
    file >> measure_time_DAQ >> gate_DAQ >> rate_DAQ
         >> Generator.bias >> Generator.amp;
    if(ext == "its") file >> temp;
    /** ������ ������� ������������ ����� **/
    UINT uSamples = rate_DAQ*measure_time_DAQ*0.001, uQuantity = 0;
    ifstream LoadFile2;
    LoadFile2.open(strName.data());
    LoadFile2.seekg (0, LoadFile2.end);
    uQuantity = (UINT)((LoadFile2.tellg()/8.5)/uSamples);
    LoadFile2.close();
    ClearMemmory();
    /** �������� **/
    int i = 0;
    while(!file.eof())
    {
        if(index_mode.load() == DLTS)
            file >> temp >> capacity;
        else if(index_mode.load() == AITS)
        {
            double bias = 0.0, amp = 0.0;
            file >> bias >> amp;
            itsBiasVoltages.push_back(bias);
            itsAmpVoltages.push_back(amp);
        }
        if(index_mode.load() == DLTS && !xAxisDLTS.empty() && temp == xAxisDLTS.back())
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
        if(index_mode.load() == DLTS)
        {
            int offset = 0;
            for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
                if(temp > *it) offset++;
            AddPointsDLTS(&vRelaxation, temp, capacity); //�������� �������� �����������
            EnterCriticalSection(&csSavedRelaxation);
                SavedRelaxations.insert(SavedRelaxations.begin()+offset, vRelaxation);
                SavedCapacity.insert(SavedCapacity.begin()+offset, capacity);
                index_relax.store(offset);
            LeaveCriticalSection(&csSavedRelaxation);
        }
        if(index_mode.load() == AITS)
        {
            EnterCriticalSection(&csSavedRelaxation);
                SavedRelaxations.push_back(vRelaxation);
                index_relax.store(SavedRelaxations.size() - 1); //��� �������, ������ ��� ������
            LeaveCriticalSection(&csSavedRelaxation);
        }
        SendMessage(hProgress, PBM_SETPOS, 100.0*i/uQuantity, 0);
        i++;
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    file.close();
    SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_RELAX, 0);
    SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
    /** ������������� ���� ��������� �������� **/
    SetEvent(hDownloadEvent);
    return TRUE;
}

VOID SaveWindow(SAVE_MODE _mode)
{
    static SAVE_MODE mode = _mode;
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
    }
    strcpy(SaveFileName, FileSaveName.data()); /* ��� ����� �� ��������� */
    const string ext = "txt", filter = "*.txt\0\0";
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
    /* ��������� ����� ����� ����� ��� ���������� */
    HANDLE thSave = (HANDLE)_beginthreadex(NULL, 0, SaveFile, (PVOID)&mode, 0, NULL);
    CloseHandle(thSave);
}

UINT CALLBACK SaveFile(PVOID _mode)
{
    SAVE_MODE mode =  *(SAVE_MODE*)_mode;
    if(SavedRelaxations.empty() && mode != SAVE_SETTINGS)
        return -1;
    MessageBox(0,to_string(*(SAVE_MODE*)_mode).c_str(),"",0);
    switch(mode)
    {
        case SAVE_SETTINGS:
            return SaveSettings();
        case SAVE_RELAXATIONS:
            return SaveRelax();
        case SAVE_DLTS:
            return SaveDLTS();
        case SAVE_ARRHENIUS:
            return SaveArrhenius();
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
    iniFile.WriteDoubleFix("DAQ", "gate [�s]", gate_DAQ, TIME_PRECISION);
    /* ��������� ��������� ����� SULA */
    iniFile.WriteInt("SULA", "Capacity range [pF]", int_range_sula[RANGE_SULA_index]);
    iniFile.WriteInt("SULA", "Pre-amplifier", int_pre_amplifier[PRE_AMP_GAIN_SULA_index]);
    /* ��������� ��������� ����� ���������� ��������� */
    if(Generator.is_active.load())
    {
        iniFile.WriteDoubleFix("Generator", "Period", Generator.period, TIME_PRECISION);
        iniFile.WriteDoubleFix("Generator", "Width", Generator.width, TIME_PRECISION);
    }
    /* ��������� ��������� ����� ITS */
    switch(index_mode.load())
    {
        case DLTS:
            iniFile.WriteDoubleFix("Generator", "Amplitude", Generator.amp, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "Bias", Generator.bias, VOLTAGE_PRECISION);
            break;
        case AITS:
            iniFile.WriteDoubleFix("Generator", "Step voltage", Generator.step_amp, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "Begin amplitude", Generator.begin_amp, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "End amplitude", Generator.end_amp, VOLTAGE_PRECISION);
            break;
        case CV:
        case BITS:
            iniFile.WriteDoubleFix("Generator", "Step voltage", Generator.step_bias, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "Begin bias", Generator.begin_bias, VOLTAGE_PRECISION);
            iniFile.WriteDoubleFix("Generator", "End bias", Generator.end_bias, VOLTAGE_PRECISION);
            break;
        case PITS:
            break;
    }
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
    /* ����� ������� */
    file << "X\tY\tY_MSQ" << endl << fixed;
    /* ������� */
    for(size_t i = 0; i < uSample; i++)
    {
        file << setprecision(2) << xAxisAr.at(i) << setprecision(2);
        for(size_t j = 0; j < yAxisAr.size(); j++)
        {
            file << '\t' << yAxisAr[j][i];
        }
        file << endl;
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
    /* ����� ������� */
    file << fixed << setprecision(THERMO_PRECISION);
    file << "[ms]";
    EnterCriticalSection(&csSavedRelaxation);
        size_t uSample = SavedRelaxations[0].size();
        file << "\t" << xAxisDLTS[0];
        for(size_t j = 1; j < SavedRelaxations.size(); j++)
            file << "\t\t" << xAxisDLTS[j];
        file << endl;
        for(size_t i = 0; i < uSample; i++)
        {
            file << setprecision(TIME_PRECISION) << (0.001*gate_DAQ + i*dt)
                 << setprecision(VOLTAGE_PRECISION);
            for(size_t j = 0; j < SavedRelaxations.size(); j++)
            {
                file << '\t' << SavedRelaxations[j][i];
            }
            file << endl;
            progress = 99*i/(uSample);
            SendMessage(hProgress, PBM_SETPOS, progress, 0);
        }
    LeaveCriticalSection(&csSavedRelaxation);
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
    for(size_t i = 0; i < uSample; i++)
    {
        file << setprecision(THERMO_PRECISION) << xAxisDLTS.at(i) << setprecision(10);
        for(size_t j = 0; j < CorTime.size(); j++)
        {
            file << '\t' << yAxisDLTS[j][i];
        }
        file << endl;
        progress = 99*i/(uSample);
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    file.close();
    return 0;
}
