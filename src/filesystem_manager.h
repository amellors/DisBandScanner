/****************** Disney Ball Scanner ******************
 *
 * filesystem_manager.h
 *
*********************************************************/

#ifndef _FILESYSTEM_MANAGER_H
#define _FILESYSTEM_MANAGER_H

#include <Arduino.h>

class FSManager
{
public:
    static FSManager &getManager();

    void process_setup();

    class FileSystemSerilizer {
        public:
            virtual ~FileSystemSerilizer();

    };

    bool readData(const String& path, char *data, size_t length);
    bool writeData(const String& path, const char *data, const size_t length);

    
    bool readData(const String& path);
    bool writeData(const String& path);

private:
    FSManager() = default;
    ~FSManager() = default;
};

#endif
