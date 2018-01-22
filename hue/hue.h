#ifndef __HUE_H__
#define __HUE_H__

#include <curl/curl.h>
#include "ArduinoJson/src/ArduinoJson.h"
#include <iostream>
#include <stdbool.h>

// arguments for controlling a particular light
typedef enum {
    dillon,
    sara,
    both
} LampOwners;

int lampStatus(LampOwners owner);
void switchLamp(LampOwners owner, bool on);

void dillonOn(void);
void dillonOff(void);
void toggleDillon(void);

void saraOn(void);
void saraOff(void);
void toggleSara(void);

void bothOn(void);
void bothOff(void);

#endif
