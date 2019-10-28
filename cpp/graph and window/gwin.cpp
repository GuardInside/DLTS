#include "gwin.h"
#define DEFAULT_HEIGHT_FONT 80
#define DEFAULT_WIDTH_FONT  20
#define DEFAULT_PRECISION   3
#define GWND_LOGICAL_SIZE   1200
#define GPLOT_LOGICAL_SIZE  1000
using namespace gwin;

namespace gwin
{
    struct gPlotInfo
    {
        ~gPlotInfo()
        {
            DeleteObject((HBITMAP)hBitMap);
            DeleteObject((HPEN)hAxisPen);
            DeleteObject((HPEN)hPlotPen);
            DeleteObject((HPEN)hGridPen);
            DeleteObject((HFONT)hTextFont);
        }
        gPlotInfo():
            hBitMap{NULL},
            iMark1{5}, iMark2{10}, iAdMark1{2}, iAdMark2{0},
            iPrec1{DEFAULT_PRECISION}, iPrec2{DEFAULT_PRECISION},
            hAxisPen{CreatePen(PS_SOLID, 0, RGB(255,255,255))},
            hPlotPen{CreatePen(PS_SOLID, 0, RGB(255,215,0))},
            hGridPen{CreatePen(PS_DOT, 0, RGB(192,192,192))},
            crTextColor{RGB(0,255,0)}, crAdInfoColor{RGB(0,255,0)},
            hTextFont{CreateFont(DEFAULT_HEIGHT_FONT, DEFAULT_WIDTH_FONT, 0, 0, FW_LIGHT, 1, 0, 0,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 PROOF_QUALITY, FIXED_PITCH | FF_MODERN, "")},
            bfEnableGrid{true}{}
        HBITMAP hBitMap;
        UINT iMark1, iMark2, iAdMark1, iAdMark2;
        INT iPrec1, iPrec2;
        HPEN hAxisPen, hPlotPen, hGridPen;
        COLORREF crTextColor, crAdInfoColor;
        HFONT hTextFont;
        BOOL bfEnableGrid, bfEnableAdInfo;
        std::string strAdInfo;
    };

    class gBuffer
    {
        public:
            std::stringstream& operator<<(double dNum)
            {
                buffer.str("");
                buffer << dNum;
                return buffer;
            }
            std::stringstream& operator<<(std::string str)
            {
                buffer.str("");
                buffer << str;
                return buffer;
            }
            std::stringstream& operator>>(std::string& str)
            {
                buffer >> str;
                return buffer;
            }
            std::string str()
            {
                return buffer.str();
            }
            void fixed()
            {
                buffer << std::fixed;
            }
            void scientific()
            {
                buffer << std::scientific;
            }
            void setprecision(int p)
            {
                buffer << std::setprecision(p);
            }
        private:
            std::stringstream buffer;
    };

    std::map<HWND, gPlotInfo> _gMap;

    VOID gWindowMetric(HWND hWnd, LONG *iWidth, LONG *iHeight);
    VOID gClearBitmap(HWND hWnd);
    VOID gTransformPlot(HWND hWnd, HDC &hdc);
    BOOL gMulDrawPlot(HWND hWnd, HDC &hdc, const gVector *vData1, const gMulVector *vMulData2);
    BOOL gDrawPlot(HWND hWnd, HDC &hdc, const gVector *vData1, const gVector *vData2);
    BOOL gDrawLegend(HWND hWnd, HDC& hdc);
    BOOL gEmbedBitmap(HWND hWnd, HDC &hdc);
    BOOL gSplit(std::string str, gInfoVector *vInfo);

    LRESULT CALLBACK _gWinProcess (HWND, UINT, WPARAM, LPARAM);
    VOID _gOnCommand(HWND, int, HWND, UINT);
    VOID _gOnPaint(HWND hWnd);
    VOID _gOnLButtonDown(HWND hWnd, BOOL, INT, INT, UINT);
}

/* ********************** */
/*  ��������� ����������  */
/* ********************** */
BOOL gwin::gMark(HWND hWnd, int iMark1, int iMark2, int iAdMark1, int iAdMark2)
{
    if(iMark1 < 0 || iMark2 < 0 || iAdMark1 < 0 || iAdMark2 < 0)
        return FALSE;
    _gMap.at(hWnd).iMark1 = iMark1;
    _gMap.at(hWnd).iMark2 = iMark2;
    _gMap.at(hWnd).iAdMark1 = iAdMark1;
    _gMap.at(hWnd).iAdMark2 = iAdMark2;
    return TRUE;
}

BOOL gwin::gAdditionalInfo(HWND hWnd, std::string str)
{
    if(str.empty())
    {
        _gMap.at(hWnd).bfEnableAdInfo = FALSE;
        return FALSE;
    }
    _gMap.at(hWnd).strAdInfo = str;
    _gMap.at(hWnd).bfEnableAdInfo = TRUE;
    return TRUE;
}

HWND gwin::gCreateWindow(HINSTANCE hInst, HWND hWndParent, DWORD style)
{
    const std::string strClass = "gWinClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = _gWinProcess;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(WHITE_BRUSH);
    wc.lpszClassName = strClass.c_str();
    RegisterClass(&wc);
    HWND hWnd = CreateWindow(strClass.c_str(), "", style, 0, 0, 100, 100, hWndParent, NULL, hInst, NULL);
    _gMap[hWnd];
    return hWnd;
}

BOOL gwin::gClose(HWND hWnd)
{
    _gMap.erase(hWnd);
    return DestroyWindow(hWnd);
}

BOOL gwin::gMulData(HWND hWnd, const gVector *vData1, const gMulVector *vMulData)
{
    /** �������� ������� **/
    if(vData1->size() == 0)
        return FALSE;
    /** ���������� � ��������� **/
    LONG iWidth, iHeight;
    gWindowMetric(hWnd, &iWidth, &iHeight);
    HDC hdc = CreateCompatibleDC(GetDC(hWnd));
    HBITMAP bitPlot = CreateCompatibleBitmap(GetDC(hWnd), iWidth, iHeight);
    SelectObject(hdc, bitPlot);
    gTransformPlot(hWnd, hdc);
    /** ��������� ������� � ������ ������ **/
    PatBlt(hdc, 0, 0, iWidth, iHeight, BLACKNESS);
    gMulDrawPlot(hWnd, hdc, vData1, vMulData);
    /** ����� ��������� ������� **/
    gClearBitmap(hWnd);
    _gMap.at(hWnd).hBitMap = bitPlot;
    /** ��������� ������� � ������ ������ **/
    //PatBlt(hdc, 0, 0, iWidth, iHeight, BLACKNESS);
    //gMulDrawPlot(hWnd, hdc, vData1, vMulData);
    /** ����� ��������� **/
    DeleteDC(hdc);
    InvalidateRect(hWnd, NULL, FALSE);
    return TRUE;
}

BOOL gwin::gData(HWND hWnd, const gVector *vData1, const gVector *vData2)
{
    /** �������� ������� **/
    if(vData1->size() == 0)
        return FALSE;
    const gMulVector vMulData2{*vData2};
    return gMulData(hWnd, vData1, &vMulData2);
}

BOOL gwin::gPosition(HWND hWnd, INT x, INT y)
{
    return SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

BOOL gwin::gSize(HWND hWnd, INT width, INT height)
{
    return SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
}

BOOL gwin::gFont(HWND hWnd, int iHeight, int iWidth, int iAngle, int iWeight, std::string strFontName)
{
    if(iWeight < 0 || iWeight > 1000) return FALSE;
    if(iWeight == 0) iWeight = FW_NORMAL;
    if(iHeight == 0) iHeight = DEFAULT_HEIGHT_FONT;
    if(iWidth  == 0) iWidth  = DEFAULT_WIDTH_FONT;
    DeleteObject((HFONT)_gMap.at(hWnd).hTextFont);
    _gMap.at(hWnd).hTextFont = CreateFont(iHeight, iWidth, 0, iAngle, iWeight, 1, 0, 0,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             PROOF_QUALITY, FIXED_PITCH | FF_MODERN, strFontName.c_str());
    return TRUE;
}


/* **************************************************************** */
/* ���������� ������� ����������. ������� ��� ������ �������������. */
/* **************************************************************** */
VOID gwin::gWindowMetric(HWND hWnd, LONG *iWidth, LONG *iHeight)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    //GetWindowRect(hWnd, &rc);
    *iWidth = abs(abs(rc.right)-abs(rc.left));
    *iHeight = abs(abs(rc.top)-abs(rc.bottom));
}

VOID gwin::gClearBitmap(HWND hWnd)
{
    if(_gMap.find(hWnd) != _gMap.end())
        DeleteObject((HBITMAP)_gMap.at(hWnd).hBitMap);
}

VOID gwin::gTransformPlot(HWND hWnd, HDC &hdc)
{
    LONG iWidth, iHeight;
    gWindowMetric(hWnd, &iWidth, &iHeight);
    SetMapMode(hdc, MM_ANISOTROPIC);
    SetWindowExtEx(hdc, GWND_LOGICAL_SIZE, -GWND_LOGICAL_SIZE, NULL);
    SetViewportExtEx(hdc, iWidth, iHeight, NULL);
}

BOOL gwin::gMulDrawPlot(HWND hWnd, HDC &hdc, const gVector *vData1, const gMulVector *vMulData2)
{
    static CONST LONG iPlotSize = GPLOT_LOGICAL_SIZE; /* ������ ������� � ���������� �������� */
    HPEN hAxisPen = _gMap.at(hWnd).hAxisPen,
         hPlotPen = _gMap.at(hWnd).hPlotPen,
         hGridPen = _gMap.at(hWnd).hGridPen,
         hOldPen;
    INT iIndent = iPlotSize/100;
    UINT iMark1 = _gMap.at(hWnd).iMark1, iMark2 = _gMap.at(hWnd).iMark2,
         iAdMark1 = _gMap.at(hWnd).iAdMark1,  iAdMark2 = _gMap.at(hWnd).iAdMark2;/* ����� ����� */
    COLORREF OldTextColor;
    HFONT hOldFont;
    gBuffer buff;
    double  z = 0.0, hx = 0.0, hy = 0.0,
            max_y = vMulData2->at(0).at(0), min_y = vMulData2->at(0).at(0),
            max_x = vData1->at(0), min_x = vData1->at(0);
    int     x = 0, y = 0;
    size_t  i = 0;
    /* ���������� ��������� */
    for(const auto &itir: *vData1)
        if(max_x < itir) max_x = itir;
        else if(min_x > itir) min_x = itir;
    for(const auto &extiter: *vMulData2)
        for(const auto &itir: extiter)
            if(max_y < itir) max_y = itir;
            else if(min_y > itir) min_y = itir;
    hx = (max_x - min_x)/iMark1;
    hy = (max_y - min_y)/iMark2;
    /* �������� ������ ��������� */
    LONG iWidth, iHeight;
    gWindowMetric(hWnd, &iWidth, &iHeight);
    POINT pIndent = {150, 100};
    SetViewportOrgEx(hdc, pIndent.x*((double)iWidth/GWND_LOGICAL_SIZE),
                          iHeight - pIndent.y*((double)iHeight/GWND_LOGICAL_SIZE), NULL);
    /* ��������� ������������ ��������� */
    hOldPen = (HPEN)SelectObject(hdc, (HPEN)hAxisPen);
    hOldFont = (HFONT)SelectObject(hdc, (HFONT)_gMap.at(hWnd).hTextFont);
    OldTextColor = SetTextColor(hdc, _gMap.at(hWnd).crTextColor);
    buff.fixed();
    SetBkMode(hdc, TRANSPARENT);
    /* ������ ������������� ������������ ����� */
    for(int c = 0; c < 2; c++)
    {
        /* ��������� ������� ��� X */
        if(c == 0) buff.setprecision(_gMap.at(hWnd).iPrec1);
        const int Y = (iPlotSize + 0.5)*c;
        MoveToEx(hdc, 0, 0+Y, NULL);
        LineTo(hdc, iPlotSize, 0+Y);
        for(z = min_x, i = 0; i <= iMark1; z += hx, i++)
        {
            x = int((z - min_x)*iPlotSize/(max_x - min_x) + 0.5);
            MoveToEx(hdc, x, -iIndent+Y, NULL);
            LineTo(hdc, x, iIndent+Y);
            if(c == 0)
            {
                buff << z;
                TextOut(hdc, x-0.5*pIndent.x, y - iIndent, buff.str().c_str(), buff.str().length());
            }
        }
        /* ��������� ������� ��� Y */
        if(c == 0) buff.setprecision(_gMap.at(hWnd).iPrec2);
        const int X = (iPlotSize + 0.5)*c;
        MoveToEx(hdc, 0+X, iPlotSize, NULL);
        LineTo(hdc, 0+X, 0);
        for (z = min_y, i = 0; i <= iMark2; z += hy, i++)
        {
            y = int((z - min_y)*iPlotSize/(max_y - min_y) + 0.5);
            MoveToEx(hdc, -iIndent+X, y, NULL);
            LineTo(hdc, iIndent+X, y);
            if(c == 0)
            {
                buff << z;
                TextOut(hdc, -pIndent.x, y+0.5*pIndent.y, buff.str().data(), buff.str().length());
            }
        }
    }
    /* ��������� �����, ���� ���������� */
    SelectObject(hdc, (HPEN)hGridPen);
    if(_gMap.at(hWnd).bfEnableGrid)
    {
        /* �������������� ����� */
        for(z = min_y + hy/(iAdMark2+1); z < max_y; z += hy/(iAdMark2+1))
        {
            y = int((z - min_y)*iPlotSize/(max_y - min_y) + 0.5);
            MoveToEx(hdc, iIndent, y, NULL);
            LineTo(hdc, iPlotSize-iIndent, y);
        }
        /* ������������ ����� */
        for(z = min_x + hx/(iAdMark1+1); z < max_x; z += hx/(iAdMark1+1))
        {
            x = int((z - min_x)*iPlotSize/(max_x - min_x) + 0.5);
            MoveToEx(hdc, x, iIndent, NULL);
            LineTo(hdc, x, iPlotSize-iIndent);
        }
    }
    /* ������ ������ */
    SelectObject(hdc, (HPEN)hPlotPen);
    const gVector &xAxis = *vData1;
    const int iMaxPlotNumber = vMulData2->size();
    int iPlotNumber = 1;
    for(const auto &extiter: *vMulData2)
    {
        if(iMaxPlotNumber > 1)
        {
            hPlotPen = CreatePen(PS_SOLID, 0, RGB(255.0*iPlotNumber/iMaxPlotNumber, 0, 0));
            SelectObject(hdc, (HPEN)hPlotPen);
            iPlotNumber++;
        }
        const gVector &yAxis = extiter;
        y = int((yAxis[0] - min_y)*iPlotSize/(max_y - min_y));
        MoveToEx(hdc, 0, y, NULL);
        for(i = 1; i < xAxis.size(); i++)
        {
            x = int((xAxis[i] - min_x)*iPlotSize/(max_x - min_x) + 0.5);
            y = int((yAxis[i] - min_y)*iPlotSize/(max_y - min_y) + 0.5);
            LineTo(hdc, x, y);
        }
        if(iMaxPlotNumber > 1) DeleteObject(hPlotPen);
    }
    /* ������� �������������� ���������� */
    SetTextColor(hdc, _gMap.at(hWnd).crAdInfoColor);
    if(_gMap.at(hWnd).bfEnableAdInfo)
    {
        gInfoVector vInfo;
        gSplit(_gMap.at(hWnd).strAdInfo, &vInfo);
        int s = 0;
        for(const auto &it: vInfo)
        {
            SIZE szText;
            GetTextExtentPoint32(hdc, it.data(), it.length(), &szText);
            TextOut(hdc, iPlotSize-szText.cx, iPlotSize - szText.cy*(s++), it.data(), it.length());
        }
    }
    SelectObject(hdc, (HPEN)hOldPen);
    SelectObject(hdc, (HFONT)hOldFont);
    SetTextColor(hdc, OldTextColor);
    return TRUE;
}

BOOL gwin::gDrawPlot(HWND hWnd, HDC &hdc, const gVector *vData1, const gVector *vData2)
{
    const gMulVector vMulData2{*vData2};
    return gMulDrawPlot(hWnd, hdc, vData1, &vMulData2);
}

BOOL gwin::gEmbedBitmap(HWND hWnd, HDC &hdc)
{
    LONG iWidth, iHeight;
    gWindowMetric(hWnd, &iWidth, &iHeight);
    HDC memhdc;
    if(_gMap.find(hWnd) != _gMap.end())
    {
        memhdc = CreateCompatibleDC(hdc);
        SelectObject(memhdc, _gMap.at(hWnd).hBitMap);
        BitBlt(hdc, 0, 0, iWidth, iHeight, memhdc, 0, 0, SRCCOPY);
        DeleteDC(memhdc);
        return TRUE;
    }
    return FALSE;
}

BOOL gwin::gSplit(std::string str, gInfoVector *vInfo)
{
    if(str.empty()) return FALSE;
    for(size_t i = 0, j = 0, s = 0, len = 0; i < str.length(); i++)
    {
        len++;
        if(str[i] == '\n' || i == str.length()-1)
        {
            vInfo->push_back(std::string(str, j, len));
            j = i + 1;
            s++;
            len = 0;
        }
    }
    return TRUE;
}

gwin::gDrawLegend(HWND hWnd, HDC &hdc)
{


}
/* *************** */
/* ������� ������� */
/* *************** */

LRESULT CALLBACK gwin::_gWinProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hWnd, WM_PAINT, _gOnPaint);
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, _gOnLButtonDown);
        //HANDLE_MSG(hWnd, WM_COMMAND, _gOnCommand);
        //HANDLE_MSG(hWnd, WM_TIMER, MainWindow_OnTimer);
        case WM_CREATE:
            break;
        default:
            return DefWindowProc (hWnd, message, wParam, lParam);
    }
    return 0;
}

VOID gwin::_gOnPaint(HWND hWnd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    hdc = BeginPaint(hWnd, &ps);
        gEmbedBitmap(hWnd, hdc);
    EndPaint(hWnd, &ps);
}

VOID gwin::_gOnLButtonDown(HWND hWnd, BOOL, INT, INT, UINT)
{
    HDC hdc;
    //gwin::gDrawLegend(hWnd, hdc);
}