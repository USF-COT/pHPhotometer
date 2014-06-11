#include "Arduino.h"
#include "Calibration.h"
#include <math.h>

Calibration::Calibration(){
}

Calibration::~Calibration(){
}

const unsigned int LINE_SIZE = 64;
boolean Calibration::load(){
  Serial.println("Hello!");
  return true;
  /*
  char filename[16];
  strncpy(filename, this->name, 4);
  strncat(filename, "CAL.CSV", 6);
  Serial.println(filename);
  
  char buffer[LINE_SIZE];
  ifstream sdin(filename);
  if(!sdin.is_open()){
    Serial.println("Unable to open calibration file");
    this->order = 0;
    return false;
  }
  
  char comma;
  sdin >> this->order;
  for(byte i=0; i <= this->order; ++i){
    sdin >> comma;
    sdin >> this->parameters[i];
  }
  
  return true;
  */
}

float Calibration::adjustReading(float reading){
  float calibratedValue = 0;
  for(byte i=0; i <= this->order; ++i){
    calibratedValue += this->parameters[i] * pow(reading, (float)this->order-i);
  }
  return calibratedValue;
}

