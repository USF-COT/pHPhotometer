/* Photometer - A library for controlling LEDs and light voltage converters
 * to sample solution characteristics.
 *
 * By: Michael Lindemuth
 * University of South Florida
 * College of Marine Science
 */
 
#ifndef PHOTOMETER_H
#define PHOTOMETER_H

#include <Arduino.h>

struct PHOTOREADING{
  int x;
  int y;
};

struct ABSREADING{
  int A1;
  int A2;
  int R;
};

class Photometer{
  private:
    byte xLightPin, yLightPin, detectorPin;
    
    PHOTOREADING blank, sample;
    ABSREADING absReading;
    
  
  public:
    Photometer(byte xLightPin, byte yLightPin, byte detectorPin);
    ~Photometer();
    void takeBlank();
    void takeSample();
    
    void getBlank(PHOTOREADING* dest);
    void getSample(PHOTOREADING* dest);
    void getAbsorbance(ABSREADING* dest);
};
    
#endif
