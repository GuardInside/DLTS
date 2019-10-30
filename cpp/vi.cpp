#include "vi.h"
using namespace std;

GENERATOR  Generator("Pulse generator Agilent");
THERMOSTAT Thermostat("Thermostat Lake Shore");

/* ****************** */
/* ����������� ������ */
/* ****************** */

VI::VI()
{
    viOpenDefaultRM(&hResMeneger);
    hInstrument = 0;
}

/* ������������� ��� �������� */
void VI::InitSession()
{
    #ifndef TEST_MODE
    string message = "An instrument " + ViName + " isn't initialized.";
    if(Init() != VI_SUCCESS)
        MessageBox(NULL, message.data(), "VI", 0);
    #endif // TEST_MODE
}

//������������� ������ �� ���������;
ViStatus VI::Init()
{
    stringstream buff;
    if(ID < 10) buff << "GPIB0::" << ID << "::INSTR";
    else buff << "GPIB::" << ID << "::INSTR";
    if(hInstrument) viClose(hInstrument);
    return viOpen(hResMeneger, (ViRsrc)buff.str().data(), VI_NULL, VI_NULL, &hInstrument);
}

/* �������� ��������� � ������� */
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

/* ��������� ���������-����� �� ������� */
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
/* ����������� ��������� */
/* ********************* */

void GENERATOR::Apply()
{
    static const double K = 2.3;
    stringstream Buff;
    rewrite(Buff) << ":OUTPUT" << channel << " OFF";
        Write(Buff);
    /* ��������� ���������. ����� � �� */
    rewrite(Buff) << ":PULSE:PER " << period*1000000 << "NS";
        Write(Buff);
    rewrite(Buff) << ":PULSE:WIDTH " << width*1000000 << "NS";
        Write(Buff);
    /* ���������, ������� �� ������������ */
    rewrite(Buff) << ":OUTP1:IMP 50OHM";
        Write(Buff);
    rewrite(Buff) << ":OUTP1:IMP:EXT 1000000OHM";
        Write(Buff);
    rewrite(Buff) << ":PULS:DOUB" << channel << " OFF";
        Write(Buff);
    rewrite(Buff) << ":PULS:TRIG1:VOLT TTL";
        Write(Buff);
    /* ��������� ���������� */
    rewrite(Buff) << ":HOLD VOLT";
        Write(Buff);
    if(index_mode == DLTS)
        rewrite(Buff) << ":VOLT1:HIGH " << (voltage_up)/(K) << "V";
    else if(index_mode == ITS)
        rewrite(Buff) << ":VOLT1:HIGH " << (begin_voltage)/(K) << "V";
    Write(Buff);
    rewrite(Buff) << ":HOLD VOLT";
        Write(Buff);
    rewrite(Buff) << ":VOLT1:LOW " << voltage_low/(K) << "V";
        Write(Buff);
    rewrite(Buff) << ":OUTPUT" << channel << " ON";
        Write(Buff);
}

/* ********************* */
/* ����������� ��������� */
/* ********************* */

void THERMOSTAT::Apply()
{
    /* ��������� ������ � ������ � */
    Write("CCHN A");
}

/* �������� ������������ ��������� ��������� ����������� */
bool THERMOSTAT::range_is_correct() const
{
    if(EndPoint >= SetPoint && TempStep >= 0)
        return true;
    else if(EndPoint <= SetPoint && TempStep <= 0)
        return true;
    return false;
}

/* */
/* */

/* ������������� �� ��������� ��� �������� ������� */
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