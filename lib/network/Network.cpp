#include "Network.h"
#include "addons/TokenHelper.h"

#include "Network.h"
#include "addons/TokenHelper.h"

#define WIFI_SSID "<YOUR_WIFI_SSID>"
#define WIFI_PASSWORD "<YOUR_WIFI_PASSWORD>"

#define API_KEY "<YOUR_WEB_API_KEY>"
#define FIREBASE_PROJECT_ID "<YOUR_PROJECT_ID>"
#define USER_EMAIL "<YOUR_USER_EMAIL>"
#define USER_PASSWORD "<YOUR_USER_PASSWORD>"

static Network *instance = NULL;

Network::Network()
{
    instance = this;
    instance->hasIP = false;
    instance->count = 0.0;
}

void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WIFI CONNECTED! BUT WAIT FOR THE LOCAL IP ADDR");
}

void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("LOCAL IP ADDRESS: ");
    Serial.println(WiFi.localIP());
    // hasIP flag to TRUE to init Firebase but dont call init here (stack error)
    instance->hasIP = true;
}

void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    // hasIP flag to FALSE
    instance->hasIP = false;
    Serial.println("WIFI DISCONNECTED!");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void FirestoreTokenStatusCallback(TokenInfo info)
{
    String tokenType = getTokenType(info);
    String tokenStatus = getTokenStatus(info);
    Serial.printf("Token Info: type = %s, status = %s\n", tokenType.c_str(), tokenStatus.c_str());
}

void Network::initWiFi()
{
    WiFi.disconnect();
    WiFi.onEvent(WiFiEventConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiEventGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiEventDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void Network::firebaseInit()
{
    Serial.println("Initializing Firebase");
    config.api_key = API_KEY;

    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    config.token_status_callback = FirestoreTokenStatusCallback;

    // fbdo.setBSSLBufferSize(4096, 1024);
    // fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
}

void Network::firestoreDataUpdate()
{
    instance->count += 0.5;
    if (!Firebase.ready() && instance->hasIP)
    {
        // If firebase not initializec and have ip -> Initialize firebase
        instance->firebaseInit();
    }
    else if (WiFi.status() == WL_CONNECTED && Firebase.ready())
    {
        Serial.println("Uploading data");
        String documentPath = "test/b" + String(1);

        FirebaseJson content;

        content.set("fields/myDouble/doubleValue", instance->count);

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "myDouble"))
        {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            return;
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }

        if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw()))
        {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            return;
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
    }
    else
    {

        Serial.println("Cant upload data");
    }
}