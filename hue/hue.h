#ifndef __HUE_H__
#define __HUE_H__

#include <curl/curl.h>
#include "ArduinoJson/src/ArduinoJson.h"
#include <iostream>

// arguments for controlling a particular light
typedef enum {
    dillon,
    sara,
    both
} LampOwners;

// arguements for sending a parameter to a light
typedef enum {
  off,
  on,
  alert
} LampParameter;

int lampStatus(LampOwners owner);
void switchLamp(LampOwners owner, LampParameter parameter);

void dillonOn(void);
void dillonOff(void);
void toggleDillon(void);

void saraOn(void);
void saraOff(void);
void toggleSara(void);

void bothOn(void);
void bothOff(void);

#endif
