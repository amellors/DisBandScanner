/****************** Disney Ball Scanner ******************
 *
 * scan_processor.h
 *
*********************************************************/

#include <Arduino.h>
#include <functional>

class ScanProcessor
{
public:
    ScanProcessor() = delete ;
    ScanProcessor(const String uid, std::function<void()> callback);
    ~ScanProcessor() = default;

    void start_processing();

private:
    String  m_uid;
    std::function<void ()> m_callback;
};
