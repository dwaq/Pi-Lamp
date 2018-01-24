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

void switchLamp(LampOwners owner, LampParameter parameter){
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
            std::cout << "Dillon's lamp: ";
            curl_easy_setopt(curl, CURLOPT_URL, "philips-hue/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/4/state");
        }
        else if (owner == sara){
            std::cout << "Sara's lamp: ";
            curl_easy_setopt(curl, CURLOPT_URL, "philips-hue/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/lights/3/state");
        }
        else if (owner == both){
            std::cout << "Both lamps: ";
            curl_easy_setopt(curl, CURLOPT_URL, "philips-hue/api/29ocf3mMaJ1XAtbqeKM60A4dFen9tSc96u1JuQAi/groups/1/action");
        }
        else{
            return;
        }
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

        if (parameter == on){
            // when turning on, set to full brightness
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"on\":true, \"bri\":254}");
        }
        else if (parameter == off) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"on\":false}");
        }
        else if (parameter == alert) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"alert\":\"select\"}");
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

        // assume true
        bool isSuccessful = true;

        if (!root.success()) {
            std::cerr << "JSON parse failed" << std::endl;
            isSuccessful = false;
        }

        if (parameter == alert) {
          // First and second objects
          JsonObject& object0 = root[0];
          JsonObject& object1 = root[1];

          // check that both are successful
          isSuccessful = object0.containsKey("success");
          isSuccessful = object1.containsKey("success");

          // first key (success)
          for (JsonPair& object1pair0 : object1) {

              // set up new object for that key
              JsonObject& object1pair0SubObject = object1[object1pair0.key];

              // second key
              for (JsonPair& object1pair1 : object1pair0SubObject) {
                  // contains alert
                  if (strstr(object1pair1.key, "alert") != NULL) {
                      std::cout << "alerted";
                  }
              }
          }
        }
        else if (parameter == on) {
          // First and second objects
          JsonObject& object0 = root[0];
          JsonObject& object1 = root[1];

          // check that both are successful
          isSuccessful = object0.containsKey("success");
          isSuccessful = object1.containsKey("success");

          // first key (success)
          for (JsonPair& object0pair0 : object0) {

              // set up new object for that key
              JsonObject& object0pair0SubObject = object0[object0pair0.key];

              // second key
              for (JsonPair& object0pair1 : object0pair0SubObject) {

                  // confirm it's been turned on
                  if (object0pair1.value == true)
                  {
                      std::cout << "switched on";
                  }
                  else {
                    isSuccessful = false;
                  }
              }
          }

          // first key (success)
          for (JsonPair& object1pair0 : object1) {

              // set up new object for that key
              JsonObject& object1pair0SubObject = object1[object1pair0.key];

              // second key
              for (JsonPair& object1pair1 : object1pair0SubObject) {
                  // contains brightness
                  if (strstr(object1pair1.key, "bri") != NULL) {
                      std::cout << " at " << object1pair1.value << " brightness";

                      // if not at full brightness
                      if (object1pair1.value != 254) {
                          isSuccessful = false;
                      }
                  }
              }
          }
        }
        else if (parameter == off) {
          // First object
          JsonObject& object0 = root[0];

          // check that it's successful
          isSuccessful = object0.containsKey("success");

          // first key (success)
          for (JsonPair& object0pair0 : object0) {

              // set up new object for that key
              JsonObject& object0pair0SubObject = object0[object0pair0.key];

              // second key
              for (JsonPair& object0pair1 : object0pair0SubObject) {

                  // confirm it's been turned off
                  if (object0pair1.value == false)
                  {
                      std::cout << "switched off";
                  }
                  else {
                    isSuccessful = false;
                  }
              }
          }
        }

        if (isSuccessful) {
            std::cout << ": successful." << std::endl;
        }
        else {
            std::cout << ": failed." << std::endl;
        }
    }
}

void dillonOn(void){
    switchLamp(dillon, on);
};
void dillonOff(void){
    switchLamp(dillon, off);
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
    switchLamp(sara, on);
};
void saraOff(void){
    switchLamp(sara, off);
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
    switchLamp(both, on);
};
void bothOff(void){
    switchLamp(both, off);
};
