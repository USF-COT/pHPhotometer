#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "Arduino.h"

class Calibration{
  private:
    char name[5];
    
    int order;
    float parameters[4];  // 3rd order calibration limit
  public:
    Calibration(char* name);
    ~Calibration();
    boolean load();
    float adjustReading(float value);
};

#endif
