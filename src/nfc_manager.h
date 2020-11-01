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
    void process_loop();

private:
    uint8_t m_scanned_uid[7];  // Buffer to store the returned UID
    uint8_t m_scanned_uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
    ulong   m_nfc_scan_timeout = 0;
    bool    m_nfc_scanned = false;
    ulong   m_nfc_scan_pause;
};
