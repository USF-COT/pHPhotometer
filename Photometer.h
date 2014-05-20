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

typedef void (* PinControlFunPtr) (int);
typedef int (* DetectorReadFunPtr) ();

struct PHOTOREADING{
  int x;
  int y;
};

struct ABSREADING{
  float A1;
  float A2;
  float R;
};

class Photometer{
  private:
    PinControlFunPtr xLightControl, yLightControl;
    DetectorReadFunPtr detectorRead;
    
    PHOTOREADING blank, sample;
    ABSREADING absReading;
    
  
  public:
    Photometer(PinControlFunPtr xLightControl, PinControlFunPtr yLightControl, DetectorReadFunPtr detectorRead);
    ~Photometer();
    
    void takeBlank();
    void takeSample();
    
    void getBlank(PHOTOREADING* dest);
    void getSample(PHOTOREADING* dest);
    void getAbsorbance(ABSREADING* dest);
};
    
#endif
