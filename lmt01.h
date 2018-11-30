#include "Arduino.h"

class LMT01 {
    int8_t power_pin;
    bool continous_mode = true; // continous or single measure mode
  public:
    LMT01();

    int16_t  TempISRCount; // counter used in ISR
    int16_t  TemperatureCounts;  // temperature in LMT01 counts
    unsigned long milliseconds; // time in milliseconds of previous pulse

    bool setup(int pin);
    bool setup(void);
    bool setContinousMode(void);
    bool setSingleMode(void);
    void ISRCorrection(void);
    float getSingleMeasure(void);
  private:
    bool setMode(bool mode);
    float performSingleMeasure(void);
    float getTemperature(void);
};
