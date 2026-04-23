#ifndef VERIFY_MODULE_H
#define VERIFY_MODULE_H

#include <stdint.h>
#include <stdbool.h>

//==================== STATUS ====================//
typedef enum
{
    VERIFY_IDLE = 0,
    VERIFY_MONITORING,
    VERIFY_OK,
    VERIFY_DRAINING,
    VERIFY_READY_FOR_MIX,
    VERIFY_FAULT

} verify_status_t;

//==================== LOCAL FAULT ENUM ====================//
typedef enum
{
    VERIFY_FAULT_NONE = 0,
    VERIFY_FAULT_DRY_RUN,
    VERIFY_FAULT_OVERFLOW,
    VERIFY_FAULT_SENSOR_INVALID,
    VERIFY_FAULT_OVERTEMP

} verify_fault_t;

//==================== API ====================//
void Verify_Init(void);
void Verify_Run(void);

void Verify_SetStart(uint8_t val);
void Verify_SetDrain(uint8_t val);

verify_status_t Verify_GetStatus(void);
verify_fault_t Verify_GetFault(void);
void Verify_ClearFault(void);

#endif
