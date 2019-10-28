#include "daq.h"
#include "variable.h"

VOID DAQmxClearState()
{
    DAQmxReadAnalog(0,0,0,0,0,0,0,0,NULL,true);
}

INT DAQmxReadAnalog(UINT uDev, UINT uAIPort, INT iTrigPort, float64 dRate, float64 dGate, INT iTrigEdge, UINT uRange, float64 uMesureTime, std::vector<double> *vData, BOOL bfResetError)
{
    static const float64 fTimeout = 10.0; //Seconds//
    static BOOL fbError = FALSE;
    /** Проверка и установка флага сбоя **/
    if(bfResetError == TRUE)
    {
        fbError = FALSE;
        return 1;
    }
    if(fbError == TRUE)
    {
        return -1;
    }
    /** Подготовка */
    TaskHandle hTask;
    UINT32 uSamples = dRate*uMesureTime;
    float64* Data = new float64[uSamples];
    int32 iRead = 0;
    stringstream buff;
    DAQmxCreateTask("ReadAnalog64Task", &hTask);
    buff << "/Dev" << uDev << "/ai" << uAIPort;
    float64 dVoltRange = 10.0;
    switch(uRange)
    {
        case 0: dVoltRange = 10.0;    break;
        case 1: dVoltRange = 5.0;     break;
        case 2: dVoltRange = 0.5;   break;
        case 3: dVoltRange = 0.05;  break;
    }
    DAQmxCreateAIVoltageChan(hTask, buff.str().data(), "ReadAnalog64Channel", DAQmx_Val_RSE, -dVoltRange, dVoltRange, DAQmx_Val_Volts, NULL);
    DAQmxCfgSampClkTiming(hTask, "", dRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, uSamples);
    if(iTrigPort > -1)
    {
        /** Параметры триггера **/
        buff.str("");
        buff << "/Dev" << uDev << "/PFI" << iTrigPort;
        DAQmxCfgDigEdgeStartTrig(hTask, buff.str().data(), iTrigEdge);
        if(dGate != 0.0)
        {
            DAQmxSetStartTrigDelayUnits(hTask, DAQmx_Val_Seconds);
            DAQmxSetStartTrigDelay(hTask, dGate);
        }
    }
    if(DAQmxFailed(DAQmxReadAnalogF64(hTask, DAQmx_Val_Auto, fTimeout, DAQmx_Val_GroupByChannel , Data, uSamples, &iRead, NULL)))
    {
        /** Что-то пошло не по плану **/
        fbError = true;
        INT32 iBufferSize = DAQmxGetExtendedErrorInfo(NULL, 0);
        char *cstrBuffer = new char[iBufferSize];
        DAQmxGetExtendedErrorInfo(cstrBuffer, iBufferSize);
        MessageBox(0, cstrBuffer, "NIDAQ", 0);
        DAQmxClearTask(hTask);
        delete[]cstrBuffer;
        delete[] Data;
        return -1;
    }
    /** Все идет по плану **/
    vector<double>().swap(*vData);
    for(int32 i = 0; i < iRead; i++)
        vData->push_back(Data[i]);
    DAQmxClearTask(hTask);
    delete[] Data;
    return 0;
}


