#include <facility.h>

void write_settings()
{
    ofstream file(SettingsFile.data());
    ofstream CorFile(SettingsCorFile.data());
    ofstream PIDFile(SettingsPIDFile.data());
    if(file.is_open())
        file << Thermostat.GetID() << " "  << Generator.GetID() << " "
        << Thermostat.TempStep << " " << Thermostat.TempDisp << " "
        << id_DAQ << " " << measure_time_DAQ << " " << rate_DAQ << " "
        << averaging_DAQ << " " << ai_port << ai_port_pulse << " " << pfi_ttl_port << " "
        << gate_DAQ << " " << FileSaveName << " " << index_mode << " " << aver_time << " " << index_range << " "
        << Generator.period << " " << Generator.width << " "
        << Generator.voltage_up << " " << Generator.voltage_low << " " << Generator.is_active << " "
        << Generator.step_voltage << " " << Generator.begin_voltage << " " << Generator.end_voltage;
    else
        MessageBox(NULL, "The settings file wasn't found.", "Information", MB_ICONINFORMATION);
    if(CorFile.is_open())
    {
        CorFile << correlation_width << " " << correlation_c << " " << CorType << endl;
        for(size_t i = 0; i < CorTime.size(); i++)
        {
            CorFile << CorTime[i];
            if(i != CorTime.size() - 1)
                CorFile << " ";
        }
    }
    else
        MessageBox(NULL, "The correlation settings file wasn't found.", "Information", MB_ICONINFORMATION);
    if(PIDFile.is_open())
    {
        for(int i = 0; i < QUANTITY_ZONE; i++)
        {
            //ZONE XX,�SSS.S,R,PPP,III,DDD
            PIDFile << i << " " << ZoneTable.upper_boundary[i] << " "
                      << ZoneTable.P[i] << " " << ZoneTable.I[i] << " " << ZoneTable.D[i]
                      << " " << ZoneTable.range[i];
            if(i != QUANTITY_ZONE)
                PIDFile << endl;
        }
    }
    else
        MessageBox(NULL, "The PID settings file wasn't found.", "Information", MB_ICONINFORMATION);
    PIDFile.close();
    CorFile.close();
    file.close();
}

void read_settings()
{
    /* ��� �������� �������� */
    ifstream file(SettingsFile.data());
    /* ��� �������� ������������ */
    ifstream CorFile(SettingsCorFile.data());
    /* ��� ��� �������� */
    ifstream PIDFile(SettingsPIDFile.data());
    double buff = 0.0;
    if(file.is_open())
        file >> Thermostat.GetID() >> Generator.GetID()
        >> Thermostat.TempStep >> Thermostat.TempDisp
        >> id_DAQ >> measure_time_DAQ >> rate_DAQ >> averaging_DAQ >> ai_port >> ai_port_pulse >> pfi_ttl_port
        >> gate_DAQ >> FileSaveName >> index_mode >> aver_time >> index_range
        >> Generator.period >> Generator.width >> Generator.voltage_up >> Generator.voltage_low
        >> Generator.is_active >> Generator.step_voltage >> Generator.begin_voltage >> Generator.end_voltage;
    else
        MessageBox(NULL, "The settings file wasn't found. It'll be created when you set new settings.", "Information", MB_ICONINFORMATION);
    if(CorFile.is_open())
    {
        CorFile >> correlation_width >> correlation_c >> CorType;
        while(!CorFile.eof())
        {
            CorFile >> buff;
            CorTime.push_back(buff);
        }
        if(yAxisDLTS != nullptr)
            delete []yAxisDLTS;
        yAxisDLTS = new vector<double>[CorTime.size()]; ///����� ���� ��������� � ������ ������������
    }
    else
        MessageBox(NULL, "The correlation settings file wasn't found.", "Information", MB_ICONINFORMATION);
    if(PIDFile.is_open())
    {
        int i = 0;
        while(!PIDFile.eof())
        {
            PIDFile >> i;
            //ZONE XX,�SSS.S,R,PPP,III,DDD
            PIDFile >> ZoneTable.upper_boundary[i]
                    >> ZoneTable.P[i] >> ZoneTable.I[i] >> ZoneTable.D[i]
                    >> ZoneTable.range[i];
        }
    }
    else
        MessageBox(NULL, "The PID settings file wasn't found.", "Information", MB_ICONINFORMATION);
    PIDFile.close();
    CorFile.close();
    file.close();
}

/* ��������� ������� ��������� ��� � ����������� ���������� */
/* ����������� ��� ������� �� ������ ����� */
/*ZONE XX,�SSS.S,R,PPP,III,DDD
        GAIN 65 [term] instructs the Model 330 to set control gain to 65. Gain corresponds to the
            Proportional (P) portion of the PID Autotuning control algorithm.
        Enter an integer from 0 to 999. Reset corresponds to the Integral (I) portion of the PID
            Autotuning control algorithm.
        Enter an integer from 0 through 200. Rate corresponds to the Differential (D) portion of the.
            PID Autotuning control algorithm.*/
void SetZones()
{
    stringstream buff;
    buff << setprecision(2);
    /* ������� � ����� ������������� ��� */
    rewrite(buff) << "TUNE " << 4; /* PID, 0 = Manual, 1 = P, 2 = PI, 3 = PID, 4 = Zone */
    Thermostat.Write(buff);
    /* ��������� ��������� ��� */
    for(int i = 0; i < QUANTITY_ZONE; i++)
    {
        rewrite(buff) << "ZONE " << i+1 << ",+" << ZoneTable.upper_boundary[i] << ","
                      << ZoneTable.range[i] << "," << ZoneTable.P[i] << ","
                      << ZoneTable.I[i] << "," << ZoneTable.D[i];
        Thermostat.Write(buff);
    }
}

//������������� �������� ������, ���������� � ��� ��� PID �����������
void ApplySettings()
{
    Thermostat.InitSession();
    Thermostat.Apply();
    if(Generator.is_active)
    {
        Generator.InitSession();
        Generator.Apply();
    }
    DAQmxClearState();
}