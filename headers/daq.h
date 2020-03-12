#ifndef DAQ_H_INCLUDED
#define DAQ_H_INCLUDED
#include <windows.h>
#include <atomic>
#include <string>
#include <sstream>
#include <vector>
#include "NIDAQmx.h"

using std::vector;  using std::atomic_bool;

/** Времена в секуднах **/
extern CRITICAL_SECTION csDataAcquisition;
extern atomic_bool      bfDAQ0k;

void CapacityMeasuring(UINT AIPort, double *capacity);
void Measuring(vector<double> *vResult, UINT AverNum, double time, UINT AIPort, size_t range_index, BOOL bfProgress = FALSE);
void PulsesMeasuring(vector<double> *vData, double *dVoltBias, double *dVoltAmp);
#endif // DAQ_H_INCLUDED
