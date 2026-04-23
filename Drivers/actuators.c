#include "actuators.h"
#include "../mcc_generated_files/pin_manager.h"

//================ WATER PUMP =================//
void Actuator_WaterPump_On(void)
{
    BlowerMotorLgEnable_SetHigh();
}

void Actuator_WaterPump_Off(void)
{
    BlowerMotorLgEnable_SetLow();
}

//================ DOSING VALVE =================//
void Actuator_DosingValve_On(void)
{
    VentValveInOpen_SetHigh();
}

void Actuator_DosingValve_Off(void)
{
    VentValveInOpen_SetLow();
}

//================ VENT =================//
void Actuator_Vent_On(void)
{
    VentValveInClosed_SetHigh();
}

void Actuator_Vent_Off(void)
{
    VentValveInClosed_SetLow();
}
