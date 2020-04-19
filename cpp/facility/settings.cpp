#include <facility.h>
#include "ini.h"
#include "vi.h"
#include "daq.h"

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
    iniFile.WriteInt("DAQ", "range", index_range.load());
    iniFile.WriteInt("DAQ", "averaging", averaging_DAQ);
    iniFile.WriteInt("DAQ", "rate", rate_DAQ);
    iniFile.WriteInt("DAQ", "time", measure_time_DAQ);
    iniFile.WriteDoubleFix("DAQ", "gate", gate_DAQ, TIME_PRECISION);
    /* Сохраняем настройки блока SULA */
    iniFile.WriteInt("SULA", "range_capacity", RANGE_SULA_index);
    iniFile.WriteInt("SULA", "pre_amp_gain", PRE_AMP_GAIN_SULA_index);
    /* Сохраняем настройки блока генератора импульсов */
    iniFile.WriteDoubleFix("Generator", "period", Generator.period, TIME_PRECISION);
    iniFile.WriteDoubleFix("Generator", "width", Generator.width, TIME_PRECISION);
    iniFile.WriteDoubleFix("Generator", "amplitude", Generator.amp, VOLTAGE_PRECISION);
    iniFile.WriteDoubleFix("Generator", "bias", Generator.bias, VOLTAGE_PRECISION);
    iniFile.WriteBool("Generator", "isActive", Generator.is_active.load());
    /* Сохраняем настройки блока общее */
    iniFile.WriteString("General", "save_path", FileSavePath);
    iniFile.WriteString("General", "file", FileSaveName);
    iniFile.WriteInt("General", "mode", (int)index_mode.load());
    /* Сохраняем настройки блока математической модели */
    iniFile.WriteDoubleSc("Model", "mass", dEfMass, 3);
    iniFile.WriteInt("Model", "g", (int)dFactorG);
    iniFile.WriteDoubleSc("Model", "Impurity", dImpurity, 3);
        /* Расширенне настройки */
    /* Расширенные настройки термостата */
    iniFile.WriteInt("Thermostat", "GPIB", Thermostat.gpib);
    /* Расширенные настройки генератора */
    iniFile.WriteInt("Generator", "GPIB", Generator.gpib);
    iniFile.WriteInt("Generator", "ch", Generator.curr_channel);
    /* Расширенные настройки DAQ */
    iniFile.WriteInt("DAQ", "Dev", id_DAQ);
    iniFile.WriteInt("DAQ", "trigger", pfi_ttl_port);
    iniFile.WriteInt("DAQ", "ai_relax", ai_port_measurement);
    iniFile.WriteInt("DAQ", "ai_pulse", ai_port_pulse);
    iniFile.WriteInt("DAQ", "ai_capacity", ai_port_capacity);
        /* Настройки корреляторов */
    /* Параметры корреляторов */
    iniFile.WriteDoubleFix("Сorrelator", "width", correlation_width, TIME_PRECISION);
    iniFile.WriteDoubleFix("Сorrelator", "c", correlation_c, 2);
    iniFile.WriteDoubleFix("Сorrelator", "alpha", correlation_alpha, 2);
    iniFile.WriteBool("Сorrelator", "Use_alpha", UseAlphaBoxCar);
    iniFile.WriteInt("Сorrelator", "type", WeightType);
    /* Количество корреляторов */
    iniFile.WriteInt("Сorrelator", "amount", CorTc.size());
    stringstream buff;
    for(size_t i = 0; i < CorTc.size(); i++)
    {
        rewrite(buff) << "_" << i;
        iniFile.WriteDoubleFix("Сorrelator", buff.str(), CorTc[i], TIME_PRECISION);
    }
        /* Настройки аппроксимации */
    iniFile.WriteInt("Fitting", "ncoeffs", bspline::ncoeffs);
    iniFile.WriteInt("Fitting", "order", bspline::order);
    //iniFile.WriteBool("Fitting", "use_for_relax", bspline::enable);
        /* Настройки PID таблицы */
    iniFile.Rename("pid.ini");
    for(int i = 0; i < QUANTITY_ZONE; i++)
    {
        rewrite(buff) << "ZONE " << i;
        iniFile.WriteInt(buff.str().data(), "TEMP", Thermostat.table[i].upper_boundary);
        iniFile.WriteInt(buff.str().data(), "P", Thermostat.table[i].P);
        iniFile.WriteInt(buff.str().data(), "I", Thermostat.table[i].I);
        iniFile.WriteInt(buff.str().data(), "D", Thermostat.table[i].D);
        iniFile.WriteInt(buff.str().data(), "RANGE", static_cast<int>(Thermostat.table[i].range));
    }
    CloseHandle((HANDLE)_beginthreadex(NULL, 0, dlg_success, NULL, 0, NULL));
}

void read_settings()
{
    bool flag;
    int  intbuff;
    ini::File iniFile{"settings.ini"};
        /* Пользовательские настройки */
    /* Загружаем настройки блока Термостата */
    iniFile.ReadDouble("Thermostat", "step", &Thermostat.TempStep);
    iniFile.ReadDouble("Thermostat", "disp", &Thermostat.TempDisp);
    /* Загружаем настройки блока Измерений */
    iniFile.ReadInt("DAQ", "range", &intbuff);
    index_range.store(intbuff);
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
    iniFile.ReadDouble("Generator", "amplitude", &Generator.amp);
    iniFile.ReadDouble("Generator", "bias", &Generator.bias);
    iniFile.ReadBool("Generator", "isActive", &flag);
    Generator.is_active.store(flag);
    /* Загружаем настройки блока общее */
    iniFile.ReadString("General", "save_path", &FileSavePath);
    iniFile.ReadString("General", "file", &FileSaveName);
    iniFile.ReadInt("General", "mode", &intbuff);
    index_mode.store(static_cast<mode> (intbuff) );
    /* Загружаем настройки блока математической модели */
    iniFile.ReadDouble("Model", "mass", &dEfMass);
    iniFile.ReadInt("Model", "g", (int*)(&dFactorG));
    iniFile.ReadDouble("Model", "Impurity", &dImpurity);
        /* Расширенне настройки */
    /* Расширенные настройки термостата */
    iniFile.ReadInt("Thermostat", "GPIB", &Thermostat.gpib);
    /* Расширенные настройки генератора */
    iniFile.ReadInt("Generator", "GPIB", &Generator.gpib);
    iniFile.ReadInt("Generator", "ch", (int*)&Generator.curr_channel);
    /* Расширенные настройки DAQ */
    iniFile.ReadInt("DAQ", "Dev", (int*)&id_DAQ);
    iniFile.ReadInt("DAQ", "trigger", (int*)&pfi_ttl_port);
    iniFile.ReadInt("DAQ", "ai_relax", (int*)&ai_port_measurement);
    iniFile.ReadInt("DAQ", "ai_pulse", (int*)&ai_port_pulse);
    iniFile.ReadInt("DAQ", "ai_capacity", (int*)&ai_port_capacity);
        /* Настройки корреляторов */
    /* Параметры корреляторов */
    iniFile.ReadDouble("Сorrelator", "width", &correlation_width);
    iniFile.ReadDouble("Сorrelator", "c", &correlation_c);
    iniFile.ReadDouble("Сorrelator", "alpha", &correlation_alpha);
    iniFile.ReadBool("Сorrelator", "Use_alpha", &UseAlphaBoxCar);
    iniFile.ReadInt("Сorrelator", "type", (int*)&WeightType);
    /* Количество корреляторов */
    int amount = 0;
    double value = 0.0;
    iniFile.ReadInt("Сorrelator", "amount", &amount);
    stringstream buff;
    CorTc.clear();
    for(size_t i = 0; i < (size_t)amount; i++)
    {
        rewrite(buff) << "_" << i;
        iniFile.ReadDouble("Сorrelator", buff.str(), &value);
        CorTc.push_back(value);
    }
    yAxisDLTS.clear();  /* Число осей совпадает с числом корреляторов */
    yAxisDLTS.resize(CorTc.size());
        /* Настройки аппроксимации */
    iniFile.ReadInt("Fitting", "ncoeffs", (int*)&bspline::ncoeffs);
    iniFile.ReadInt("Fitting", "order", (int*)&bspline::order);
    //iniFile.ReadBool("Fitting", "use_for_relax", &bspline::enable);
        /* Настройки PID таблицы */
    iniFile.Rename("pid.ini");
    for(int i = 0; i < QUANTITY_ZONE; i++)
    {
        rewrite(buff) << "ZONE " << i;
        iniFile.ReadInt(buff.str().data(), "TEMP", (int*)&Thermostat.table[i].upper_boundary);
        iniFile.ReadInt(buff.str().data(), "P", (int*)&Thermostat.table[i].P);
        iniFile.ReadInt(buff.str().data(), "I", (int*)&Thermostat.table[i].I);
        iniFile.ReadInt(buff.str().data(), "D", (int*)&Thermostat.table[i].D);
        iniFile.ReadInt(buff.str().data(), "RANGE", (int*)&Thermostat.table[i].range);
    }
}
//Инициализация сборщика данных, термостата и зон для PID регулировки
int ApplySettings()
{
    Thermostat.connect();
    Generator.connect();

    Generator.Apply();
    Thermostat.ApplyZones();
    bfDAQ0k.store(true);

    /* Проверка корреляторов */
    for(const auto &Tc: CorTc)
    {
        double Tg = Tc / correlation_c;
        if(0.001*gate_DAQ > Tg)
        {
            MessageBox(NULL, ("You must change correlation settings\n"
                       "Gate > Tg\nChange the Tc = " + to_string(Tc) + " [ms]").c_str(), "Note", MB_ICONINFORMATION);
            return -1;
        }
        if( Tc + Tg > measure_time_DAQ) /* Все в мс */
        {
            MessageBox(NULL, ("You must change correlation settings\n"
                       "Tc+Tg > measure time\nChange the Tc = " + to_string(Tc) + " [ms]").c_str(), "Note", MB_ICONINFORMATION);
            return -1;
        }
    }
    /* Формирем ось времени */
    double  gate    = 0.001 * gate_DAQ;
    double  delta   = 1000.0 / rate_DAQ;
    size_t  n       = measure_time_DAQ / delta; // Все в мс
    TimeAxis.clear();
    for(size_t i = 0; i < n; i++)
        TimeAxis.push_back(gate + i*delta);

    if(index_mode.load() == ITS)
    {
        /* Деактивируем возможность менять пункт меню Divide S on Tc */
        EnableMenuItem( GetSubMenu(GetMenu(hMainWindow), 1), ID_MENU_DIVIDE_S_ON_TC, MF_GRAYED );
    }
    else
    {
        EnableMenuItem( GetSubMenu(GetMenu(hMainWindow), 1), ID_MENU_DIVIDE_S_ON_TC, MF_ENABLED);
    }
    return 0;
}
