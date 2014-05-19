#ifndef ECTSHIELD_H
#define ECTSHIELD_H

#include <Arduino.h>
#include <OneWire.h>

typedef void (* ECProbeControlFun)(int);
typedef int (* ECProbeReadFun)();

struct ECTReading{
  float conductivity;
  float temperature;
  float salinity;
};

class ECTShield{
  private:
    OneWire ds;
    ECProbeControlFun probeControl;
    ECProbeReadFun readProbe;
    
  public:
    ECTShield(ECProbeControlFun probeControl, ECProbeReadFun readProbe, byte oneWirePin);
    ~ECTShield();
    
    float getTemp();
    boolean calibrate(float temperature, float salinity);
    boolean takeReading(ECTReading* reading);
};


#endif
