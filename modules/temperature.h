#include <xc.h> // include processor files - each processor file is guarded.  
#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <xc.h>
#include <stdint.h>

void Temp_Init(void);
float Temp_GetChamber(void);
float Temp_GetHeater(void);

#endif   // TEMPERATURE_H

