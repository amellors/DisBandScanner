/****************** Disney Ball Scanner ******************
 *
 * nfc_manager.h
 *
*********************************************************/

#include <Arduino.h>

class NfcManager
{
public:
    static NfcManager &getManager();
    void process_setup();

};
