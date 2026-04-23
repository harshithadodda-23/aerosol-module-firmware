
#include "level_sensor.h"
#include <xc.h>

// Macros for readability
#define LOW_LEVEL_PIN     PORTHbits.RH5
#define FULL_LEVEL_PIN    PORTHbits.RH6
#define ESTOP_LOW_PIN     PORTHbits.RH4
#define ESTOP_HIGH_PIN    PORTHbits.RH7

void Level_Init(void)
{
    // Set pins as input
    TRISHbits.TRISH4 = 1;
    TRISHbits.TRISH5 = 1;
    TRISHbits.TRISH6 = 1;
    TRISHbits.TRISH7 = 1;
}

level_status_t Level_GetStatus(void)
{
    level_status_t status;

    status.low_level   = LOW_LEVEL_PIN;
    status.full_level  = FULL_LEVEL_PIN;
    status.estop_low   = ESTOP_LOW_PIN;
    status.estop_high  = ESTOP_HIGH_PIN;

    return status;
}

bool Level_IsSafe(void)
{
    level_status_t s = Level_GetStatus();

    // Safety logic
    if (s.estop_low == 0)   // no fluid at critical level
        return false;

    if (s.estop_high == 1)  // overflow detected
        return false;

    return true;
}
bool Level_IsLow(void)
{
    return (PORTHbits.RH5 == 0);
}

bool Level_IsFull(void)
{
    return (PORTHbits.RH6 == 1);
}

bool Level_IsDry(void)
{
    return (PORTHbits.RH4 == 0);
}

bool Level_IsOverflow(void)
{
    return (PORTHbits.RH7 == 1);
}
bool Level_IsSensorValid(void)
{
    level_status_t s = Level_GetStatus();

    // Invalid combinations

    // Dry + Overflow at same time ? impossible
    if (s.estop_low == 0 && s.estop_high == 1)
        return false;

    // Low level active but full not active ? inconsistent
    if (s.low_level == 1 && s.full_level == 0)
        return false;

    return true;
}
