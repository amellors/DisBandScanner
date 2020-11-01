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

volatile bool interruptOccured = false;
int numberOfInterrupts = 0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR handleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  interruptOccured = true;
  portEXIT_CRITICAL_ISR(&mux);
}

#define POLL_CMD_SIZE 4
byte POLL_CMD[POLL_CMD_SIZE];

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

    pinMode(PN532_IRQ, INPUT);
    pinMode(PN532_IRQ, INPUT_PULLUP);

    POLL_CMD[0] = PN532_COMMAND_INAUTOPOLL;
    POLL_CMD[1] = 0xFF; 
    POLL_CMD[2] = 0x0A;
    POLL_CMD[3] = 0x10;

    attachInterrupt(digitalPinToInterrupt(PN532_IRQ), handleInterrupt, FALLING);
}

#define NFC_SCAN_PAUSE    1500L
#define NFC_SCAN_TIMEOUT  50000L

void
NfcManager::process_loop()
{
    ulong current_millis = millis();
    
    if(interruptOccured){
 
        portENTER_CRITICAL(&mux);
        interruptOccured = false;
        portEXIT_CRITICAL(&mux);
    
        numberOfInterrupts++;
        Serial.print("An interrupt has occurred. Total: ");
        Serial.println(numberOfInterrupts);
        #define PN532_PACKBUFFSIZ 64
        byte read_buffer[PN532_PACKBUFFSIZ];
        nfc.readdata(read_buffer, 30);

        if (read_buffer[7] == 1) {
  
            /* ISO14443A card poll response should be in the following format:

                byte            Description
                -------------   ------------------------------------------
                b0..6           Frame header and preamble
                b7              Tags Found
                b8              Type ID
                b9              Data Lenth
                b10              Tag Number (only one used in this example)
                b11..12          SENS_RES
                b13             SEL_RES
                b14             NFCID Length
                b15..NFCIDLen   NFCID                                      */
            uint16_t sens_res = read_buffer[11];
            sens_res <<= 8;
            sens_res |= read_buffer[12];


            /* Card appears to be Mifare Classic */
            m_scanned_uidLength = read_buffer[14];

            for (uint8_t i = 0; i < read_buffer[14]; i++) {
                m_scanned_uid[i] = read_buffer[15 + i];
            }


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
    }

    if ( ((current_millis > m_nfc_scan_timeout) ||  (m_nfc_scan_timeout == 0)) || (m_nfc_scanned && current_millis > m_nfc_scan_pause) )
    {
        Serial.println("Starting Autopoll.... ");
        if (nfc.sendCommandCheckAck(POLL_CMD, POLL_CMD_SIZE)) {
        Serial.println("...... started");
        }
        m_nfc_scan_timeout = current_millis + NFC_SCAN_TIMEOUT;
        m_nfc_scan_pause = 0;
        m_nfc_scanned = false;
    }
}
