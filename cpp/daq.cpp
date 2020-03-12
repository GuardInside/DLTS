#include <algorithm>

#include "daq.h"
#include "vi.h"
#include "variable.h"

using std::stringstream;

#define PBM_SETPOS (WM_USER+2)

#define DAQmxErrChk(functionCall) { if( DAQmxFailed(error=(functionCall)) ) { goto Error; } }

atomic_bool         bfDAQ0k{true};
CRITICAL_SECTION    csDataAcquisition;

INT ReadAnalogSignal(UINT uDev, UINT uAIPort, INT iTrigPort, float64 dRate, float64 dGate, INT iTrigEdge, UINT uRange, float64 uMesureTime, vector<double> *vData)
{
    static const float64 fTimeout = 10.0; //Seconds//
    if(bfDAQ0k.load() == false)
        return -1;
    /** Подготовка */
    TaskHandle hTask = 0;
    int32 error = 0;
    UINT32 uSamples = dRate*uMesureTime;
    float64* Data = new float64[uSamples];
    int32 iRead = 0;
    stringstream buff;
    buff << "/Dev" << uDev << "/ai" << uAIPort;
    float64 dVoltRange = 10.0;
    DAQmxErrChk(DAQmxCreateTask("DAQmxTask", &hTask));
    switch(uRange)
    {
        case 0: dVoltRange = 10.0;    break;
        case 1: dVoltRange = 5.0;     break;
        case 2: dVoltRange = 0.5;   break;
        case 3: dVoltRange = 0.05;  break;
    }
    DAQmxErrChk(DAQmxCreateAIVoltageChan(hTask, buff.str().data(), "DAQmxAIVoltageChan", DAQmx_Val_RSE, -dVoltRange, dVoltRange, DAQmx_Val_Volts, NULL));
    DAQmxErrChk(DAQmxCfgSampClkTiming(hTask, "", dRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, uSamples));
    if(iTrigPort > -1)
    {
        /** Параметры триггера **/
        buff.str("");
        buff << "/Dev" << uDev << "/PFI" << iTrigPort;
        DAQmxErrChk(DAQmxSetStartTrigDelayUnits(hTask, DAQmx_Val_Seconds));
        if(Generator.bfvi0k.load() == false)
        {
            iTrigEdge = DAQmx_Val_Falling;
            DAQmxErrChk(DAQmxSetStartTrigDelay(hTask, dGate));
        }
        else
        {
            iTrigEdge = DAQmx_Val_Rising;
            DAQmxErrChk(DAQmxSetStartTrigDelay(hTask, dGate + Generator.width*0.001));
        }
        DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(hTask, buff.str().data(), iTrigEdge));
    }
    DAQmxErrChk(DAQmxStartTask(hTask));
    DAQmxErrChk(DAQmxReadAnalogF64(hTask, DAQmx_Val_Auto, fTimeout, DAQmx_Val_GroupByChannel , Data, uSamples, &iRead, NULL));
    Error:
    if(hTask != 0)
    {
       DAQmxStopTask (hTask);
       DAQmxClearTask (hTask);
    }
    if(DAQmxFailed(error))
    {
        /** Что-то пошло не по плану **/
        bfDAQ0k.store(false);
        INT32 iBufferSize = DAQmxGetExtendedErrorInfo(NULL, 0);
        char *cstrBuffer = new char[iBufferSize];
        DAQmxGetExtendedErrorInfo(cstrBuffer, iBufferSize);
        #ifndef TEST_MODE
        MessageBox(0, cstrBuffer, "DAQmx", 0);
        #endif // TEST_MODE
        delete[] cstrBuffer;
        delete[] Data;
        return -1;
    }
    /** Все идет по плану **/
    vData->clear();
    vData->reserve( static_cast<size_t>(iRead) );
    for(int32 i = 0; i < iRead; i++)
        vData->push_back(Data[i]);
    delete[] Data;
    return 0;
}

void CapacityMeasuring(UINT AIPort, double *capacity)
{
    vector<double> vResult;
    vResult.reserve(rate_DAQ*(Generator.period - Generator.width)*0.001);
    EnterCriticalSection(&csDataAcquisition);
    Generator.Pulses(vi::switcher::off);
    Sleep(10 * measure_time_DAQ);
    ReadAnalogSignal(id_DAQ, AIPort, pfi_ttl_port,
                rate_DAQ, gate_DAQ*0.000001, DAQmx_Val_Rising, index_range.load(), (Generator.period - Generator.width)*0.001,
                &vResult);
    Generator.Pulses(vi::switcher::on);
    LeaveCriticalSection(&csDataAcquisition);
    *capacity = accumulate(vResult.begin(), vResult.end(), static_cast<double>(0.0));
    *capacity /= vResult.size();
}

void Measuring(vector<double> *vResult, UINT AverNum, double time, UINT AIPort,  size_t range_index, BOOL bfProgress)
{
    if(vResult == NULL || AverNum < 1) return;
    vector<double> vData;
    vData.reserve(rate_DAQ*time);
    vResult->clear();
    vData.reserve(rate_DAQ*time);
    for(UINT i = 0; i < AverNum; i++)
    {
        EnterCriticalSection(&csDataAcquisition);
        ReadAnalogSignal(id_DAQ, AIPort, pfi_ttl_port,
                    rate_DAQ, gate_DAQ*0.000001, DAQmx_Val_Rising, range_index, time,
                    &vData);
        LeaveCriticalSection(&csDataAcquisition);
        if(i == 0)
            std::copy(vData.cbegin(), vData.cend(), vResult->begin());//for(const auto &it: vData) vResult->push_back(it);
        else
            std::transform(vData.cbegin(), vData.cend(), vResult->begin(), vResult->begin(), std::plus<double>());
        //for(size_t i = 0; i < vData.size(); i++)
                //vResult[i] += vData[i];
        if(bfProgress) SendMessage(hProgress, PBM_SETPOS, 100.0*i/AverNum, 0);
        vData.clear();
    }
    for(auto &it: *vResult)
        it = it/AverNum;
    if(bfProgress) SendMessage(hProgress, PBM_SETPOS, 0, 0);
}

void PulsesMeasuring(vector<double> *vData, double *dVoltBias, double *dVoltAmp)
{
    static CONST UINT T_NUM = 2, A_NUM = 1;
    *dVoltBias = 0.0;
    *dVoltAmp = 0.0;
    UINT uiCount1 = 0, uiCount2 = 0;
    double dMeasTime = 0.0;
    if(Generator.bfvi0k.load()) dMeasTime = Generator.period*T_NUM*0.001;
    else dMeasTime = measure_time_DAQ*T_NUM*0.001;
    size_t range_index = 0;
    vector<double> vData2;
    vData2.reserve(rate_DAQ*dMeasTime);
    Measuring(&vData2, A_NUM, dMeasTime, ai_port_pulse, range_index);
    /* Допустимое отклонение напряжения в Вольтах*/
    double dV = 0.05;
    /* Рассчитываем примерные значения амплитуды и смещения */
    double PredBias = 0.0, PredAmp = 0.0;
    if(Generator.bfvi0k.load())
    {
        PredBias = Generator.bias;
        if(index_mode.load() == DLTS)
            PredAmp = Generator.amp;
        else if(index_mode.load() == AITS)
            PredAmp = Generator.begin_amp;
    }
    else
    {
        /* Bias всегда больше либо равен Amp */
        auto AmpBias = minmax_element(vData2.begin(), vData2.end());
        PredAmp = *AmpBias.first;
        PredBias = *AmpBias.second;
    }
    /* Рассчитываем истинные значения амплитуды и смещения */
    for(const auto &v: vData2)
    {
        if( ( v >= (PredBias-dV) ) && ( v <= (PredBias+dV) ) )
        {
            *dVoltBias += v;
            uiCount2++;
        }
        else if( ( v >= (PredAmp-dV) ) && ( v <= (PredAmp+dV) ) )
        {
            *dVoltAmp += v;
            uiCount1++;
        }
    }
    if(uiCount1 != 0)
        *dVoltAmp /= uiCount1;
    else *dVoltAmp = 0.0;
    if(uiCount2 != 0)
        *dVoltBias /= uiCount2;
    else *dVoltBias = 0.0;
    if(std::abs(PredBias) > 10.0)
        *dVoltBias = PredBias;
    if(std::abs(PredAmp) > 10.0)
        *dVoltAmp = PredAmp;
    if(vData != NULL) *vData = vData2;
}
