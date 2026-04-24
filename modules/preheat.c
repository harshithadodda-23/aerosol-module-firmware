#include "preheat.h"
#include "temperature.h"
#include "safety.h"
#include "actuators.h"

//================ STATES =================//
typedef enum
{
    PH_IDLE,
    PH_START,
    PH_HEATING,
    PH_STABLE,
    PH_DONE,
    PH_FAULT

} PH_STATE;

//================ VARIABLES =================//
static PH_STATE state;
static preheat_status_t status;
static uint8_t start_req = 0;

static uint32_t startTime = 0;

extern volatile uint32_t g_msTicks;

//================ INIT =================//
void Preheat_Init(void)
{
    state = PH_IDLE;
    status = PREHEAT_IDLE;
}

//================ START =================//
void Preheat_Start(uint8_t val)
{
    start_req = val;
}

//================ STATUS =================//
preheat_status_t Preheat_GetStatus(void)
{
    return status;
}

//================ RUN =================//
void Preheat_Run(void)
{
    float chamberTemp = Temp_GetChamber();
    float heaterTemp  = Temp_GetHeater();

    switch(state)
    {
        //---------------- IDLE ----------------//
        case PH_IDLE:

            if (start_req)
            {
                state = PH_START;
                status = PREHEAT_RUNNING;
            }
            break;

        //---------------- START ----------------//
        case PH_START:

            Actuator_HeaterSafety_On();
            Actuator_WaterPump_On();

            startTime = g_msTicks;

            state = PH_HEATING;
            break;

        //---------------- HEATING ----------------//
        case PH_HEATING:

            if (Safety_CheckOverTemp(chamberTemp, heaterTemp))
            {
                state = PH_FAULT;
                break;
            }

            if (chamberTemp < 108.0f)
                Actuator_Heater_On();
            else
                Actuator_Heater_Off();

            if (chamberTemp >= 108.0f && chamberTemp <= 115.0f)
            {
                state = PH_STABLE;
            }

            if ((g_msTicks - startTime) > 600000)
            {
                state = PH_FAULT;
            }

            break;

        //---------------- STABLE ----------------//
        case PH_STABLE:

            if (chamberTemp < 108.0f)
                Actuator_Heater_On();
            else if (chamberTemp > 115.0f)
                Actuator_Heater_Off();

            state = PH_DONE;
            status = PREHEAT_DONE;
            break;

        //---------------- DONE ----------------//
        case PH_DONE:

            Actuator_Heater_Off();
            Actuator_HeaterSafety_Off();
            break;

        //---------------- FAULT ----------------//
        case PH_FAULT:

            Actuator_Heater_Off();
            Actuator_HeaterSafety_Off();
            Actuator_WaterPump_Off();

            status = PREHEAT_FAULT;
            break;
    }
}

