
#ifndef MIX_FILL_H
#define MIX_FILL_H

#include <stdint.h>
#include <stdbool.h>

//==================== STATUS ====================//
typedef enum
{
    MIX_IDLE = 0,
    MIX_RUNNING,
    MIX_FILLED,
    MIX_MAINTAINING,
    MIX_ABORT,
    MIX_DONE

} mix_status_t;

//==================== API ====================//
void Mix_Init(void);
void Mix_Run(void);

// Control inputs (to be driven by CAN later)
void Mix_SetStart(uint8_t val);
void Mix_SetPreheatRequest(uint8_t val);

// Status outputs
mix_status_t Mix_GetStatus(void);

#endif


