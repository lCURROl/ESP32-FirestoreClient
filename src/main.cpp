#include <Arduino.h>
#include <Network.h>

Network *network;

// Functions declarations
void initNetwork();

// SETUP
void setup()
{
  Serial.begin(115200);
  initNetwork();
}

// LOOP
void loop()
{
  network->firestoreDataUpdate();
  delay(5000);
}

// Functions
void initNetwork()
{
  network = new Network();
  network->initWiFi();
}
