#ifndef DAQ_H_INCLUDED
#define DAQ_H_INCLUDED
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include "NIDAQmx.h"

using std::vector;

/** Времена в секуднах **/
INT  DAQmxReadAnalog(UINT uDev, UINT uAIPort, INT iTrigPort, float64 dRate, float64 dGate, INT iTrigEdge, UINT uRange, float64 uMesureTime, std::vector<double> *vData, BOOL bfResetError = FALSE);
VOID DAQmxClearState();
/** Специализированы под эту программу **/
BOOL DAQMeasure_Capacity(UINT AIPort, double *capacity);
BOOL MyDAQMeasure(vector<double> *vResult, UINT AverNum, double time, UINT AIPort, BOOL bfProgress = FALSE);
BOOL MeasurePulse(vector<double> *vData, double *dVoltBias, double *dVoltAmp);
#endif // DAQ_H_INCLUDED
