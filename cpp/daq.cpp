#include <algorithm>

#include "daq.h"
#include "variable.h"
#include "vi.h"

#define DAQmxErrChk(functionCall) { if( DAQmxFailed(error=(functionCall)) ) { goto Error; } }

VOID DAQmxClearState()
{
    DAQmxReadAnalog(0,0,0,0,0,0,0,0,NULL,true);
}

INT DAQmxReadAnalog(UINT uDev, UINT uAIPort, INT iTrigPort, float64 dRate, float64 dGate, INT iTrigEdge, UINT uRange, float64 uMesureTime, std::vector<double> *vData, BOOL bfResetError)
{
    static const float64 fTimeout = 10.0; //Seconds//
    /** Проверка и установка флага сбоя **/
    if(bfResetError == TRUE)
    {
        bfDAQ0k = true;
        return 1;
    }
    if(bfDAQ0k == false)
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
        if(!Generator.is_active)
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
        bfDAQ0k = false;
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
    vector<double>().swap(*vData);
    for(int32 i = 0; i < iRead; i++)
        vData->push_back(Data[i]);
    delete[] Data;
    return 0;
}


BOOL DAQMeasure_Capacity(UINT AIPort, double *capacity)
{
    vector<double> vResult;
    EnterCriticalSection(&csDataAcquisition);
    DAQmxReadAnalog(id_DAQ, AIPort, pfi_ttl_port,
                rate_DAQ, gate_DAQ*0.000001, DAQmx_Val_Rising, index_range, (Generator.period - Generator.width)*0.001,
                &vResult);
    LeaveCriticalSection(&csDataAcquisition);
    for(auto &it: vResult)
        *capacity += it;
    *capacity /= vResult.size();
    return TRUE;
}

BOOL MyDAQMeasure(vector<double> *vResult, UINT AverNum, double time, UINT AIPort, BOOL bfProgress)
{
    if(vResult == NULL || AverNum < 1) return FALSE;
    vector<double> vData;
    vResult->clear();
    for(UINT i = 0; i < AverNum; i++)
    {
        EnterCriticalSection(&csDataAcquisition);
        DAQmxReadAnalog(id_DAQ, AIPort, pfi_ttl_port,
                    rate_DAQ, gate_DAQ*0.000001, DAQmx_Val_Rising, index_range, time,
                    &vData);
        LeaveCriticalSection(&csDataAcquisition);
        if(i == 0) for(const auto &it: vData) vResult->push_back(it);
        else for(size_t i = 0; i < vData.size(); i++)
            vResult->at(i) += vData[i];
        if(bfProgress) SendMessage(hProgress, PBM_SETPOS, 100.0*i/AverNum, 0);
        vData.clear();
    }
    for(auto &it: *vResult)
        it = it/AverNum;
    if(bfProgress)
        SendMessage(hProgress, PBM_SETPOS, 0, 0);   //Очищаем Progress Bar
    return TRUE;
}

BOOL MeasurePulse(vector<double> *vData, double *dVoltBias, double *dVoltAmp)
{
    static CONST UINT T_NUM = 2, A_NUM = 1;
    *dVoltBias = 0.0;
    *dVoltAmp = 0.0;
    UINT uiCount1 = 0, uiCount2 = 0;
    vector<double> vData2;
    double dMeasTime = 0.0;
    if(Generator.is_active) dMeasTime = Generator.period*T_NUM*0.001;
    else dMeasTime = measure_time_DAQ*T_NUM*0.001;
    MyDAQMeasure(&vData2, A_NUM, dMeasTime, ai_port_pulse);
    /* Допустимое отклонение напряжения в Вольтах*/

    double dV = 0.05;
    /* Рассчитываем примерные значения амплитуды и смещения */
    double PredBias = 0.0, PredAmp = 0.0;
    if(Generator.is_active)
    {
        PredBias = Generator.bias;
        if(index_mode == DLTS)
            PredAmp = Generator.amplitude;
        else if(index_mode == ITS)
            PredAmp = Generator.begin_amplitude;
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
    if(vData != NULL) *vData = vData2;
    return TRUE;
}
