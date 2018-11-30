#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include "lmt01.h"

#define COUNTER_TIMEOUT  50

LMT01::LMT01() {}

bool LMT01::setMode(bool mode)
{
  if ( power_pin != -1 ) {
    continous_mode = mode;
    if ( continous_mode ) {
      // power on LMT01
      digitalWrite(power_pin, HIGH);
    }else{
      // power off LMT01
      digitalWrite(power_pin, LOW);
    }
  }else{
    continous_mode = true;
  }
  return (continous_mode == mode);
}

float LMT01::performSingleMeasure()
{
  // clear ISR counter
  noInterrupts();
  TempISRCount = 0;
  interrupts();

  // power on LMT01
  digitalWrite(power_pin, HIGH);

  // wait for conversion to complete 55ms max
  delay(55);

  int current_time = 0;
  int counts = -1;
  int old_counts;

  // wait for stop counting
  do {
    old_counts = counts;
    noInterrupts();
    counts = TempISRCount;
    interrupts();
    delay(5);
    current_time++;
  } while( counts != old_counts && current_time < 11);

  noInterrupts();
  TemperatureCounts = TempISRCount;
  interrupts();

  // power off LMT01
  digitalWrite(power_pin, LOW);
  return getTemperature();
}

// you can optimize it for speed by using fixed instead of floating point arithmetics
// to improve accuracy you can use T = -0.00000049*counts^2 + 0.0642*counts9 - 51.6
float LMT01::getTemperature()
{
    return (TemperatureCounts - 808)*0.0625;
}

// setup sensor using dedicated pin to power LMT01
// returns false in failure to use pin as power to LMT01
bool LMT01::setup(int pin)
{
 // set up Comparator module
 ACSR =
   (0 << ACD) |    // Analog Comparator: Enabled
   (1 << ACBG) |   // Analog Comparator Bandgap Select: 1.25V REF is applied to the positive input
   (0 << ACO) |    // Analog Comparator Output: Off
   (1 << ACI) |    // Analog Comparator Interrupt Flag: Clear Pending Interrupt
   (1 << ACIE) |   // Analog Comparator Interrupt: Enabled
   (0 << ACIC) |   // Analog Comparator Input Capture: (1 << ACIS0);   // Analog Comparator Interrupt Mode: Comparator Interrupt on Rising Output Edge
   (1 << ACIS1) | (1 << ACIS0);   // Analog Comparator Interrupt Mode: Comparator Interrupt on Rising Output Edge
 ADCSRB &= ~(1 << ACME);    // Using AIN1 as negative input

 // it's better to check pin value for errors
 power_pin = pin;
 pinMode(power_pin, OUTPUT);
 digitalWrite(power_pin, LOW);

 // default mode of conversion
 continous_mode = true;
 return true;
}

bool LMT01::setup()
{
 // set up Comparator module
 ACSR =
   (0 << ACD) |    // Analog Comparator: Enabled
   (1 << ACBG) |   // Analog Comparator Bandgap Select: 1.25V REF is applied to the positive input
   (0 << ACO) |    // Analog Comparator Output: Off
   (1 << ACI) |    // Analog Comparator Interrupt Flag: Clear Pending Interrupt
   (1 << ACIE) |   // Analog Comparator Interrupt: Enabled
   (0 << ACIC) |   // Analog Comparator Input Capture: Disabled
   (1 << ACIS1) | (1 << ACIS0);   // Analog Comparator Interrupt Mode: Comparator Interrupt on Rising Output Edge
 ADCSRB &= ~(1 << ACME);    // Using AIN1 as negative input

 // default mode of conversion
 continous_mode = true;
 return true;
}

bool LMT01::setContinousMode()
{
   return LMT01::setMode(true);
}

bool LMT01::setSingleMode()
{
   return LMT01::setMode(false);
}

float LMT01::getSingleMeasure()
{
  if ( continous_mode ) {
    // in contonous mode data is already available
    return LMT01::getTemperature();
  }
  return LMT01::performSingleMeasure();
}

void LMT01::ISRCorrection(){

  if ( continous_mode ) {
    unsigned long current = millis();
    unsigned long difference = current - milliseconds;
    if ( difference > COUNTER_TIMEOUT ) {
        // previous pulse was too long ago, lets start counting new measure
        TemperatureCounts = TempISRCount;
        TempISRCount = 1;
    }else{
        // just counting
        TempISRCount++;
    }
    // store current time
    milliseconds = current;
    // switch off debug LED
  }else{
    // just counting - class LMT01 handles 50ms timeout
    TempISRCount++;
  }
}
