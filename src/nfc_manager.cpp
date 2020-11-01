/****************** Disney Ball Scanner ******************
 *
 * nfc_manager.cpp
 *
*********************************************************/

#include "nfc_manager.h"

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SS   (4)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (21)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a SPI connection:
// Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
Adafruit_PN532 nfc(PN532_SS);

/*static*/
NfcManager &
NfcManager::getManager()
{
    static NfcManager instance;
    return instance;
}

void
NfcManager::process_setup()
{
    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (! versiondata) {
        Serial.print("Didn't find PN53x board");
        while (1); // halt
    }
    // Got ok data, print it out!
    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
    Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
    Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
    
    // configure board to read RFID tags
    nfc.SAMConfig();
}

#define NFC_SCAN_PAUSE    1500L
#define NFC_SCAN_TIMEOUT  50000L

void
NfcManager::process_loop()
{
    ulong current_millis = millis();
    
    bool success = false;
    if (!m_nfc_scanned) {
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, m_scanned_uid, &m_scanned_uidLength);
    }

    if(success){
        Serial.println("Found an ISO14443A card");
        Serial.print("  UID Length: ");Serial.print(m_scanned_uidLength, DEC);Serial.println(" bytes");
        Serial.print("  UID Value: ");
        nfc.PrintHex(m_scanned_uid, m_scanned_uidLength);
        
        if (m_scanned_uidLength == 4)
        {
            // We probably have a Mifare Classic card ... 
            uint32_t cardid = m_scanned_uid[0];
            cardid <<= 8;
            cardid |= m_scanned_uid[1];
            cardid <<= 8;
            cardid |= m_scanned_uid[2];  
            cardid <<= 8;
            cardid |= m_scanned_uid[3]; 
            Serial.print("Seems to be a Mifare Classic card #");
            Serial.println(cardid);
        }
        Serial.println("");

        m_nfc_scanned = true;
        m_nfc_scan_pause = current_millis + NFC_SCAN_PAUSE;
    }

    if (m_nfc_scanned && current_millis > m_nfc_scan_pause )
    {
        m_nfc_scanned = false;
    }
}
