#include "vi.h"
#include "dlts_math.h"
#include <cmath>
#include <stdexcept>
#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>

using std::setprecision;

vi_generator  Generator;
vi_thermostat Thermostat;

/* ****************** */
/* ����������� ������ */
/* ****************** */

vi::vi(): gpib{0}, bfvi0k{true}, hResMeneger{0}, hInstrument{0}
{
    viOpenDefaultRM(&hResMeneger);
}
void vi::connect()
{
    stringstream buff;
    buff << "GPIB" << (gpib < 10 ? "0": "") << "::" << gpib << "::INSTR";

    viClose(hInstrument);
    ViStatus state = viOpen(hResMeneger, (ViRsrc)buff.str().data(), VI_NULL, VI_NULL, &hInstrument);
    if(state == VI_SUCCESS)
        bfvi0k.store(true);
    else bfvi0k.store(false);
//bfvi0k.store(true);
    #ifndef TEST_MODE
    if(bfvi0k.load() == false)
    {
        string message = "An instrument " + ViName + " isn't initialized.";
        MessageBox(NULL, message.data(), "vi", 0);
    }
    #endif // TEST_MODE
}
ViStatus vi::Write(stringstream &message, ViUInt32* rcount)
{
    ViStatus status = viWrite(hInstrument, (ViBuf)message.str().data(), (ViUInt32)message.str().length(), rcount);
    message.str("");
    return status;
}
ViStatus vi::Write(string const &message, ViUInt32* rcount)
{
    return viWrite(hInstrument, (ViBuf)message.data(), (ViUInt32)message.length(), rcount);
}
void vi::ReadStr(string &strResult)
{
    ZeroMemory(ViBuffer, READ_BUFFER_SIZE);
    viRead(hInstrument, ViBuffer, READ_BUFFER_SIZE-1, nullptr);
    strResult = (string)(char*)ViBuffer;
}
void vi::ReadDigit(double& digit)
{
    ViUInt32 read = 0;
    viRead(hInstrument, ViBuffer, READ_BUFFER_SIZE-1, &read);
    ViBuffer[read] = '\0';
    digit = atof((char*)ViBuffer);
}
void vi::ReadDigit(int& digit)
{
    ViUInt32 read = 0;
    viRead(hInstrument, ViBuffer, READ_BUFFER_SIZE-1, &read);
    ViBuffer[read] = '\0';
    digit = atoi((char*)ViBuffer);
}

/* ********************* */
/* ����������� ��������� */
/* ********************* */
vi_generator::vi_generator(): vi{}, is_active{true}, curr_channel{0}, amp{0.0}, bias{0.0} {}
void vi_generator::Reset()
{
    ViName = "Agilent";
    Write("*RST");
    Impedance();
    DoublePulses(switcher::off);
    TTL();
}
void vi_generator::Impedance()
{
    stringstream Buff;
    Buff << ":OUTP" << curr_channel << ":IMP 50OHM";
        Write(Buff);
    Buff << ":OUTP" << curr_channel << ":IMP:EXT 1000000OHM";
        Write(Buff);
}
void vi_generator::TTL()
{
    Write(":PULS:TRIG1:VOLT TTL");
}
void vi_generator::DoublePulses(switcher state)
{
    stringstream Buff;
    if(state == off)
        Buff << ":PULS:DOUB" << curr_channel << " OFF";
    else
        Buff << ":PULS:DOUB" << curr_channel << " ON";
    Write(Buff);
}
void vi_generator::ErrorCheck(switcher state)
{
    if(state == off)
        Write(":SYST:CHEC OFF");
    else if(state == on)
        Write(":SYST:CHEC ON");
}
void vi_generator::Channel(switcher state)
{
    stringstream Buff;
    if(state == off)
        Buff << ":OUTPUT" << curr_channel << " OFF";
    else
        Buff << ":OUTPUT" << curr_channel << " ON";
    Write(Buff);
}
void vi_generator::Pulses(switcher state)
{
    Write(":HOLD VOLT");
    stringstream Buff;
    if(state == off)
    {
        Buff << ":VOLT" << curr_channel << ":HIGH " << -1.0*bias/(K) << "V";
        Write(Buff);
        Buff << ":VOLT" << curr_channel << ":LOW " << -1.0*bias/(K) << "V";
        Write(Buff);
    }
    else
    {
        Buff << ":VOLT" << curr_channel << ":HIGH " << -1.0*round((amp)/(K), 3) << "V";
        Write(Buff);
        Buff << ":VOLT" << curr_channel << ":LOW " << -1.0*bias/(K) << "V";
        Write(Buff);
    }
}
void vi_generator::Period(double period)
{
    stringstream Buff;
    Buff << ":PULSE:PER " << period*1000000 << "NS";
    Write(Buff);
}
void vi_generator::Width(double width)
{
    stringstream Buff;
    Buff << ":PULSE:WIDTH " << width*1000000 << "NS";
    Write(Buff);
}
void vi_generator::Amp(double _amp)
{
    amp = _amp;
    stringstream Buff;
    Buff << ":VOLT" << curr_channel << ":HIGH " << -1.0*round((_amp)/(K), 3) << "V";
    Write(Buff);
}
void vi_generator::Bias(double _bias)
{
    bias = _bias;
    stringstream Buff;
    Buff << ":VOLT" << curr_channel << ":LOW " << -1.0*(_bias)/(K) << "V";
    Write(Buff);
}
void vi_generator::Apply()
{
    if(bfvi0k == false || is_active == false) return;
    ErrorCheck(vi::switcher::off);
        Pulses(vi::switcher::on);
        Period(period);
        Width(width);
    Channel(vi::switcher::on);
}

/* ********************* */
/* ����������� ��������� */
/* ********************* */

vi_thermostat::vi_thermostat(): vi{},
    BeginPoint{0.0}, EndPoint{0.0}, TempStep{0.0}, TempDisp{0.0}
{
    ViName = "LakeShore";
    Write("CCHN A");
    Write("TUNE 4"); /* PID, 0 = Manual, 1 = P, 2 = PI, 3 = PID, 4 = Zone */
}
heatlvl vi_thermostat::curr_heatlvl()
{
    double Temp = 0.0;
    Write("CDAT?");
    ReadDigit(Temp);
    for(int i = 0; i < QUANTITY_ZONE; i++)
        if(Temp <= table[i].upper_boundary)
            return table[i].range;
    throw std::runtime_error("Current temperature isn't in zone table range.");
}

/* �������� ������������ ��������� ��������� ����������� */
bool vi_thermostat::range_is_correct()
{
    if(EndPoint >= BeginPoint && TempStep >= 0)
        return true;
    else if(EndPoint <= BeginPoint && TempStep <= 0)
        return true;
    return false;
}
void vi_thermostat::ApplyZones()
{
    stringstream buff;
    buff << setprecision(2);
    /* ��������� ��������� ��� */
    for(int i = 0; i < QUANTITY_ZONE; i++)
    {
        buff << "ZONE " << i+1 << ",+" << table[i].upper_boundary << ","
                      << static_cast<int>(table[i].range) << "," << table[i].P << ","
                      << table[i].I << "," << table[i].D;
        Write(buff);
    }
}
void vi_thermostat::SetPoint(double T)
{
    stringstream buff;
    buff << "SETP " << T << "K";
    Write(buff);
}
void vi_thermostat::CurrentTemp(double* T)
{
    Write("CDAT?");
    ReadDigit(*T);
}
void vi_thermostat::CurrentHeatPercent(double* percent)
{
    Write("HEAT?");
    ReadDigit(*percent);
}
void vi_thermostat::CurrentSetPoint(double* SetPoint)
{
    Write("SETP?");
    ReadDigit(*SetPoint);
}

void vi_thermostat::SwitchHeater(switcher state)
{
    stringstream buff;
    if(state == off)
        Write("RANG 0");
    else
    {
        buff << "RANG " << static_cast<int>(curr_heatlvl());
        Write(buff);
    }
}
