#include <sstream>
#include <string>
#include <exception>

#include "facility.h"
#include "variable.h"
#include "gwin.h"
#include "graph.h"
#include "ini.h"

using namespace std;

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
        file.lpstrFilter = "All files(*.*)\0*.*\0DLTS file(*.dlts)\0*.dlts\0ITS file(*.its)\0*.its\0\0";
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
        index_mode = DLTS;
    else if(ext == "its")
        index_mode = ITS;
    else
    {
        MessageBox(NULL, "You should chose file with .dlts or .its extension.", "Warning", MB_ICONWARNING);
        return;
    }
    HANDLE thDownload = (HANDLE)_beginthreadex(NULL, 0, LoadFile, NULL, 0, NULL);
    CloseHandle(thDownload);
}

UINT CALLBACK LoadFile(PVOID)
{
    string ext;
    if(index_mode == DLTS) ext = "dlts";
    else if(index_mode == ITS) ext = "its";
    string strName = FileSavePath + FileSaveName + '.' + ext;
    ifstream file;
    file.open(strName);
    if(!file.is_open())
        return FALSE;
    /** ������� �������-���� **/
    /* ����� ������ ���������� �� ���������� ������ ����� �������� */
    ResetEvent(hDownloadEvent);
    double itsBias = 0.0, itsAmp = 0.0;
    double buff = 0.0, temp = 0.0;
    /** ��������� ��������� **/
    file >> averaging_DAQ >> measure_time_DAQ >> rate_DAQ >> gate_DAQ
         >> Generator.amplitude >> Generator.bias;
    if(ext == "its") file >> itsTemperature;
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
        if(index_mode == DLTS)
            file >> temp;
        else if(index_mode == ITS)
        {
            file >> itsBias >> itsAmp;
            itsBiasVoltages.push_back(itsBias);
            itsAmpVoltages.push_back(itsAmp);
        }
        if(index_mode == DLTS && !xAxisDLTS.empty() && temp == xAxisDLTS.back())
            continue;
        vector <double> vRelaxation;
        UINT uSamples = rate_DAQ*measure_time_DAQ*0.001;
        for(size_t i = 0; i < uSamples; i++)
        {
            file >> buff;
            vRelaxation.push_back(buff);
        }
        if(index_mode == DLTS)
        {
            int offset = 0;
            for(auto it = xAxisDLTS.begin(); it != xAxisDLTS.end(); it++)
                if(temp > *it) offset++;
            AddPointsDLTS(&vRelaxation, temp); //�������� �������� �����������
            SavedRelaxations.insert(SavedRelaxations.begin()+offset, vRelaxation);
            index_relax = offset;
        }
        if(index_mode == ITS)
        {
            SavedRelaxations.push_back(vRelaxation);
            index_relax = SavedRelaxations.size() - 1; //��� �������, ������ ��� ������
        }
        SendMessage(hProgress, PBM_SETPOS, 100.0*i/uQuantity, 0);
        i++;
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    //������ ������������ ��� ��������� ������ ����� ����������
    file.close();
    PlotRelax();
    PlotDLTS();
    /** ������������� ���� ��������� �������� **/
    SetEvent(hDownloadEvent);
    return TRUE;
}

VOID SaveWindow(SAVE_MODE mode)
{
    string strTitle;
    switch(mode)
    {
    case SAVE_RELAXATIONS:
        strTitle = "Save relaxations";
        break;
    case SAVE_DLTS:
        strTitle = "Save DLTS";
        break;
    case SAVE_ARRHENIUS:
        strTitle = "Save Arrhenius";
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

UINT CALLBACK SaveFile(PVOID mode)
{
    if(SavedRelaxations.empty())
        return -1;
    switch(*(SAVE_MODE*)mode)
    {
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

UINT SaveArrhenius()
{
    if(xAxisAr.size() == 0)
    {
        MessageBox(NULL, "You should select the Arrhenius plot.", "Information", MB_OK | MB_ICONEXCLAMATION);
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
            file << '\t' << yAxisAr.at(j).at(i);
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
    size_t uSample = SavedRelaxations.at(0).size();
    double dt = 1000.0/rate_DAQ; /* ms */
    /* ����� ������� */
    file << fixed << setprecision(THERMO_PRECISION);
    file << "[ms]";
    for(size_t j = 0; j < SavedRelaxations.size(); j++)
        file << '\t' << xAxisDLTS.at(j);
    file << endl;
    for(size_t i = 0; i < uSample; i++)
    {
        file << setprecision(TIME_PRECISION) << (0.001*gate_DAQ + i*dt)
             << setprecision(VOLTAGE_PRECISION);
        for(size_t j = 0; j < SavedRelaxations.size(); j++)
        {
            file << '\t' << SavedRelaxations.at(j).at(i);
        }
        file << endl;
        progress = 99*i/(uSample);
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
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
            file << '\t' << yAxisDLTS[j].at(i);
        }
        file << endl;
        progress = 99*i/(uSample);
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    file.close();
    return 0;
}
