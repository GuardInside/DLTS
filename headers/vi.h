#ifndef GPIB_H
#define GPIB_H

#include <windows.h>
#include <visatype.h>
#include <visa.h>
#include <string>
#include <sstream>
#include <variable.h>
#include <facility.h>
#include <daq.h>
#include "resource.h"

#define READ_BUFFER_SIZE  64
#define WRITE_BUFFER_SIZE 64
#define QUANTITY_ZONE     10

/* ������� ����� */
class VI
{
    public:
        VI();
        /* ������������� ������ �� ���������; */
        VOID InitSession();
        /* �������� ��������� � ������� */
        ViStatus Write(const std::stringstream& mes, ViUInt32*  rcount = nullptr);
        ViStatus Write(const std::string mes, ViUInt32*  rcount = nullptr);
        VOID Read(std::string&);
        /* ��������� ���������-����� �� ������� */
        VOID ReadDigit(double& digit);
        VOID ReadDigit(int& digit);
        int& GetID(){ return ID; }
    protected:
        std::string     ViName;
        ViSession       hResMeneger;
        ViSession       hInstrument;
        ViUInt32        read;
        unsigned char   ViBuffer[READ_BUFFER_SIZE];
        int             ID;
        ViStatus Init();
};

struct GENERATOR: public VI
{
    GENERATOR(std::string arg_ViName){ ViName = arg_ViName; };
    /* ��������� ������� ��������� � ������� */
    VOID Apply();

    double period, width, voltage_up, voltage_low;
    /* ��� ������ ITS */
    double begin_voltage, end_voltage, step_voltage;
    unsigned int channel;
    bool is_active;
};

struct THERMOSTAT: public VI
{
    private:
        struct PIDTABLE
        {
            unsigned int P[QUANTITY_ZONE], I[QUANTITY_ZONE], D[QUANTITY_ZONE],
                         range[QUANTITY_ZONE], upper_boundary[QUANTITY_ZONE];
            PIDTABLE();
            /* ���������� ������������� ������� */
            int GetIDRes(const int, const string);
            UINT GetActuallyHeatRange();
        };
    public:
        THERMOSTAT(std::string arg_ViName){ ViName = arg_ViName; };
        VOID Apply();
        /* �������� ������������ ��������� ��������� ����������� */
        bool range_is_correct() const;
        double SetPoint, EndPoint, TempStep, TempDisp;
        PIDTABLE ZoneTable;
};

extern GENERATOR    Generator;
extern THERMOSTAT   Thermostat;

#endif