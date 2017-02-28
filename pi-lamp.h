#include "clickButton/clickButton.h"
#include <curl/curl.h>
#include <wiringPi.h>
#include <stdio.h>

// arguments for matchToggle
typedef enum {
    dillon,
    sara
} LampOwners;

void switchLamp(LampOwners owner, boolean on){
    CURL *curl;
    CURLcode res;

    struct curl_slist *headers=NULL;

    curl = curl_easy_init();

    if (curl) {
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // set options based on owner
        if (owner == dillon){
            curl_easy_setopt(curl, CURLOPT_URL, "philips-hue/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/4/state");
        }
        else if (owner == sara){
            curl_easy_setopt(curl, CURLOPT_URL, "philips-hue/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/3/state");
        }
        else{
            return;
        }
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

        if (on){
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"on\":true}");
        }
        else {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"on\":false}");
        }
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "GET request failed: %s\n", curl_easy_strerror(res));

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}


void matchToggle(LampOwners owner);

void dillonOn(void){
    switchLamp(dillon, true);
};
void dillonOff(void){
    switchLamp(dillon, false);
};
void saraOn(void){
    switchLamp(sara, true);
};
void saraOff(void){
    switchLamp(sara, false);
};

void toggleDillon(void);
void toggleSara(void);

void switchLamps(boolean on);

void toggleLight(void);
