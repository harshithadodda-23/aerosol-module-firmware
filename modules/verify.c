#include "verify_module.h"
#include "level_sensor.h"
#include "temperature.h"
#include "safety.h"
#include "actuators.h"

//==================== STATES ====================//
typedef enum
{
    V_STATE_INIT,
    V_STATE_MONITOR,
    V_STATE_VALIDATE,
    V_STATE_PROCESS_CMD,
    V_STATE_DRAIN,
    V_STATE_WAIT_START,
    V_STATE_FAULT

} V_STATE;

//==================== VARIABLES ====================//
static V_STATE state;
static verify_status_t status;
static verify_fault_t fault;

static uint8_t start_request = 0;
static uint8_t drain_request = 0;

//==================== FAULT REPORT ====================//
static void ReportFault(verify_fault_t f)
{
    // TODO: CAN or logging
}

//==================== FAULT DETECTION ====================//
static verify_fault_t DetectFault(void)
{
    level_status_t s = Level_GetStatus();
    float chamberTemp = Temp_GetChamber();
    float heaterTemp  = Temp_GetHeater();

    if (!Level_IsSensorValid())
        return VERIFY_FAULT_SENSOR_INVALID;

    if (s.estop_low == 0)
        return VERIFY_FAULT_DRY_RUN;

    if (s.estop_high == 1)
        return VERIFY_FAULT_OVERFLOW;

    if (Safety_CheckOverTemp(chamberTemp, heaterTemp))
        return VERIFY_FAULT_OVERTEMP;

    return VERIFY_FAULT_NONE;
}

//==================== INIT ====================//
void Verify_Init(void)
{
    state = V_STATE_INIT;
    status = VERIFY_IDLE;
    fault = VERIFY_FAULT_NONE;
}

//==================== MAIN ====================//
void Verify_Run(void)
{
    switch(state)
    {
        case V_STATE_INIT:
            state = V_STATE_MONITOR;
            status = VERIFY_MONITORING;
            break;

        case V_STATE_MONITOR:
            state = V_STATE_VALIDATE;
            break;

        case V_STATE_VALIDATE:
        {
            verify_fault_t f = DetectFault();

            if (f != VERIFY_FAULT_NONE)
            {
                fault = f;
                ReportFault(f);
                state = V_STATE_FAULT;
                status = VERIFY_FAULT;
            }
            else
            {
                state = V_STATE_PROCESS_CMD;
                status = VERIFY_OK;
            }
        }
        break;

        case V_STATE_PROCESS_CMD:

            if (drain_request)
            {
                state = V_STATE_DRAIN;
                status = VERIFY_DRAINING;
            }
            else
            {
                Actuator_DosingValve_Off();
                Actuator_Vent_Off();
                state = V_STATE_WAIT_START;
            }
            break;

        case V_STATE_DRAIN:

            Actuator_DosingValve_On();
            Actuator_Vent_On();

            if (!drain_request)
            {
                Actuator_DosingValve_Off();
                Actuator_Vent_Off();
                state = V_STATE_WAIT_START;
            }
            break;

        case V_STATE_WAIT_START:

            if (start_request)
            {
                state = V_STATE_MONITOR;
                status = VERIFY_READY_FOR_MIX;
            }
            else
            {
                state = V_STATE_MONITOR;
            }
            break;

        case V_STATE_FAULT:

            Actuator_DosingValve_Off();
            Actuator_Vent_Off();
            status = VERIFY_FAULT;
            break;

        default:
            state = V_STATE_FAULT;
            break;
    }
}

//==================== GETTERS ====================//
verify_status_t Verify_GetStatus(void)
{
    return status;
}

verify_fault_t Verify_GetFault(void)
{
    return fault;
}

void Verify_ClearFault(void)
{
    fault = VERIFY_FAULT_NONE;
    state = V_STATE_INIT;
}
