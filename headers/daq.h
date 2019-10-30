#ifndef DAQ_H_INCLUDED
#define DAQ_H_INCLUDED
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include "NIDAQmx.h"

/** ������� � �������� **/
INT  DAQmxReadAnalog(UINT uDev, UINT uAIPort, INT iTrigPort, float64 dRate, float64 dGate, INT iTrigEdge, UINT uRange, float64 uMesureTime, std::vector<double> *vData, BOOL bfResetError = FALSE);
VOID DAQmxClearState();
/** ���������������� ��� ��� ��������� **/
BOOL MyDAQMeasure(std::vector<double> *vResult, UINT AverNum, double time, UINT AIPort, BOOL bfProgress = FALSE);
BOOL MeasurePulse(std::vector<double> *vData, double *dMinVoltage, double *dMaxVoltage);
#endif // DAQ_H_INCLUDED
