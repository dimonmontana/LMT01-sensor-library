#include "lmt01.h"

LMT01 lmt;

ISR(ANALOG_COMP_vect)
{
  // // switch on debug LED to measure ISR time
  PORTB |= (1<<5);
  lmt.ISRCorrection();
  PORTB &= ~(1<<5);
}

void setup() {
 // initialize serial communications at 9600 bps:
 Serial.begin(9600);
 // using pin 12 to power on LMT01
 lmt.setup(12);
// LMT01Instance.SetContinousMode();
 lmt.setSingleMode();
}

void loop() {
 Serial.print("LMT01 Temperature = ");
 Serial.print( lmt.getSingleMeasure() );
 Serial.println(" `C");
 delay(1000);
}
