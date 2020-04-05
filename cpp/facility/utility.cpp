#include "facility.h"
#include <fstream>

#include "graph.h"
#include "ini.h"
#include "variable.h"
#include "dlts_math.h"

map<int, map<double,corinfo>> CorInfo;
corinfo get_corinfo(int type, double Tc)
{
    if(CorInfo[type].find(Tc) == CorInfo[type].end())
    {
        double Tg = Tc / correlation_c;
        CorInfo[type][Tc].tau0 = find_tau(Tg, Tc, type);
        CorInfo[type][Tc].SN   = SN(Tg, Tc, type);
        CorInfo[type][Tc].lw   = lw(Tg, Tc, type);
    }

    return CorInfo[type][Tc];
}

int GetResPidTable(int i, string const &str)
{
    if(str == "UPPER_BOUNDARY")
    {
        const int id_upper_bound[] = {ID_EDITCONTROL_UBOUNDARY_1, ID_EDITCONTROL_UBOUNDARY_2, ID_EDITCONTROL_UBOUNDARY_3,
                            ID_EDITCONTROL_UBOUNDARY_4, ID_EDITCONTROL_UBOUNDARY_5, ID_EDITCONTROL_UBOUNDARY_6,
                            ID_EDITCONTROL_UBOUNDARY_7, ID_EDITCONTROL_UBOUNDARY_8, ID_EDITCONTROL_UBOUNDARY_9,
                            ID_EDITCONTROL_UBOUNDARY_10};
        return id_upper_bound[i];
    }
    else if(str == "P")
    {
        const int id_p[] = {ID_EDITCONTROL_P_1, ID_EDITCONTROL_P_2, ID_EDITCONTROL_P_3, ID_EDITCONTROL_P_4,
                        ID_EDITCONTROL_P_5, ID_EDITCONTROL_P_6, ID_EDITCONTROL_P_7, ID_EDITCONTROL_P_8,
                        ID_EDITCONTROL_P_9, ID_EDITCONTROL_P_10};
        return id_p[i];
    }
    else if(str == "I")
    {
        const int id_i[] = {ID_EDITCONTROL_I_1, ID_EDITCONTROL_I_2, ID_EDITCONTROL_I_3, ID_EDITCONTROL_I_4,
                        ID_EDITCONTROL_I_5, ID_EDITCONTROL_I_6, ID_EDITCONTROL_I_7, ID_EDITCONTROL_I_8,
                        ID_EDITCONTROL_I_9, ID_EDITCONTROL_I_10};
        return id_i[i];
    }
    else if(str == "D")
    {
        const int id_d[] = {ID_EDITCONTROL_D_1, ID_EDITCONTROL_D_2, ID_EDITCONTROL_D_3, ID_EDITCONTROL_D_4,
                        ID_EDITCONTROL_D_5, ID_EDITCONTROL_D_6, ID_EDITCONTROL_D_7, ID_EDITCONTROL_D_8,
                        ID_EDITCONTROL_D_9, ID_EDITCONTROL_D_10};
        return id_d[i];
    }
    return -1;
}

double MeanSquareErrorOfTemp(std::vector<double>::const_iterator b,
                             std::vector<double>::const_iterator e)
{
    /* ���������� ��������� N �������� ����������� ��� �������� �������� � ����������,
       ��� ���� ����� �������� ������� ������������ */
    constexpr static const size_t LNS = AVERAGING_TIME/(0.001*REFRESH_TIME_THERMOSTAT);
    size_t range = static_cast<size_t>( std::abs( std::distance(b, e) ) );
    size_t diff = LNS;
    if(range < LNS) diff = range;
    double result = MeanSquareError(e - diff, e);
    constexpr double lower_MSK = pow(10, -THERMO_PRECISION);
    return (result < lower_MSK) ? 0.0 : result;
}

double MeanOfTemp(std::vector<double>::const_iterator b,
                  std::vector<double>::const_iterator e)
{
    /* ���������� ��������� N �������� ����������� ��� �������� �������� � ����������,
       ��� ���� ����� �������� ������� ������������ */
    constexpr static const size_t LNS = 0.001*REFRESH_TIME_THERMOSTAT*AVERAGING_TIME;
    size_t range = static_cast<size_t>( std::abs( std::distance(b, e) ) );
    size_t diff = LNS;
    if(range < LNS) diff = range;
    return Mean(e - diff, e);
}

void FileOpeningTest(ofstream& file)
{
    if(!file.is_open())
    {
        MessageBox(NULL, "Save file isn't being found.", "Error", MB_ICONERROR);
        return;
    }
}

void VoltageToCapacity(double *value)
{
    static const double _SULA_CONST = 0.2;
    *value = _SULA_CONST*(*value)*int_range_sula[RANGE_SULA_index];// / int_pre_amp_gain[PRE_AMP_GAIN_SULA_index];
}

void ClearMemmory()
{
    /* �������� ��������� �������� */
    auto_peak_search.store(true);
    SendMessage(hMainWindow, WM_COMMAND, WM_REFRESH_MENU, 0);
    ClearMemmoryDLTS();
    SavedCapacity.clear();
    SavedRelaxations.clear();
    CorInfo.clear();
    gwin::gDefaultPlot(hRelax, "\0");
    gwin::gDefaultPlot(hGraph_DLTS, "\0");
}

void ClearMemmoryDLTS()
{
    yAxisDLTS.clear();
    yAxisDLTS.resize(CorTc.size());
    xAxisDLTS.clear();

    yAxisITS.clear();
    xAxisITS.clear();
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
        AddPointsDLTS(&SavedRelaxations[i], xAxis[i], SavedCapacity[i]);
        /* ���������� ���������� � ���� ������ */
        progress = 99 * i/(xAxis.size());
        SendMessage(hProgress, PBM_SETPOS, progress, 0);
    }
    SendMessage(hProgress, PBM_SETPOS, 0, 0);
    auto_peak_search.store(true);
    SendMessage(hMainWindow, WM_COMMAND, WM_REFRESH_MENU, 0);
    SendMessage(hMainWindow, WM_COMMAND, WM_PAINT_DLTS, 0);
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
        /* ��������� ���������, ���� ���� ������ ������� */
        file << fixed << setprecision(0)
             << "Time: " << measure_time_DAQ << " [ms]\n"
             << "Gate: " << gate_DAQ << " [mcs]\n"
             << "Rate: " << rate_DAQ << " [Hz]\n"
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
         << MeanTemp
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

/* ���������� ���������� ��������� ����� */
string GetExtensionFile(string str)
{
    for(size_t i = 0; i < str.length(); i++)
        if(str[i] == '.')
            return string(str, i);
    return string("Incorrect format");
}
