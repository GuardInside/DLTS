#include <GPIB.h>

/* Объект-термостат */
THERMOSTAT Thermostat;

void THERMOSTAT::Apply()
{
    /* Считываем данные с канала А */
    Write("CCHN A");
}

/* Проверка корректности диапазона измерения температуры */
bool THERMOSTAT::range_is_correct() const
{
    if(EndPoint >= SetPoint && TempStep >= 0)
        return true;
    else if(EndPoint <= SetPoint && TempStep <= 0)
        return true;
    return false;
}
