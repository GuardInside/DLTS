#include "vi.h"
#include <cmath>
using namespace std;

GENERATOR  Generator("Pulse generator Agilent");
THERMOSTAT Thermostat("Thermostat Lake Shore");

/* ****************** */
/* Виртуальный прибор */
/* ****************** */

VI::VI()
{
    viOpenDefaultRM(&hResMeneger);
    hInstrument = 0;
}

/* Инициализация при создании */
void VI::InitSession()
{
    #ifndef TEST_MODE
    string message = "An instrument " + ViName + " isn't initialized.";
    if(Init() != VI_SUCCESS)
        MessageBox(NULL, message.data(), "VI", 0);
    #endif // TEST_MODE
}

//Инициализация сессии по умолчанию;
ViStatus VI::Init()
{
    stringstream buff;
    if(ID < 10) buff << "GPIB0::" << ID << "::INSTR";
    else buff << "GPIB::" << ID << "::INSTR";
    if(hInstrument) viClose(hInstrument);
    return viOpen(hResMeneger, (ViRsrc)buff.str().data(), VI_NULL, VI_NULL, &hInstrument);
}

/* Записать сообщение в очередь */
ViStatus VI::Write(const stringstream& mes, ViUInt32*  rcount)
{
    return viWrite(hInstrument, (ViBuf)mes.str().data(), (ViUInt32)mes.str().length(), rcount);
}
ViStatus VI::Write(const string mes, ViUInt32*  rcount)
{
    return viWrite(hInstrument, (ViBuf)mes.data(), (ViUInt32)mes.length(), rcount);
}
void VI::Read(string& strResult)
{
    ZeroMemory(ViBuffer, READ_BUFFER_SIZE);
    viRead(hInstrument, ViBuffer, READ_BUFFER_SIZE-1, &read);
    strResult = (string)(char*)ViBuffer;
}

/* Прочитать сообщение-число из очереди */
void VI::ReadDigit(double& digit)
{
    viRead(hInstrument, ViBuffer, READ_BUFFER_SIZE-1, &read);
    ViBuffer[read] = '\0';
    digit = atof((char*)ViBuffer);
}

void VI::ReadDigit(int& digit)
{
    viRead(hInstrument, ViBuffer, READ_BUFFER_SIZE-1, &read);
    ViBuffer[read] = '\0';
    digit = atoi((char*)ViBuffer);
}

/* ********************* */
/* Виртуальный генератор */
/* ********************* */
void GENERATOR::Pulses(SWITCHER state)
{
    static const double K = 2.3;
    Write(":HOLD VOLT");
    stringstream Buff;
    if(state == OFF)
    {
        rewrite(Buff) << ":VOLT" << channel << ":HIGH " << 0.0 << "V";
        Write(Buff);
        rewrite(Buff) << ":VOLT" << channel << ":LOW " << 0.0 << "V";
        Write(Buff);
    }
    else if(state == ON)
    {
        if(index_mode == DLTS)
            rewrite(Buff) << ":VOLT" << channel << ":HIGH " << -1.0*round((amplitude)/(K), 3) << "V";
        else if(index_mode == ITS)
            rewrite(Buff) << ":VOLT" << channel << ":HIGH " << -1.0*round((begin_amplitude)/(K), 3) << "V";
        Write(Buff);
        rewrite(Buff) << ":VOLT" << channel << ":LOW " << -1.0*bias/(K) << "V";
            Write(Buff);
    }
}

void GENERATOR::Reset()
{
    Write("*RST");
}

void GENERATOR::ErrorCheck(SWITCHER state)
{
    if(state == OFF)
        Write(":SYST:CHEC OFF");
    else if(state == ON)
        Write(":SYST:CHEC ON");
}

void GENERATOR::Channel(SWITCHER state)
{
    stringstream Buff;
    if(state == OFF)
        rewrite(Buff) << ":OUTPUT" << channel << " OFF";
    else if(state == ON)
        rewrite(Buff) << ":OUTPUT" << channel << " ON";
    Write(Buff);
}

void GENERATOR::Apply()
{
    if(is_active == false)
        return;
    stringstream Buff;
    Reset();
    ErrorCheck(OFF);
    Channel(OFF);
    /* Временные настройки. Время в мс */
    rewrite(Buff) << ":PULSE:PER " << period*1000000 << "NS";
        Write(Buff);
    rewrite(Buff) << ":PULSE:WIDTH " << width*1000000 << "NS";
        Write(Buff);
    /* Настройки, скрытые от пользователя */
    rewrite(Buff) << ":OUTP" << channel << ":IMP 50OHM";
        Write(Buff);
    rewrite(Buff) << ":OUTP" << channel << ":IMP:EXT 1000000OHM";
        Write(Buff);
    rewrite(Buff) << ":PULS:DOUB" << channel << " OFF";
        Write(Buff);
    Write(":PULS:TRIG1:VOLT TTL");
    /* Конец скрытых настроек */
    Pulses(ON);
    Channel(ON);
}

/* ********************* */
/* Виртуальный термостат */
/* ********************* */

void THERMOSTAT::Apply()
{
    /* Считываем данные с канала А */
    Write("CCHN A");
}

/* Проверка корректности диапазона измерения температуры */
bool THERMOSTAT::range_is_correct() const
{
    if(EndPoint >= BeginPoint && TempStep >= 0)
        return true;
    else if(EndPoint <= BeginPoint && TempStep <= 0)
        return true;
    return false;
}

/* */
/* */

/* Инициализация по умолчанию при создании таблицы */
THERMOSTAT::PIDTABLE::PIDTABLE()
{
    for(int i = 0; i < QUANTITY_ZONE; i++)
    {
        P[i] = 50; I[i] = 20; D[i] = 0;
        range[i] = 0;
        upper_boundary[i] = 0;
    }
}

UINT THERMOSTAT::PIDTABLE::GetActuallyHeatRange()
{
    double Temperature = 0.0;
    Thermostat.Write("CDAT?");
    Thermostat.ReadDigit(Temperature);
    for(int i = 0; i < QUANTITY_ZONE; i++)
        if(Temperature <= upper_boundary[i])
            return range[i];
    MessageBox(0, "Invalid GetActuallyHeatRange.\n", "Error", MB_ICONERROR);
    return 1;
}

int THERMOSTAT::PIDTABLE::GetIDRes(const int i, const string str)
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
