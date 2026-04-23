#include "state_machine.h"
#include "temperature.h"
#include "safety.h"
#include "../mcc_generated_files/pin_manager.h"
#include "../app/app.h"
#include "verify_module.h"
#include "mix_fill.h"
#include "actuators.h"

//==================== STATES ====================//
typedef enum
{
    STATE_INIT,
    STATE_VERIFY,
    STATE_MIX_FILL,
    STATE_HEAT,
    STATE_FOG,
    STATE_FAULT

} SYSTEM_STATE;

//==================== VARIABLES ====================//
static SYSTEM_STATE currentState;
static uint32_t heatStartTime = 0;

extern volatile uint32_t g_msTicks;

//==================== INIT ====================//
void SM_Init(void)
{
    currentState = STATE_INIT;

    Verify_Init();
    Mix_Init();
}

//==================== MAIN ====================//
void SM_Run(void)
{
    static float chamberTemp = 0;
    static float heaterTemp  = 0;

    switch(currentState)
    {
        //---------------- INIT ----------------//
        case STATE_INIT:

            currentState = STATE_VERIFY;
            break;

        //---------------- VERIFY ----------------//
        case STATE_VERIFY:

            Verify_Run();

            if (Verify_GetStatus() == VERIFY_READY_FOR_MIX)
            {
                Mix_SetStart(1);
                currentState = STATE_MIX_FILL;
            }
            else if (Verify_GetStatus() == VERIFY_FAULT)
            {
                currentState = STATE_FAULT;
            }

            break;

        //---------------- MIX & FILL ----------------//
        case STATE_MIX_FILL:

            Mix_Run();

            if (Mix_GetStatus() == MIX_DONE)
            {
                currentState = STATE_HEAT;
            }
            else if (Mix_GetStatus() == MIX_ABORT)
            {
                currentState = STATE_FAULT;
            }

            break;

        //---------------- HEAT ----------------//
        case STATE_HEAT:

            chamberTemp = Temp_GetChamber();
            heaterTemp  = Temp_GetHeater();

            if (heatStartTime == 0)
                heatStartTime = g_msTicks;

            if (Safety_CheckOverTemp(chamberTemp, heaterTemp))
            {
                Heater_SetLow();
                currentState = STATE_FAULT;
                break;
            }

            if (chamberTemp < 108.0f)
                Heater_SetHigh();
            else if (chamberTemp > 115.0f)
                Heater_SetLow();

            if (chamberTemp >= 108.0f && chamberTemp <= 115.0f)
            {
                Heater_SetLow();
                heatStartTime = 0;

                Mix_SetPreheatRequest(1);
                currentState = STATE_FOG;
            }
            else if ((g_msTicks - heatStartTime) > 600000)
            {
                Heater_SetLow();
                currentState = STATE_FAULT;
            }

            break;

        //---------------- FOG ----------------//
        case STATE_FOG:

            chamberTemp = Temp_GetChamber();

            if (chamberTemp < 108.0f)
                Heater_SetHigh();
            else if (chamberTemp > 115.0f)
                Heater_SetLow();

            Actuator_DosingValve_On();
            Actuator_Vent_On();

            // Loop back
            currentState = STATE_VERIFY;

            break;

        //---------------- FAULT ----------------//
        case STATE_FAULT:

            // Safe shutdown
            Heater_SetLow();
            Actuator_WaterPump_Off();
            Actuator_DosingValve_Off();
            Actuator_Vent_Off();

            // TODO: Add fault reporting via CAN
            // fault_code_t f = Verify_GetFault();

            break;

        //---------------- DEFAULT ----------------//
        default:
            currentState = STATE_FAULT;
            break;
    }
}
