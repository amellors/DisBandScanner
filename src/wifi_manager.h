/****************** Disney Ball Scanner ******************
 *
 * wifi_manager.h
 *
*********************************************************/

#ifndef _WIFI_MANAGER_H
#define _WIFI_MAANGER_H

#include <Arduino.h>

class WifiManager
{
public:
    WifiManager() = default;
    ~WifiManager() = default;

    void process_setup(bool forceSetup);
    void process_loop();

private:
    void int_check_WiFi();
    uint8_t int_connectMultiWiFi();
    void int_toggleLED();
    void int_heartBeatPrint();
    void int_startServices();

    bool m_services_not_started;
};

#endif //_WIFI_MANAGER_H