#include <GPIB.h>

/* ������-��������� */
THERMOSTAT Thermostat;

void THERMOSTAT::Apply()
{
    /* ��������� ������ � ������ � */
    Write("CCHN A");
}

/* �������� ������������ ��������� ��������� ����������� */
bool THERMOSTAT::range_is_correct() const
{
    if(EndPoint >= SetPoint && TempStep >= 0)
        return true;
    else if(EndPoint <= SetPoint && TempStep <= 0)
        return true;
    return false;
}
