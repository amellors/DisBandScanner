/****************** Disney Ball Scanner ******************
 *
 * scan_processor.cpp
 *
*********************************************************/

#include "scan_processor.h"

#include <Arduino.h>

ScanProcessor::ScanProcessor(const String uid, std::function<void()> callback)
: m_uid(uid)
, m_callback(callback)
{}

void
ScanProcessor::start_processing()
{
    Serial.println("starting processing for: " + m_uid);
    delay(5000);
    Serial.println("finished processing for: " + m_uid);
    m_callback();
}