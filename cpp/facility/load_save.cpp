#include <sstream>
#include <string>
#include <exception>

#include "facility.h"
#include "variable.h"
#include "gwin.h"
#include "graph.h"

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
    if(!GetOpenFileName(&file))
        return;
    /* ��������� ����� ����� ����� ��� ���������� */
    FileSaveName.clear();
    for(WORD i = file.nFileOffset; i < (file.nFileExtension-1); i++)
        FileSaveName.push_back(LoadFileName[i]);
    HANDLE thDownload = (HANDLE)_beginthreadex(NULL, 0, LoadFile, NULL, 0, NULL);
    CloseHandle(thDownload);
}

UINT CALLBACK LoadFile(PVOID)
{
    /* �������� ���� � ����� */
    string strName = Save + FileSaveName;
    if(index_mode == DLTS) strName += ".dlts";
    else if(index_mode == ITS) strName += ".its";
    ifstream file;
    file.open(strName);
    if(!file.is_open())
        return FALSE;
    /* ������� �������-���� */
    ResetEvent(hDownloadEvent);
    double itsUpper_boundary = 0.0, itsLower_boundary = 0.0;
    double buff = 0.0, temp = 0.0;
    string ext = GetExtensionFile(strName);
    file >> averaging_DAQ >> measure_time_DAQ >> rate_DAQ >> gate_DAQ
         >> Generator.amplitude >> Generator.bias;
    if(ext == ".dlts")
        index_mode = DLTS;
    else if(ext == ".its")
    {
        index_mode = ITS;
        file >> itsTemperature;
    }
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
            file >> itsLower_boundary >> itsUpper_boundary;
            itsLowVoltages.push_back(itsLower_boundary);
            itsUpVoltages.push_back(itsUpper_boundary);
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
    SetEvent(hDownloadEvent);
    return TRUE;
}

VOID SaveWindow()
{
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
    if(!GetSaveFileName(&file))
        return;
    /* ��������� ����� ����� ����� ��� ���������� */
    HANDLE thSave = (HANDLE)_beginthreadex(NULL, 0, SaveFile, NULL, 0, NULL);
    CloseHandle(thSave);
}

UINT CALLBACK SaveFile(PVOID)
{
    if(SavedRelaxations.empty())
        return -1;
    /* �������� ������ */
    WORD wFlag = 0x00;
    if(string(SaveFileName).find("_DLTS") != string::npos)
        wFlag = 0x01;
    if(string(SaveFileName).find("_AR") != string::npos)
        wFlag = 0x10;
    /* �������� � ������������ � ������ */
    if(wFlag == 0x01)
        return SaveDLTS();
    else if(wFlag == 0x10)
        return SaveArrhenius();
    return SaveRelax();
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
