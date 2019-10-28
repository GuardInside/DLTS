#include <GPIB.h>
/* -1.0 to 0.0  => -0.02 to 2.3(3)
    -2.0 to 0.0 => +0.00 to 4.6(3)
    -2.5 to 0.0 => -0.01 to 5.7(7)
    -3.0 to 0.0 => -0.01 to 6.9(5)

    -1.0 to 1.0 => -2.3 to 2.3
    -2.0 to 2.0 => -4.6 to +4.7
    K = 2.3;
*/

/* ������-��������� */
GENERATOR Generator;

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

/* �������� ��������� �������� */
void GENERATOR::mesure_pulse()
{
    //const int FREQUENCY = 100000;
    //std::vector<double> yAxis;
    //DAQPulseVoltageInst.InitSession(id_DAQ, 0, FREQUENCY, period, true, 0, DAQmx_Val_Falling);
    //int32 read = 0; /* ������� ����� �� ����� */
    //DAQPulseVoltageInst.ReadAnalog(yAxis, &read);
    /* ������������ �������� �������� �������� */
    //double N = FREQUENCY/1000.0*period, /* ����� ������� */
        //SSW = FREQUENCY/1000.0*width/4; /* �������� �������� � �������*/
    /* ��������� �� ����� �� �������� */
    //for(int i = 0; i < N/2-SSW; i++)
        //voltage_up_measured += yAxis[i];
    /* ��������� �� ����� ����� �������� */
    //for(int i = N/2+4*SSW+SSW; i < N; i++)
        //voltage_up_measured += yAxis[i];
    //voltage_up_measured /= N/2-SSW + (N - (N/2+4*SSW+SSW));
    /* ��������� �� ����� ��������������� �������� */
    //for(int i = N/2+SSW; i < N/2+3*SSW; i++)
        //voltage_low_measured += yAxis[i];
    //voltage_low_measured /= 2*SSW;
}
