#include "ECTShield.h"

ECTShield::ECTShield(ECProbeControlFun probeControl, ECProbeReadFun readProbe, byte oneWirePin):ds(oneWirePin){
  this->probeControl = probeControl;
  this->readProbe = readProbe;
}

ECTShield::~ECTShield(){
}

float ECTShield::getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius
  byte data[12];
  byte addr[8];

  if ( !this->ds.search(addr)) {
      //no more sensors on chain, reset search
      this->ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  this->ds.reset();
  this->ds.select(addr);
  this->ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = this->ds.reset();
  this->ds.select(addr);    
  this->ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = this->ds.read();
  }
  
  this->ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
}

boolean ECTShield::calibrate(float temperature, float salinity){
  
}
boolean ECTShield::takeReading(ECTReading* reading){
  reading->temperature = this->getTemp();
}
