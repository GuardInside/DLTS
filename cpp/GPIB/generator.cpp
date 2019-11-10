#include <GPIB.h>

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
    vector<double> vData;
    EnterCriticalSection(&csDataAcquisition);
    DAQmxReadAnalog(id_DAQ, ai_port_pulse, pfi_ttl_port,
                rate_DAQ, gate_DAQ*0.000001, DAQmx_Val_Rising, index_range, measure_time_DAQ*0.001,
                &vData);
    LeaveCriticalSection(&csDataAcquisition);
    // ������������ �������� �������� ��������
    /*double High = 0.0, Low = 0.0;
    double N = rate_DAQ/1000.0*Generator.period, // ����� �������
        SSW = rate_DAQ/1000.0*Generator.width/4; // �������� �������� � �������
    // ��������� �� ����� �� ��������
    for(int i = 0; i < N/2-SSW; i++)
        High += vData[i];
    // ��������� �� ����� ����� ��������
    for(int i = N/2+4*SSW+SSW; i < N; i++)
        High += vData[i];
    High /= N/2-SSW + (N - (N/2+4*SSW+SSW));
    // ��������� �� ����� ��������������� ��������
    for(int i = N/2+SSW; i < N/2+3*SSW; i++)
        Low += vData[i];
    Low /= 2*SSW;
    voltage_low_measured = Low;
    voltage_up_measured = High;*/
}
