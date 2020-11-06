/****************** Disney Ball Scanner ******************
 *
 * main.cpp
 *
*********************************************************/
#if !( defined(ESP8266) ||  defined(ESP32) )
  #error This code is intended to run on the ESP8266 or ESP32 platform! Please check your Tools->Board setting.
#endif

#include <Arduino.h>            // for button
#include <OneButton.h>          // for button

// Now support ArduinoJson 6.0.0+ ( tested with v6.15.2 to v6.16.1 )
#include <ArduinoJson.h>        // get it from https://arduinojson.org/ or install via Arduino library manager

//For ESP32, To use ESP32 Dev Module, QIO, Flash 4MB/80MHz, Upload 921600
//Ported to ESP32
#ifdef ESP32
  #define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

  #define LED_BUILTIN       2
  #define LED_ON            HIGH
  #define LED_OFF           LOW
#endif

// These defines must be put before #include <ESP_DoubleResetDetector.h>
// to select where to store DoubleResetDetector's variable.
// For ESP32, You must select one to be true (EEPROM or SPIFFS)
// For ESP8266, You must select one to be true (RTC, EEPROM, SPIFFS or LITTLEFS)
// Otherwise, library will use default EEPROM storage
#ifdef ESP32
  // These defines must be put before #include <ESP_DoubleResetDetector.h>
  // to select where to store DoubleResetDetector's variable.
  // For ESP32, You must select one to be true (EEPROM or SPIFFS)
  // Otherwise, library will use default EEPROM storage
  #define ESP_DRD_USE_SPIFFS      true
  #define ESP_DRD_USE_EEPROM      false
#endif
 
#define DOUBLERESETDETECTOR_DEBUG       true  //false

#include <ESP_DoubleResetDetector.h>      //https://github.com/khoih-prog/ESP_DoubleResetDetector

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 3

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

DoubleResetDetector* drd = NULL;

#include "wifi_manager.h"
#include "filesystem_manager.h"
#include "nfc_manager.h"

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)

  #define VS1053_CS      32     // VS1053 chip select pin (output)
  #define VS1053_DCS     33     // VS1053 Data/command select pin (output)
  #define CARDCS         14     // Card chip select pin
  #define VS1053_DREQ    15     // VS1053 Data request, ideally an Interrupt pin

const uint16_t PixelCount = 16; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
const RgbColor CylonEyeColor(HtmlColor(0x7f0000));

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// for esp8266 omit the pin
// NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);

NeoPixelAnimator animations(2); // only ever need 2 animations

// Setup function
void setup()
{
  // Initialize the LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Put your setup code here, to run once
  Serial.begin(115200);
  while (!Serial);

  Serial.print("\nStarting DisBallScanner");
  Serial.println(" on " + String(ARDUINO_BOARD));

  Serial.setDebugOutput(false);
  
  FSManager::getManager().process_setup();
  NfcManager::getManager().process_setup();

  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);

  bool forceSetup = false;

  if (!drd)
  {
    Serial.println(F("Can't instantiate. Disable DRD feature"));
  }
  else if (drd->detectDoubleReset())
  {
    // DRD, disable timeout.
    //ESP_wifiManager.setConfigPortalTimeout(0);
    
    Serial.println(F("Open Config Portal without Timeout: Double Reset Detected"));
    forceSetup = true;
  }
  

  WifiManager::getManager().process_setup(forceSetup);
}

// Loop function
void loop()
{
  // Call the double reset detector loop method every so often,
  // so that it can recognise when the timeout expires.
  // You can also call drd.stop() when you wish to no longer
  // consider the next reset as a double reset.
  if (drd)
    drd->loop();

  WifiManager::getManager().process_loop(); 
  NfcManager::getManager().process_loop();
}
