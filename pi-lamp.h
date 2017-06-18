#include "clickButton/clickButton.h"
#include <curl/curl.h>
#include <wiringPi.h>
#include <stdio.h>
#include <iostream>
#include <string>

// http://stackoverflow.com/a/9786295
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// arguments for matchToggle
typedef enum {
    dillon,
    sara
} LampOwners;

// 1 == on
// 0 == off
int lampStatus(LampOwners owner){
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    std::string statusOn ("true");

    curl = curl_easy_init();
    if(curl) {
        // set options based on owner
        if (owner == dillon){
            curl_easy_setopt(curl, CURLOPT_URL, "philips-hue/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/4");
        }
        else if (owner == sara){
            curl_easy_setopt(curl, CURLOPT_URL, "philips-hue/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/3");
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "GET request failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);

        // compares the 15-18th characters of readBuffer (the response)
        // which is either "true" or "false"
        // to statusOn which is "true"
        // 0 if they're the same, 1 if they're different
        // which is inverse of what we want
        // return 1 if matches "true" (0)
        // return 0 if doesn't match "true" (1)
        return statusOn.compare(readBuffer.substr(15, 4))? 0 : 1;
    }
    // error!
    return -1;
}

void switchLamp(LampOwners owner, boolean on){
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

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

        // redirect the response to a buffer
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "GET request failed: %s\n", curl_easy_strerror(res));

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        // print only the important part ("true" or "false") " " ("success" or "failure")
        std::cout << "Lamp switch to " << readBuffer.substr(34, 4) << " " << readBuffer.substr(3, 7) << std::endl;
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
