#ifndef SAFETY_H
#define SAFETY_H

#include <stdbool.h>

bool Safety_CheckOverTemp(float chamberTemp, float heaterTemp);
bool Safety_CheckWater(void);

#endif
