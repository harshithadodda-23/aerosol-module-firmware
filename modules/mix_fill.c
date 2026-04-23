#include "mix_fill.h"
#include "level_sensor.h"
#include "temperature.h"
#include "safety.h"
#include "actuators.h"

//==================== CONFIG ====================//
#define MAX_FILL_TIME_MS   120000

//==================== LOCAL FAULT ENUM ====================//
typedef enum
{
    MIX_FAULT_NONE = 0,
    MIX_FAULT_TIMEOUT,
    MIX_FAULT_SENSOR,
    MIX_FAULT_OVERTEMP

} mix_fault_t;

//==================== STATES ====================//
typedef enum
{
    MF_STATE_IDLE,
    MF_STATE_MONITOR,
    MF_STATE_VALIDATE,
    MF_STATE_FILL,
    MF_STATE_FILLED,
    MF_STATE_MAINTAIN,
    MF_STATE_ABORT

} MF_STATE;

//==================== VARIABLES ====================//
static MF_STATE state;
static mix_status_t status;
static mix_fault_t fault;

static uint8_t start_request = 0;
static uint8_t preheat_request = 0;
static uint32_t fillStartTime = 0;

extern volatile uint32_t g_msTicks;

//==================== REPORT ====================//
static void ReportFault(mix_fault_t f)
{
    // TODO: CAN/logging
}

//==================== INIT ====================//
void Mix_Init(void)
{
    state = MF_STATE_IDLE;
    status = MIX_IDLE;
    fault = MIX_FAULT_NONE;
}

//==================== CONTROL ====================//
void Mix_SetStart(uint8_t val)
{
    start_request = val;
}

void Mix_SetPreheatRequest(uint8_t val)
{
    preheat_request = val;
}

//==================== STATUS ====================//
mix_status_t Mix_GetStatus(void)
{
    return status;
}

//==================== MAIN ====================//
void Mix_Run(void)
{
    level_status_t level = Level_GetStatus();
    float chamberTemp = Temp_GetChamber();
    float heaterTemp  = Temp_GetHeater();

    switch(state)
    {
        case MF_STATE_IDLE:

            status = MIX_IDLE;

            if (start_request)
            {
                state = MF_STATE_MONITOR;
                status = MIX_RUNNING;
            }
            break;

        case MF_STATE_MONITOR:
            state = MF_STATE_VALIDATE;
            break;

        case MF_STATE_VALIDATE:

            if (!Level_IsSensorValid() ||
                !Level_IsSafe() ||
                Safety_CheckOverTemp(chamberTemp, heaterTemp))
            {
                fault = MIX_FAULT_SENSOR;
                ReportFault(fault);
                state = MF_STATE_ABORT;
                status = MIX_ABORT;
            }
            else
            {
                state = MF_STATE_FILL;
                fillStartTime = g_msTicks;
            }
            break;

        case MF_STATE_FILL:

            status = MIX_RUNNING;

            if (level.full_level == 0)
            {
                Actuator_WaterPump_On();
            }
            else
            {
                Actuator_WaterPump_Off();
                state = MF_STATE_FILLED;
                status = MIX_FILLED;
            }

            if ((g_msTicks - fillStartTime) > MAX_FILL_TIME_MS)
            {
                fault = MIX_FAULT_TIMEOUT;
                ReportFault(fault);
                state = MF_STATE_ABORT;
            }

            break;

        case MF_STATE_FILLED:
            state = MF_STATE_MAINTAIN;
            break;

        case MF_STATE_MAINTAIN:

            status = MIX_MAINTAINING;

            if (level.low_level == 0)
                Actuator_WaterPump_On();
            else if (level.full_level == 1)
                Actuator_WaterPump_Off();

            if (preheat_request)
            {
                Actuator_WaterPump_Off();
                state = MF_STATE_IDLE;
                status = MIX_DONE;
            }

            break;

        case MF_STATE_ABORT:

            Actuator_WaterPump_Off();
            status = MIX_ABORT;
            break;

        default:
            state = MF_STATE_ABORT;
            break;
    }
}
