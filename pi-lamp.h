#ifndef __PI_LAMP_H__
#define __PI_LAMP_H__

#include <wiringPi.h>
#include "clickButton/clickButton.h"
#include "switchmate/switchmate.h"
#include "hue/hue.h"

void matchToggle(LampOwners owner);

void toggleLight(void);

#endif
