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
        /* Пользовательские настройки */
    /* Сохраняем настройки блока Термостата */
    iniFile.WriteDoubleFix("Thermostat", "step", Thermostat.TempStep, THERMO_PRECISION);
    iniFile.WriteDoubleFix("Thermostat", "disp", Thermostat.TempDisp, THERMO_PRECISION);
    /* Сохраняем настройки блока Измерений */
    iniFile.WriteInt("DAQ", "range", index_range);
    iniFile.WriteInt("DAQ", "averaging", averaging_DAQ);
    iniFile.WriteInt("DAQ", "rate", rate_DAQ);
    iniFile.WriteInt("DAQ", "time", measure_time_DAQ);
    iniFile.WriteDoubleFix("DAQ", "gate", gate_DAQ, 2);
    /* Сохраняем настройки блока SULA */
    iniFile.WriteInt("SULA", "range_capacity", RANGE_SULA_index);
    iniFile.WriteInt("SULA", "pre_amp_gain", PRE_AMP_GAIN_SULA_index);
    /* Сохраняем настройки блока генератора импульсов */
    iniFile.WriteDoubleFix("Generator", "period", Generator.period, 2);
    iniFile.WriteDoubleFix("Generator", "width", Generator.width, 2);
    iniFile.WriteDoubleFix("Generator", "amplitude", Generator.amplitude, 3);
    iniFile.WriteDoubleFix("Generator", "bias", Generator.bias, 3);
    iniFile.WriteBool("Generator", "isActive", Generator.is_active);
    /* Сохраняем настройки блока ITS */
    iniFile.WriteDoubleFix("Generator", "step_voltage", Generator.step_voltage, 3);
    iniFile.WriteDoubleFix("Generator", "begin_amplitude", Generator.begin_amplitude, 3);
    iniFile.WriteDoubleFix("Generator", "end_amplitude", Generator.end_amplitude, 3);
    /* Сохраняем настройки блока общее */
    iniFile.WriteString("General", "save_path", FileSavePath);
    iniFile.WriteString("General", "file", FileSaveName);
    iniFile.WriteInt("General", "mode", index_mode);
    /* Сохраняем настройки блока математической модели */
    iniFile.WriteDoubleSc("Model", "mass", dEfMass, 3);
    iniFile.WriteInt("Model", "g", (int)dFactorG);
        /* Расширенне настройки */
    /* Расширенные настройки термостата */
    iniFile.WriteInt("Thermostat", "GPIB", Thermostat.GetID());
    /* Расширенные настройки генератора */
    iniFile.WriteInt("Generator", "GPIB", Generator.GetID());
    iniFile.WriteInt("Generator", "ch", Generator.channel);
    /* Расширенные настройки DAQ */
    iniFile.WriteInt("DAQ", "Dev", id_DAQ);
    iniFile.WriteInt("DAQ", "trigger", pfi_ttl_port);
    iniFile.WriteInt("DAQ", "ai_relax", ai_port);
    iniFile.WriteInt("DAQ", "ai_pulse", ai_port_pulse);
    iniFile.WriteInt("DAQ", "ai_capacity", ai_port_capacity);
        /* Настройки корреляторов */
    /* Параметры корреляторов */
    iniFile.WriteDoubleFix("Сorrelator", "width", correlation_width, 2);
    iniFile.WriteDoubleFix("Сorrelator", "c", correlation_c, 2);
    iniFile.WriteInt("Сorrelator", "type", CorType);
    /* Количество корреляторов */
    iniFile.WriteInt("Сorrelator", "amount", CorTime.size());
    stringstream buff;
    for(size_t i = 0; i < CorTime.size(); i++)
    {
        rewrite(buff) << "_" << i;
        iniFile.WriteDoubleFix("Сorrelator", buff.str(), CorTime[i], 2);
    }
        /* Настройки аппроксимации */
    iniFile.WriteDoubleSc("Fitting", "abs_error", AprErr, 0);
    iniFile.WriteInt("Fitting", "max_iter", AprIter);
    iniFile.WriteBool("Fitting", "use_for_relax", AprEnableRelax);
    iniFile.WriteBool("Fitting", "use_for_dlts", AprEnableDLTS);
        /* Настройки PID таблицы */
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
        /* Пользовательские настройки */
    /* Загружаем настройки блока Термостата */
    iniFile.ReadDouble("Thermostat", "step", &Thermostat.TempStep);
    iniFile.ReadDouble("Thermostat", "disp", &Thermostat.TempDisp);
    /* Загружаем настройки блока Измерений */
    iniFile.ReadInt("DAQ", "range", (int*)&index_range);
    iniFile.ReadInt("DAQ", "averaging", (int*)&averaging_DAQ);
    iniFile.ReadDouble("DAQ", "rate", &rate_DAQ);
    iniFile.ReadInt("DAQ", "time", (int*)&measure_time_DAQ);
    iniFile.ReadDouble("DAQ", "gate", &gate_DAQ);
    /* Загружаем настройки блока SULA */
    iniFile.ReadInt("SULA", "range_capacity", &RANGE_SULA_index);
    iniFile.ReadInt("SULA", "pre_amp_gain", &PRE_AMP_GAIN_SULA_index);
    /* Загружаем настройки блока генератора импульсов */
    iniFile.ReadDouble("Generator", "period", &Generator.period);
    iniFile.ReadDouble("Generator", "width", &Generator.width);
    iniFile.ReadDouble("Generator", "amplitude", &Generator.amplitude);
    iniFile.ReadDouble("Generator", "bias", &Generator.bias);
    iniFile.ReadBool("Generator", "isActive", &Generator.is_active);
    /* Загружаем настройки блока ITS */
    iniFile.ReadDouble("Generator", "step_voltage", &Generator.step_voltage);
    iniFile.ReadDouble("Generator", "begin_amplitude", &Generator.begin_amplitude);
    iniFile.ReadDouble("Generator", "end_amplitude", &Generator.end_amplitude);
    /* Загружаем настройки блока общее */
    iniFile.ReadString("General", "save_path", &FileSavePath);
    iniFile.ReadString("General", "file", &FileSaveName);
    iniFile.ReadInt("General", "mode", (int*)&index_mode);
    /* Загружаем настройки блока математической модели */
    iniFile.ReadDouble("Model", "mass", &dEfMass);
    iniFile.ReadInt("Model", "g", (int*)(&dFactorG));
        /* Расширенне настройки */
    /* Расширенные настройки термостата */
    iniFile.ReadInt("Thermostat", "GPIB", &Thermostat.GetID());
    /* Расширенные настройки генератора */
    iniFile.ReadInt("Generator", "GPIB", &Generator.GetID());
    iniFile.ReadInt("Generator", "ch", (int*)&Generator.channel);
    /* Расширенные настройки DAQ */
    iniFile.ReadInt("DAQ", "Dev", (int*)&id_DAQ);
    iniFile.ReadInt("DAQ", "trigger", (int*)&pfi_ttl_port);
    iniFile.ReadInt("DAQ", "ai_relax", (int*)&ai_port);
    iniFile.ReadInt("DAQ", "ai_pulse", (int*)&ai_port_pulse);
    iniFile.ReadInt("DAQ", "ai_capacity", (int*)&ai_port_capacity);
        /* Настройки корреляторов */
    /* Параметры корреляторов */
    iniFile.ReadDouble("Сorrelator", "width", &correlation_width);
    iniFile.ReadDouble("Сorrelator", "c", &correlation_c);
    iniFile.ReadInt("Сorrelator", "type", (int*)&CorType);
    /* Количество корреляторов */
    int amount = 0;
    double value = 0.0;
    iniFile.ReadInt("Сorrelator", "amount", &amount);
    stringstream buff;
    CorTime.clear();
    for(size_t i = 0; i < (size_t)amount; i++)
    {
        rewrite(buff) << "_" << i;
        iniFile.ReadDouble("Сorrelator", buff.str(), &value);
        CorTime.push_back(value);
    }
    if(yAxisDLTS != nullptr)
        delete []yAxisDLTS;
    yAxisDLTS = new vector<double>[CorTime.size()]; /* Число осей совпадает с числом корреляторов */
        /* Настройки аппроксимации */
    iniFile.ReadDouble("Fitting", "abs_error", &AprErr);
    iniFile.ReadInt("Fitting", "max_iter", &AprIter);
    iniFile.ReadBool("Fitting", "use_for_relax", &AprEnableRelax);
    iniFile.ReadBool("Fitting", "use_for_dlts", &AprEnableDLTS);
        /* Настройки PID таблицы */
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
    /* Проверка корреляторов */
    for(const auto &t: CorTime)
        if(t*correlation_c > measure_time_DAQ) /* Все в мс */
            MessageBox(NULL, "You must change correlation settings.", "Note", MB_ICONINFORMATION);
}
