#include <GPIB.h>

using namespace std;

GPIB::GPIB()
{
    viOpenDefaultRM(&hResMeneger);
    hInstrument = 0;
}

/* ������������� ��� �������� */
void GPIB::InitSession()
{
    #ifndef TEST_MODE
    if(Init() != VI_SUCCESS)
        MessageBox(NULL, "An instrument wasn't initialized.", "Error", MB_ICONWARNING);
    #endif // TEST_MODE
}

//������������� ������ �� ���������;
ViStatus GPIB::Init()
{
    stringstream buff;
    if(ID < 10) buff << "GPIB0::" << ID << "::INSTR";
    else buff << "GPIB::" << ID << "::INSTR";
    if(hInstrument) viClose(hInstrument);
    return viOpen(hResMeneger, (ViRsrc)buff.str().data(), VI_NULL, VI_NULL, &hInstrument);
}

/* �������� ��������� � ������� */
ViStatus GPIB::Write(const stringstream& mes, ViUInt32*  rcount)
{
    return viWrite(hInstrument, (ViBuf)mes.str().data(), (ViUInt32)mes.str().length(), rcount);
}
ViStatus GPIB::Write(const string mes, ViUInt32*  rcount)
{
    return viWrite(hInstrument, (ViBuf)mes.data(), (ViUInt32)mes.length(), rcount);
}
void GPIB::Read(string& strResult)
{
    ZeroMemory(ViBuffer, READ_BUFFER_SIZE);
    viRead(hInstrument, ViBuffer, READ_BUFFER_SIZE-1, &read);
    strResult = (string)(char*)ViBuffer;
}

/* ��������� ���������-����� �� ������� */
void GPIB::ReadDigit(double& digit)
{
    viRead(hInstrument, ViBuffer, READ_BUFFER_SIZE-1, &read);
    ViBuffer[read] = '\0';
    digit = atof((char*)ViBuffer);
}
void GPIB::ReadDigit(int& digit)
{
    viRead(hInstrument, ViBuffer, READ_BUFFER_SIZE-1, &read);
    ViBuffer[read] = '\0';
    digit = atoi((char*)ViBuffer);
}
