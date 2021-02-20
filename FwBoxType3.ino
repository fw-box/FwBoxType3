//
// Copyright (c) 2020 Fw-Box (https://fw-box.com)
// Author: Hartman Hsieh
//
// Description :
//   None
//
// Connections :
//
// Required Library :
//

#include "FwBox.h"

#define DEVICE_TYPE 3
#define FIRMWARE_VERSION "1.1.2"

#define PIN_ON_OFF 12
#define PIN_LED 13
#define PIN_BUTTON 0

#define VAL_INDEX_ON_OFF 0

//
// Debug definitions
//
#define FW_BOX_DEBUG 0

#if FW_BOX_DEBUG == 1
  #define DBG_PRINT(VAL) Serial.print(VAL)
  #define DBG_PRINTLN(VAL) Serial.println(VAL)
  #define DBG_PRINTF(FORMAT, ARG) Serial.printf(FORMAT, ARG)
  #define DBG_PRINTF2(FORMAT, ARG1, ARG2) Serial.printf(FORMAT, ARG1, ARG2)
#else
  #define DBG_PRINT(VAL)
  #define DBG_PRINTLN(VAL)
  #define DBG_PRINTF(FORMAT, ARG)
  #define DBG_PRINTF2(FORMAT, ARG1, ARG2)
#endif

//
// Function definitions
//
void ICACHE_RAM_ATTR onButtonPressed();

//
// Global variable
//
bool FlagButtonPressed = false;

void setup()
{
  Serial.begin(9600);

  //
  // Initialize the fw-box core (early stage)
  //
  fbEarlyBegin(DEVICE_TYPE, FIRMWARE_VERSION);
  FwBoxIns.setGpioStatusLed(PIN_LED);

  pinMode(PIN_ON_OFF, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);

  //
  // Initialize the fw-box core
  //
  fbBegin(DEVICE_TYPE, FIRMWARE_VERSION);

  //
  // Setup MQTT subscribe callback
  //
  setRcvValueCallback(onReceiveValue);

  //
  // Attach interrupt for button
  //
  attachInterrupt(PIN_BUTTON, onButtonPressed, FALLING); //assign int0

} // void setup()

void loop()
{
  if(FlagButtonPressed == true) {
    //
    // Toggle the GPIO status of relay
    //
    if(digitalRead(PIN_ON_OFF) == 0)
      digitalWrite(PIN_ON_OFF, 1);
    else
      digitalWrite(PIN_ON_OFF, 0);

    DBG_PRINT("digitalRead(PIN_ON_OFF)=");
    DBG_PRINTLN(digitalRead(PIN_ON_OFF));

    if(digitalRead(PIN_ON_OFF) == 0) {
      FwBoxIns.mqttPublish(VAL_INDEX_ON_OFF, "OFF");

      //
      // Sync the status to LED
      //
      digitalWrite(PIN_LED, HIGH); // This value is reverse
    }
    else {
      FwBoxIns.mqttPublish(VAL_INDEX_ON_OFF, "ON");

      //
      // Sync the status to LED
      //
      digitalWrite(PIN_LED, LOW); // This value is reverse
    }

    //
    // Reset the flag
    //
    FlagButtonPressed = false;
  }

  FwBoxIns.setValue(VAL_INDEX_ON_OFF, digitalRead(PIN_ON_OFF));

  //
  // Run the handle
  //
  fbHandle();

} // END OF "void loop()"

void onReceiveValue(int valueIndex, String* payload)
{
  DBG_PRINT("onReceiveValue valueIndex = ");
  DBG_PRINTLN(valueIndex);
  DBG_PRINT("onReceiveValue *payload = ");
  DBG_PRINTLN(*payload);

  if(valueIndex == 0) { // Relay
    payload->toUpperCase();
    if(payload->equals("ON") == true)
    {
      digitalWrite(PIN_ON_OFF, HIGH);
      FwBoxIns.mqttPublish(valueIndex, "ON"); // Sync the status to MQTT server

      //
      // Sync the status to LED
      //
      digitalWrite(PIN_LED, LOW); // This value is reverse
    }
    else
    {
      digitalWrite(PIN_ON_OFF, LOW);
      FwBoxIns.mqttPublish(valueIndex, "OFF"); // Sync the status to MQTT server

      //
      // Sync the status to LED
      //
      digitalWrite(PIN_LED, HIGH); // This value is reverse
    }
  }
}

void ICACHE_RAM_ATTR onButtonPressed()
{
  static unsigned long previous_pressed_time = 0;
  if((millis() - previous_pressed_time) > 900) { // Debouncing
    FlagButtonPressed = true;
    previous_pressed_time = millis();
  }
}
