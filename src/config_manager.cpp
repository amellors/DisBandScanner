/****************** Disney Ball Scanner ******************
 *
 * config_manager.cpp
 *
*********************************************************/

#include "config_manager.h"


#include <SPIFFS.h>
#define FileFS          SPIFFS

#define GLOBAL_ForceFormat     false

// static
ConfigManager &
ConfigManager::getManager() 
{
    static ConfigManager instance;
    return instance;
}

void
ConfigManager::process_setup()
{

    if(GLOBAL_ForceFormat) {
        Serial.println(F("Forced Formatting."));
        FileFS.format();
    }

    // Format FileFS if not yet
    if (!FileFS.begin(true)) 
    {
        Serial.print(F("FS Initilize failed - AutoFormatting."));
    }
}

bool
ConfigManager::readData(const String& path, char *data, size_t length)
{
    File file = FileFS.open(path, "r");
    
    if (file)
    {
        file.readBytes(data, length);
        file.close();

        return true;
    }

    return false;
}

bool
ConfigManager::writeData(const String& path, const char *data, const size_t length)
{
    File file = FileFS.open(path, "w");

    if (file)
    {
        file.write((uint8_t*)data, length);
        file.close();
        return true;
    }
    
    return false;
}
    
bool
ConfigManager::readData(const String& path)
{
    
    return false;
}

bool
ConfigManager::writeData(const String& path)
{
    
    return false;
}
