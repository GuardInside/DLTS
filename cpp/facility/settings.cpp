#include <facility.h>

BOOL CALLBACK SuccessWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        SetTimer(hWnd, 0, 800, NULL);
        return TRUE;
    case WM_TIMER:
        EndDialog(hWnd, 0);
        return TRUE;
    }
    return FALSE;
}

UINT CALLBACK dlg_success(PVOID)
{
    return DialogBox(hInst, MAKEINTRESOURCE(ID_SUCCESS_WINDOW), HWND_DESKTOP, SuccessWndProc);
}

void write_settings()
{
    ofstream file(SettingsFile.data());
    ofstream CorFile(SettingsCorFile.data());
    ofstream PIDFile(SettingsPIDFile.data());
    if(file.is_open())
        file << Thermostat.GetID() << " "  << Generator.GetID() << " "
        << Thermostat.TempStep << " " << Thermostat.TempDisp << " "
        << id_DAQ << " " << measure_time_DAQ << " " << rate_DAQ << " "
        << averaging_DAQ << " " << ai_port << " " << ai_port_pulse << " " << pfi_ttl_port << " "
        << gate_DAQ << " " << FileSaveName << " " << index_mode << " " << aver_time << " " << index_range << " "
        << Generator.period << " " << Generator.width << " "
        << Generator.amplitude << " " << Generator.bias << " " << Generator.is_active << " "
        << Generator.step_voltage << " " << Generator.begin_amplitude << " " << Generator.end_amplitude;
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
            //ZONE XX,±SSS.S,R,PPP,III,DDD
            PIDFile << i << " " << Thermostat.ZoneTable.upper_boundary[i] << " "
                      << Thermostat.ZoneTable.P[i] << " " << Thermostat.ZoneTable.I[i] << " " << Thermostat.ZoneTable.D[i]
                      << " " << Thermostat.ZoneTable.range[i];
            if(i != QUANTITY_ZONE)
                PIDFile << endl;
        }
    }
    else
        MessageBox(NULL, "The PID settings file wasn't found.", "Information", MB_ICONINFORMATION);
    PIDFile.close();
    CorFile.close();
    file.close();
    HANDLE hThreadSuccess = (HANDLE)_beginthreadex(NULL, 0, dlg_success, NULL, 0, NULL);
    CloseHandle(hThreadSuccess);
}

void read_settings()
{
    /* Для основных настроек */
    ifstream file(SettingsFile.data());
    /* Для настроек корреляторов */
    ifstream CorFile(SettingsCorFile.data());
    /* Для ПИД настроек */
    ifstream PIDFile(SettingsPIDFile.data());
    double buff = 0.0;
    if(file.is_open())
        file >> Thermostat.GetID() >> Generator.GetID()
        >> Thermostat.TempStep >> Thermostat.TempDisp
        >> id_DAQ >> measure_time_DAQ >> rate_DAQ >> averaging_DAQ >> ai_port >> ai_port_pulse >> pfi_ttl_port
        >> gate_DAQ >> FileSaveName >> index_mode >> aver_time >> index_range
        >> Generator.period >> Generator.width >> Generator.amplitude >> Generator.bias
        >> Generator.is_active >> Generator.step_voltage >> Generator.begin_amplitude >> Generator.end_amplitude;
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
        yAxisDLTS = new vector<double>[CorTime.size()]; ///Число осей совпадает с числом корреляторов
    }
    else
        MessageBox(NULL, "The correlation settings file wasn't found.", "Information", MB_ICONINFORMATION);
    if(PIDFile.is_open())
    {
        int i = 0;
        while(!PIDFile.eof())
        {
            PIDFile >> i;
            //ZONE XX,±SSS.S,R,PPP,III,DDD
            PIDFile >> Thermostat.ZoneTable.upper_boundary[i]
                    >> Thermostat.ZoneTable.P[i] >> Thermostat.ZoneTable.I[i] >> Thermostat.ZoneTable.D[i]
                    >> Thermostat.ZoneTable.range[i];
        }
    }
    else
        MessageBox(NULL, "The PID settings file wasn't found.", "Information", MB_ICONINFORMATION);
    PIDFile.close();
    CorFile.close();
    file.close();
}

/* Применяет текущие настройки зон к физическому устройству */
/* Применяются при нажатии на кнопку старт */
void SetZones()
{
    stringstream buff;
    buff << setprecision(2);
    /* Переход в режим использования зон */
    rewrite(buff) << "TUNE " << 4; /* PID, 0 = Manual, 1 = P, 2 = PI, 3 = PID, 4 = Zone */
    Thermostat.Write(buff);
    /* Применяем настройки зон */
    for(int i = 0; i < QUANTITY_ZONE; i++)
    {
        rewrite(buff) << "ZONE " << i+1 << ",+" << Thermostat.ZoneTable.upper_boundary[i] << ","
                      << Thermostat.ZoneTable.range[i] << "," << Thermostat.ZoneTable.P[i] << ","
                      << Thermostat.ZoneTable.I[i] << "," << Thermostat.ZoneTable.D[i];
        Thermostat.Write(buff);
    }
}

//Инициализация сборщика данных, термостата и зон для PID регулировки
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
