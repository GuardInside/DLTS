#include <facility.h>
#include "ini.h"

BOOL CALLBACK SuccessWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        SetTimer(hWnd, 0, 800, NULL);
        return TRUE;
    case WM_TIMER:
        KillTimer(hWnd, 0);
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
    ini::File iniFile{"settings.ini"};
        /* ���������������� ��������� */
    /* ��������� ��������� ����� ���������� */
    iniFile.WriteDoubleFix("Thermostat", "step", Thermostat.TempStep, THERMO_PRECISION);
    iniFile.WriteDoubleFix("Thermostat", "disp", Thermostat.TempDisp, THERMO_PRECISION);
    /* ��������� ��������� ����� ��������� */
    iniFile.WriteInt("DAQ", "range", index_range);
    iniFile.WriteInt("DAQ", "averaging", averaging_DAQ);
    iniFile.WriteInt("DAQ", "rate", rate_DAQ);
    iniFile.WriteInt("DAQ", "time", measure_time_DAQ);
    iniFile.WriteDoubleFix("DAQ", "gate", gate_DAQ, 2);
    /* ��������� ��������� ����� SULA */
    iniFile.WriteInt("SULA", "range_capacity", RANGE_SULA_index);
    iniFile.WriteInt("SULA", "pre_amp_gain", PRE_AMP_GAIN_SULA_index);
    /* ��������� ��������� ����� ���������� ��������� */
    iniFile.WriteDoubleFix("Generator", "period", Generator.period, 2);
    iniFile.WriteDoubleFix("Generator", "width", Generator.width, 2);
    iniFile.WriteDoubleFix("Generator", "amplitude", Generator.amplitude, 3);
    iniFile.WriteDoubleFix("Generator", "bias", Generator.bias, 3);
    iniFile.WriteBool("Generator", "isActive", Generator.is_active);
    /* ��������� ��������� ����� ITS */
    iniFile.WriteDoubleFix("Generator", "step_voltage", Generator.step_voltage, 3);
    iniFile.WriteDoubleFix("Generator", "begin_amplitude", Generator.begin_amplitude, 3);
    iniFile.WriteDoubleFix("Generator", "end_amplitude", Generator.end_amplitude, 3);
    /* ��������� ��������� ����� ����� */
    iniFile.WriteString("General", "save_path", FileSavePath);
    iniFile.WriteString("General", "file", FileSaveName);
    iniFile.WriteInt("General", "mode", index_mode);
    /* ��������� ��������� ����� �������������� ������ */
    iniFile.WriteDoubleSc("Model", "mass", dEfMass, 3);
    iniFile.WriteInt("Model", "g", (int)dFactorG);
        /* ���������� ��������� */
    /* ����������� ��������� ���������� */
    iniFile.WriteInt("Thermostat", "GPIB", Thermostat.GetID());
    /* ����������� ��������� ���������� */
    iniFile.WriteInt("Generator", "GPIB", Generator.GetID());
    iniFile.WriteInt("Generator", "ch", Generator.channel);
    /* ����������� ��������� DAQ */
    iniFile.WriteInt("DAQ", "Dev", id_DAQ);
    iniFile.WriteInt("DAQ", "trigger", pfi_ttl_port);
    iniFile.WriteInt("DAQ", "ai_relax", ai_port);
    iniFile.WriteInt("DAQ", "ai_pulse", ai_port_pulse);
    iniFile.WriteInt("DAQ", "ai_capacity", ai_port_capacity);
        /* ��������� ������������ */
    /* ��������� ������������ */
    iniFile.WriteDoubleFix("�orrelator", "width", correlation_width, 2);
    iniFile.WriteDoubleFix("�orrelator", "c", correlation_c, 2);
    iniFile.WriteInt("�orrelator", "type", CorType);
    /* ���������� ������������ */
    iniFile.WriteInt("�orrelator", "amount", CorTime.size());
    stringstream buff;
    for(size_t i = 0; i < CorTime.size(); i++)
    {
        rewrite(buff) << "_" << i;
        iniFile.WriteDoubleFix("�orrelator", buff.str(), CorTime[i], 2);
    }
        /* ��������� ������������� */
    iniFile.WriteDoubleSc("Fitting", "abs_error", AprErr, 0);
    iniFile.WriteInt("Fitting", "max_iter", AprIter);
    iniFile.WriteBool("Fitting", "use_for_relax", AprEnableRelax);
    iniFile.WriteBool("Fitting", "use_for_dlts", AprEnableDLTS);
        /* ��������� PID ������� */
    iniFile.Rename("pid.ini");
    for(int i = 0; i < QUANTITY_ZONE; i++)
    {
        rewrite(buff) << "ZONE " << i;
        iniFile.WriteInt(buff.str().data(), "TEMP", Thermostat.ZoneTable.upper_boundary[i]);
        iniFile.WriteInt(buff.str().data(), "P", Thermostat.ZoneTable.P[i]);
        iniFile.WriteInt(buff.str().data(), "I", Thermostat.ZoneTable.I[i]);
        iniFile.WriteInt(buff.str().data(), "D", Thermostat.ZoneTable.D[i]);
        iniFile.WriteInt(buff.str().data(), "RANGE", Thermostat.ZoneTable.range[i]);
    }
    HANDLE hThreadSuccess = (HANDLE)_beginthreadex(NULL, 0, dlg_success, NULL, 0, NULL);
    CloseHandle(hThreadSuccess);
}

void read_settings()
{
    ini::File iniFile{"settings.ini"};
        /* ���������������� ��������� */
    /* ��������� ��������� ����� ���������� */
    iniFile.ReadDouble("Thermostat", "step", &Thermostat.TempStep);
    iniFile.ReadDouble("Thermostat", "disp", &Thermostat.TempDisp);
    /* ��������� ��������� ����� ��������� */
    iniFile.ReadInt("DAQ", "range", (int*)&index_range);
    iniFile.ReadInt("DAQ", "averaging", (int*)&averaging_DAQ);
    iniFile.ReadDouble("DAQ", "rate", &rate_DAQ);
    iniFile.ReadInt("DAQ", "time", (int*)&measure_time_DAQ);
    iniFile.ReadDouble("DAQ", "gate", &gate_DAQ);
    /* ��������� ��������� ����� SULA */
    iniFile.ReadInt("SULA", "range_capacity", &RANGE_SULA_index);
    iniFile.ReadInt("SULA", "pre_amp_gain", &PRE_AMP_GAIN_SULA_index);
    /* ��������� ��������� ����� ���������� ��������� */
    iniFile.ReadDouble("Generator", "period", &Generator.period);
    iniFile.ReadDouble("Generator", "width", &Generator.width);
    iniFile.ReadDouble("Generator", "amplitude", &Generator.amplitude);
    iniFile.ReadDouble("Generator", "bias", &Generator.bias);
    iniFile.ReadBool("Generator", "isActive", &Generator.is_active);
    /* ��������� ��������� ����� ITS */
    iniFile.ReadDouble("Generator", "step_voltage", &Generator.step_voltage);
    iniFile.ReadDouble("Generator", "begin_amplitude", &Generator.begin_amplitude);
    iniFile.ReadDouble("Generator", "end_amplitude", &Generator.end_amplitude);
    /* ��������� ��������� ����� ����� */
    iniFile.ReadString("General", "save_path", &FileSavePath);
    iniFile.ReadString("General", "file", &FileSaveName);
    iniFile.ReadInt("General", "mode", (int*)&index_mode);
    /* ��������� ��������� ����� �������������� ������ */
    iniFile.ReadDouble("Model", "mass", &dEfMass);
    iniFile.ReadInt("Model", "g", (int*)(&dFactorG));
        /* ���������� ��������� */
    /* ����������� ��������� ���������� */
    iniFile.ReadInt("Thermostat", "GPIB", &Thermostat.GetID());
    /* ����������� ��������� ���������� */
    iniFile.ReadInt("Generator", "GPIB", &Generator.GetID());
    iniFile.ReadInt("Generator", "ch", (int*)&Generator.channel);
    /* ����������� ��������� DAQ */
    iniFile.ReadInt("DAQ", "Dev", (int*)&id_DAQ);
    iniFile.ReadInt("DAQ", "trigger", (int*)&pfi_ttl_port);
    iniFile.ReadInt("DAQ", "ai_relax", (int*)&ai_port);
    iniFile.ReadInt("DAQ", "ai_pulse", (int*)&ai_port_pulse);
    iniFile.ReadInt("DAQ", "ai_capacity", (int*)&ai_port_capacity);
        /* ��������� ������������ */
    /* ��������� ������������ */
    iniFile.ReadDouble("�orrelator", "width", &correlation_width);
    iniFile.ReadDouble("�orrelator", "c", &correlation_c);
    iniFile.ReadInt("�orrelator", "type", (int*)&CorType);
    /* ���������� ������������ */
    int amount = 0;
    double value = 0.0;
    iniFile.ReadInt("�orrelator", "amount", &amount);
    stringstream buff;
    CorTime.clear();
    for(size_t i = 0; i < (size_t)amount; i++)
    {
        rewrite(buff) << "_" << i;
        iniFile.ReadDouble("�orrelator", buff.str(), &value);
        CorTime.push_back(value);
    }
    if(yAxisDLTS != nullptr)
        delete []yAxisDLTS;
    yAxisDLTS = new vector<double>[CorTime.size()]; /* ����� ���� ��������� � ������ ������������ */
        /* ��������� ������������� */
    iniFile.ReadDouble("Fitting", "abs_error", &AprErr);
    iniFile.ReadInt("Fitting", "max_iter", &AprIter);
    iniFile.ReadBool("Fitting", "use_for_relax", &AprEnableRelax);
    iniFile.ReadBool("Fitting", "use_for_dlts", &AprEnableDLTS);
        /* ��������� PID ������� */
    iniFile.Rename("pid.ini");
    for(int i = 0; i < QUANTITY_ZONE; i++)
    {
        rewrite(buff) << "ZONE " << i;
        iniFile.ReadInt(buff.str().data(), "TEMP", (int*)&Thermostat.ZoneTable.upper_boundary[i]);
        iniFile.ReadInt(buff.str().data(), "P", (int*)&Thermostat.ZoneTable.P[i]);
        iniFile.ReadInt(buff.str().data(), "I", (int*)&Thermostat.ZoneTable.I[i]);
        iniFile.ReadInt(buff.str().data(), "D", (int*)&Thermostat.ZoneTable.D[i]);
        iniFile.ReadInt(buff.str().data(), "RANGE", (int*)&Thermostat.ZoneTable.range[i]);
    }
}

/* ��������� ������� ��������� ��� � ����������� ���������� */
/* ����������� ��� ������� �� ������ ����� */
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
        rewrite(buff) << "ZONE " << i+1 << ",+" << Thermostat.ZoneTable.upper_boundary[i] << ","
                      << Thermostat.ZoneTable.range[i] << "," << Thermostat.ZoneTable.P[i] << ","
                      << Thermostat.ZoneTable.I[i] << "," << Thermostat.ZoneTable.D[i];
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
    /* �������� ������������ */
    for(const auto &t: CorTime)
        if(t*correlation_c > measure_time_DAQ) /* ��� � �� */
            MessageBox(NULL, "You must change correlation settings.", "Note", MB_ICONINFORMATION);
}
