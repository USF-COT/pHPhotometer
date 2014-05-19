#ifndef LINEARCALIBRATION_H
#define LINEARCALIBRATION_H

#include <Arduino.h>

struct CALIBRATIONPOINT{
  float x;
  float y;
};

class LinearCalibration{
  private:
    CALIBRATIONPOINT minimum;
    CALIBRATIONPOINT maximum;
    float m;
    float b;
    
  public:
    LinearCalibration(CALIBRATIONPOINT minimum, CALIBRATIONPOINT maximum);
    ~LinearCalibration();
    
    void save(Print* printer);
    //static LinearCalibration load(Stream* stream);
    float correct(float reading);
};

#endif
