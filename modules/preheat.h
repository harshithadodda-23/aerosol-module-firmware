#ifndef PREHEAT_H
#define PREHEAT_H

#include <stdint.h>

//================ STATUS =================//
typedef enum
{
    PREHEAT_IDLE = 0,
    PREHEAT_RUNNING,
    PREHEAT_DONE,
    PREHEAT_FAULT

} preheat_status_t;

//================ API =================//
void Preheat_Init(void);
void Preheat_Start(uint8_t val);
void Preheat_Run(void);
preheat_status_t Preheat_GetStatus(void);

#endif
