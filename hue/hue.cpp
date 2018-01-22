#include "hue.h"

// http://stackoverflow.com/a/9786295
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

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
            curl_easy_setopt(curl, CURLOPT_URL, "192.168.1.16/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/4");
        }
        else if (owner == sara){
            curl_easy_setopt(curl, CURLOPT_URL, "192.168.1.16/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/3");
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "GET request failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);

        DynamicJsonBuffer jsonBuffer;

        JsonObject& root = jsonBuffer.parseObject(readBuffer);

        if (!root.success()) {
            std::cerr << "JSON parse failed" << std::endl;
            return -1;
        }

        std::string state = root["state"]["on"];

        // compares the "on" "state" of readBuffer (the response)
        // which is either "true" or "false"
        // to statusOn which is "true"
        // 0 if they're the same, 1 if they're different
        // which is inverse of what we want
        // return 1 if matches "true" (0)
        // return 0 if doesn't match "true" (1)
        return statusOn.compare(state)? 0 : 1;
    }
    // error!
    return -1;
}

void switchLamp(LampOwners owner, bool on){
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
            std::cout << "Switching Dillon's lamp: ";
            curl_easy_setopt(curl, CURLOPT_URL, "192.168.1.16/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/4/state");
        }
        else if (owner == sara){
            std::cout << "Switching Sara's lamp: ";
            curl_easy_setopt(curl, CURLOPT_URL, "192.168.1.16/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/3/state");
        }
        else if (owner == both){
            std::cout << "Switching both lamps: ";
            curl_easy_setopt(curl, CURLOPT_URL, "192.168.1.16/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/groups/1/action");
        }
        else{
            return;
        }
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

        if (on){
            // when turning on, set to full brightness
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"on\":true, \"bri\":254}");
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

        DynamicJsonBuffer jsonBuffer;

        // this time it is an array
        JsonArray& root = jsonBuffer.parseArray(readBuffer);

        if (!root.success()) {
            std::cerr << "JSON parse failed" << std::endl;
        }

        // first object [0] in array is on/off state
        // second (when turning on) is the brightness it was set to (ignoring)
        JsonObject& object0 = root[0];

        // read first key
        for (JsonPair& pair : object0)
        {
            // success or error
            std::cout << pair.key;

            // set up new object for that key
            JsonObject& objectStatus = object0[pair.key];

            // if success:
            if (strcmp(pair.key, "success") == 0){
                // read second key
                for (JsonPair& pair : objectStatus)
                {
                    // true or false (on or off)
                    if (pair.value)
                    {
                        std::cout << ". Light is now on." << std::endl;
                    }
                    else{
                        std::cout << ". Light is now off." << std::endl;
                    }
                }
            }
            // else is error:
            else{
                std::cout << "! " << objectStatus["description"] << std::endl;
            }
        }
    }
}

void dillonOn(void){
    switchLamp(dillon, true);
};
void dillonOff(void){
    switchLamp(dillon, false);
};
/* read status of Dillon's lamp and toggle*/
void toggleDillon(void){
    // if off,
    if (lampStatus(dillon) == 0){
        //turn on Dillon's lamp
        dillonOn();
    }
    // if on,
    else{
        // turn off Dillon's lamp
        dillonOff();
    }
}

void saraOn(void){
    switchLamp(sara, true);
};
void saraOff(void){
    switchLamp(sara, false);
};
/* read status of Sara's lamp and toggle */
void toggleSara(void){
    // if off,
    if (lampStatus(sara) == 0){
        // turn on Sara's lamp
        saraOn();
    }
    // if on,
    else{
        // turn off Sara's lamp
        saraOff();
    }
}

/* set both lamps to a new state */
void bothOn(void){
    switchLamp(both, true);
};
void bothOff(void){
    switchLamp(both, false);
};
