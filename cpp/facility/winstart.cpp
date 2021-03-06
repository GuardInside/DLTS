#include <facility.h>
#include <GPIB.h>

/* ����� ����� � �����, ��������� ���� */
UINT CALLBACK fMain_thCreateWindow(void*);

/*In Zone mode, the instrument will update the controlsettings each time the setpoint
crosses into a new zone. If you change the settingsmanually, the controller will use
the new setting while it is in the same zone, and willupdate to the programmed zone table
settings when the setpoint crosses into anew zone.*/
/* ������� ������� 0 = off, 1 = low, 2 = medium, 3 = high */
void StartButPush(HWND hwnd)
{
    /* ����������� ������ �� ������� DLTS-������ � ���� ������� ����� */
    stringstream buff;
    if(start == false)
        buff << "Stop";
    else if(start == true)
        buff << "Start";
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_SET), start);
    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_END), start);
    EnableWindow(GetDlgItem(hwnd, ID_EDITCONTROL_BEGIN), start);
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_SETTINGS), start);
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_GET_ARRHENIUS_GRAPH), start);
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_EXIT), start);
    EnableWindow(GetDlgItem(hwnd, ID_BUTTON_LOAD), start);
    SetDlgItemText(hwnd, ID_BUTTON_START, buff.str().data());
    if(start == false)
    {
        /* ��������� ��������� ��� ���������� */
        rewrite(buff) << "SETP " << Thermostat.SetPoint << "K";
        Thermostat.Write(buff);
        /* ������� �������, ���� ���� ����������� �������� */
        if(loading == true)
        {
            loading = false;
            /*ClearAxisDLTS();
            vector<vector<double>>().swap(SavedRelaxations);
            vector <double>().swap(Relaxation);
            InvalidateRect(hRelax, NULL, TRUE);
            InvalidateRect(hGraph_DLTS, NULL, TRUE);*/
        }
        start = true;
        endofits = false;
        /* ��������� RANGE � ������������ � ������� ����� */
        rewrite(buff) << "RANG " << ZoneTable.GetActuallyHeatRange();
        Thermostat.Write(buff);
    }
    else if(start == true)
    {
        stability = false;
        start = false;
        /* �������� ��������� ��������� */
        SendMessage(hProgress, PBM_SETPOS, 0, 0);
        /* ���� ������ � ������� ���� */
        Thermostat.Write("RANG 0");
    }
}

/* ����� ����� � �����, ��������� ���� */
UINT CALLBACK fMain_thCreateWindow(void* DLG_ID)
{
    MSG messages;
    CreateDialog(hInst, MAKEINTRESOURCE(*(int*)DLG_ID), 0, 0);
    while(GetMessage(&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
    return 0;
}

void thCreateWindow(HANDLE& thMain, int DLG_ID)
{
    DWORD status = 0;
    if(thMain != nullptr)
        GetExitCodeThread(thMain, &status);
    if(thMain == nullptr || status != STILL_ACTIVE)
        thMain = (HANDLE)_beginthreadex(nullptr, 0, fMain_thCreateWindow, &DLG_ID, 0, nullptr);
}
