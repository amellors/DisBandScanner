/****************** Disney Ball Scanner ******************
 *
 * filesystem_manager.cpp
 *
*********************************************************/

#include "filesystem_manager.h"


#include <SPIFFS.h>
#define FileFS          SPIFFS

#define GLOBAL_ForceFormat     false

// static
FSManager &
FSManager::getManager() 
{
    static FSManager instance;
    return instance;
}

void
FSManager::process_setup()
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
FSManager::readData(const String& path, char *data, size_t length)
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
FSManager::writeData(const String& path, const char *data, const size_t length)
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
FSManager::readData(const String& path)
{
    
    return false;
}

bool
FSManager::writeData(const String& path)
{
    
    return false;
}
