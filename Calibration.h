#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "Arduino.h"

class Calibration{
  private:
    String name;
    
    byte order;
    float parameters[4];  // 3rd order calibration limit
  public:
    Calibration();
    ~Calibration();
    boolean load();
    float adjustReading(float value);
};

#endif
