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
        if(!Generator.is_active) iTrigEdge = DAQmx_Val_Falling;
        else iTrigEdge = DAQmx_Val_Rising;
        DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(hTask, buff.str().data(), iTrigEdge));
        DAQmxErrChk(DAQmxSetStartTrigDelayUnits(hTask, DAQmx_Val_Seconds));
        DAQmxErrChk(DAQmxSetStartTrigDelay(hTask, dGate + Generator.width*0.001));
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

BOOL MeasurePulse(vector<double> *vData, double *dMinVoltage, double *dMaxVoltage)
{
    static CONST UINT T_NUM = 2, A_NUM = 1;
    *dMinVoltage = 0.0;
    *dMaxVoltage = 0.0;
    vector<double> vData2;
    MyDAQMeasure(&vData2, A_NUM, Generator.period*T_NUM*0.001, ai_port_pulse);
    /* Рассчитываем истинные значения амплитуд */
    double N = rate_DAQ/1000.0*Generator.period*T_NUM, /* Число сэмплов */
        SSW = rate_DAQ/1000.0*Generator.width; /* Ширина импульса в сэмплах*/
    /* Пробегаем по точка до импульса */
    for(int i = 0; i < N/2-2*SSW; i++)
        *dMaxVoltage += vData2[i];
    /* Пробегаем по точкам после импульса */
    for(int i = N/2+SSW; i < N-2*SSW; i++)
        *dMaxVoltage += vData2[i];
    *dMaxVoltage /= (N/2-2*SSW) + ((N-2*SSW)-(N/2+SSW));
    /* Пробегаем по точка соответствующим импульсу */
    for(int i = N/2-SSW+SSW/4; i < N/2-SSW/4; i++)
        *dMinVoltage += vData2[i];
    *dMinVoltage /= N/2-SSW/4 - (N/2-SSW+SSW/4);
    if(vData != NULL) *vData = vData2;
    return TRUE;
}
