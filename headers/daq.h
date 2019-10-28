#ifndef DAQ_H_INCLUDED
#define DAQ_H_INCLUDED
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include "NIDAQmx.h"

/** Времена в секуднах **/
INT  DAQmxReadAnalog(UINT uDev, UINT uAIPort, INT iTrigPort, float64 dRate, float64 dGate, INT iTrigEdge, UINT uRange, float64 uMesureTime, std::vector<double> *vData, BOOL bfResetError = FALSE);
VOID DAQmxClearState();

#endif // DAQ_H_INCLUDED
