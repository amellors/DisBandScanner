/****************** Disney Ball Scanner ******************
 *
 * config_manager.h
 *
*********************************************************/

#ifndef _CONFIG_MANAGER_H
#define _CONFIG_MANAGER_H

#include <Arduino.h>

class ConfigManager
{
public:
    static ConfigManager &getManager();

    void process_setup();

    class ConfigSerilizer {
        public:
            virtual ~ConfigSerilizer();

    };

    bool readData(const String& path, char *data, size_t length);
    bool writeData(const String& path, const char *data, const size_t length);

    
    bool readData(const String& path);
    bool writeData(const String& path);

private:
    ConfigManager() = default;
    ~ConfigManager() = default;

};

#endif
