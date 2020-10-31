/****************** Disney Ball Scanner ******************
 *
 * config_server.h
 *
*********************************************************/

#include <Arduino.h>

class ConfigServer {
public:
    static ConfigServer& getConfigServer();

    void startServer();

private:
    ConfigServer() = default;
    ~ConfigServer() = default;
};