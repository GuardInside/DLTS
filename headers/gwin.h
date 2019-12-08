#ifndef GWIN_H
#define GWIN_H

#include <windows.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <cmath>

//#define GWIN_FLAG_CROSS  0x100

namespace gwin{

typedef std::vector<double> gVector;
typedef std::vector<std::vector<double>> gMulVector;
typedef std::vector<std::string> gInfoVector;
struct gPoint{double x; double y;};

HWND gCreateWindow(HINSTANCE hInst, HWND hWndParent, DWORD style);
BOOL gClose(HWND hWnd);
BOOL gMulData(HWND hWnd, const gVector *vData1, const gMulVector *vMulData);
BOOL gData(HWND hWnd, const gVector *vData1, const gVector *vData2);
BOOL gPosition(HWND hWnd, INT x, INT y);
BOOL gSize(HWND hWnd, INT x, INT y);
BOOL gFont(HWND hWnd, int iHeight, int iWidth, int iAngle, int iWeight, std::string strFontName = "");
BOOL gAdditionalInfo(HWND hWnd, std::string);
BOOL gMark(HWND hWnd, int iMark1, int iMark2, int iAdMark1, int iAdMark2);
BOOL gPrecision(HWND hWnd, INT iPrec1, INT iPrec2);
BOOL gBand(HWND hWnd, double dMinX, double dMaxX, double dMinY, double dMaxY);
BOOL gDefaultPlot(HWND hWnd, std::string message);
BOOL gCross(HWND hWnd, const gVector *vData1, const gVector *vData2);

BOOL gBandGet(HWND hWnd, double* dMinX, double* dMaxX, double* dMinY, double* dMaxY);

BOOL gDvToLp(HWND hWnd, gPoint *pt);
BOOL gLpToGp(HWND hWnd, gPoint *pt); //Логические координаты
BOOL gGpToLp(HWND hWnd, gPoint *pt);

}
#endif // GWIN_H
