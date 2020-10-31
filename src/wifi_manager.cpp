/****************** Disney Ball Scanner ******************
 *
 * wifi_manager.cpp
 *
*********************************************************/

#include "wifi_manager.h"

#include <Arduino.h>

#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <FS.h>

// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#define _WIFIMGR_LOGLEVEL_    1

#define USING_CORS_FEATURE          true
#include <ESP_WiFiManager.h>
#include <ESP_WiFiManager_Debug.h>

// From v1.1.0
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

// For Config Portal
// SSID and PW for Config Portal
String ssid = "DisBall_" + String(ESP_getChipId(), HEX);
const char* password = "disBallPass";
// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

#define MIN_AP_PASSWORD_SIZE    8
#define SSID_MAX_LEN            32
//From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN            64
typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

typedef struct
{
  String wifi_ssid;
  String wifi_pw;
}  WiFi_Credentials_String;

#define NUM_WIFI_CREDENTIALS      2

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
} WM_Config;

WM_Config         WM_config;

#define  CONFIG_FILENAME              F("/wifi_cred.dat")

#define LED_BUILTIN       2
#define LED_ON            HIGH
#define LED_OFF           LOW
#define USE_DHCP_IP     true

#include "config_manager.h"

/*static*/
WifiManager &
WifiManager::getManager()
{
  static WifiManager instance;
  return instance;
}

void loadConfigData(void)
{
  ConfigManager::getManager().readData(CONFIG_FILENAME, (char *)&WM_config, sizeof(WM_config));
}
    
void saveConfigData(void)
{
  ConfigManager::getManager().writeData(CONFIG_FILENAME, (char *)&WM_config, sizeof(WM_config));
}

static void wifi_manager(bool forceSetup)
{
    Serial.println(F("\nConfig Portal requested."));
  digitalWrite(LED_BUILTIN, LED_ON); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.

  //Local intialization. Once its business is done, there is no need to keep it around
  ESP_WiFiManager ESP_wifiManager("Dis_Ball");

  //Check if there is stored WiFi router/password credentials.
  //If not found, device will remain in configuration mode until switched off via webserver.
  Serial.print(F("Opening Configuration Portal. "));
  
  Router_SSID = ESP_wifiManager.WiFi_SSID();
  Router_Pass = ESP_wifiManager.WiFi_Pass();
  
  // From v1.1.1, Don't permit NULL password
  if ( !forceSetup && (Router_SSID != "") && (Router_Pass != "") )
  {
    //If valid AP credential and not DRD, set timeout 120s.
    ESP_wifiManager.setConfigPortalTimeout(120);
    Serial.println("Got stored Credentials. Timeout 120s");
  }
  else
  {
    ESP_wifiManager.setConfigPortalTimeout(0);

    Serial.print(F("No timeout : "));
    
    if (forceSetup)
    {
      Serial.println(F("DRD or No stored Credentials.."));
    }
    else
    {
      Serial.println(F("No stored Credentials."));
    }
  }

  // Sets timeout in seconds until configuration portal gets turned off.
  // If not specified device will remain in configuration mode until
  // switched off via webserver or device is restarted.
  //ESP_wifiManager.setConfigPortalTimeout(120);

  ESP_wifiManager.setMinimumSignalQuality(-1);

  // From v1.0.10 only
  // Set config portal channel, default = 1. Use 0 => random channel from 1-13
  ESP_wifiManager.setConfigPortalChannel(0);
  //////
  
  //set custom ip for portal
  //ESP_wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 0));
  
  // New from v1.1.
  ESP_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");

  // Start an access point
  // and goes into a blocking loop awaiting configuration.
  // Once the user leaves the portal with the exit button
  // processing will continue
  // SSID to uppercase
  ssid.toUpperCase();
  
  if (!ESP_wifiManager.startConfigPortal((const char *) ssid.c_str(), password))
  {
    Serial.println(F("Not connected to WiFi but continuing anyway."));
  }
  else
  {
    // If you get here you have connected to the WiFi
    Serial.println(F("Connected...yeey :)"));
    Serial.print(F("Local IP: "));
    Serial.println(WiFi.localIP());
  }

  // Only clear then save data if CP entered and with new valid Credentials
  // No CP => stored getSSID() = ""
  if ( String(ESP_wifiManager.getSSID(0)) != "" || String(ESP_wifiManager.getSSID(1)) != "" )
  {
    Serial.println(F("Attempting to save entered credentials: "));
    // Stored  for later usage, from v1.1.0, but clear first
    memset(&WM_config, 0, sizeof(WM_config));
    
    bool credentialsToSave = false;
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
        String tempSSID = ESP_wifiManager.getSSID(i);
        String tempPW   = ESP_wifiManager.getPW(i);
  
        // Only save & Add if SSID != null and Password is of valid size (> 8)
        if (tempSSID != "" && strlen(tempPW.c_str()) >= MIN_AP_PASSWORD_SIZE) {
            credentialsToSave = true;
            LOGERROR2(F("SSID = "),  WM_config.WiFi_Creds[i].wifi_ssid, " is ok to save");

            if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
                strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
            else
                strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);
        
            if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
                strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
            else
                strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);  
        
            LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
            wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
        }
    }
    
    if (credentialsToSave) {
        saveConfigData();
    }
  }

  // Writing JSON config file to flash for next boot
  // writeConfigFile();

  digitalWrite(LED_BUILTIN, LED_OFF); // Turn LED off as we are not in configuration mode.
}

// ***** WiFiManager Public *****

void WifiManager::process_setup(bool forceSetup)
{
  if (forceSetup)
  {
    wifi_manager(true);
  }
  else
  {   
    // Load stored data, the addAP ready for MultiWiFi reconnection
    loadConfigData();

    // Pretend CP is necessary as we have no AP Credentials
    bool initialConfig = true;

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR2(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = ******") );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
        initialConfig = false;
      }
    }

    if (initialConfig)
    {
      Serial.println(F("Open Config Portal without Timeout: No stored WiFi Credentials"));
    
      wifi_manager(false);
    }
    else if ( WiFi.status() != WL_CONNECTED ) 
    {
      Serial.println("ConnectMultiWiFi in setup");
     
      int_connectMultiWiFi();
    }
  }

  digitalWrite(LED_BUILTIN, LED_OFF); // Turn led off as we are not in configuration mode.   

    m_services_not_started = true;
}

#define WIFICHECK_INTERVAL    1000L
#define LED_INTERVAL          2000L
#define HEARTBEAT_INTERVAL    10000L
#define PUBLISH_INTERVAL      60000L
void WifiManager::process_loop()
{
  static ulong checkstatus_timeout  = 0;
  static ulong LEDstatus_timeout    = 0;
  static ulong checkwifi_timeout    = 0;
  
  ulong current_millis = millis();


  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
  {
    int_check_WiFi();
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }

  if ((current_millis > LEDstatus_timeout) || (LEDstatus_timeout == 0))
  {
    // Toggle LED at LED_INTERVAL = 2s
    int_toggleLED();
    LEDstatus_timeout = current_millis + LED_INTERVAL;
  }

  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0))
  { 
    int_heartBeatPrint();
    checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
  }

  if (WiFi.status() == WL_CONNECTED && m_services_not_started) 
  {
    int_startServices();
  }
}

// ***** WiFiManager Private *****

void WifiManager::int_check_WiFi()
{
    if ( (WiFi.status() != WL_CONNECTED) )
    {
        Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
        int_connectMultiWiFi();
    }
}

uint8_t WifiManager::int_connectMultiWiFi()
{

#if ESP32
  // For ESP32, this better be 0 to shorten the connect time
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS       0
#else
  // For ESP8266, this better be 2200 to enable connect the 1st time
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS       2200L
#endif

#define WIFI_MULTI_CONNECT_WAITING_MS           100L
  
  uint8_t status;

  LOGERROR(F("ConnectMultiWiFi with :"));
  
  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
    {
      LOGERROR2(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = ***** "));
    }
  }
  
  LOGERROR(F("Connecting MultiWifi..."));

  WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP    
  #if USE_CONFIGURABLE_DNS  
    // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
    WiFi.config(stationIP, gatewayIP, netMask, dns1IP, dns2IP);  
  #else
    // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
    WiFi.config(stationIP, gatewayIP, netMask);
  #endif 
#endif

  int i = 0;
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ( ( i++ < 20 ) && ( status != WL_CONNECTED ) )
  {
    status = wifiMulti.run();

    if ( status == WL_CONNECTED )
      break;
    else
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
  }

  if ( status == WL_CONNECTED )
  {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
  }
  else
    LOGERROR(F("WiFi not connected"));

  return status;
}
    
void WifiManager::int_toggleLED()
{
  //toggle state
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void WifiManager::int_heartBeatPrint()
{
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
    Serial.print(F("W"));        // W means connected to WiFi
  else
    Serial.print(F("N"));        // N means not connected to WiFi

  if (num == 40)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 5 == 0)
  {
    Serial.print(F(" "));
  }
}

void WifiManager::int_startServices() {
    m_services_not_started = false;
}