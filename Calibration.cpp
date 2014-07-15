#include "Arduino.h"
#include "Calibration.h"
#include <SdFat.h>
#include <math.h>

Calibration::Calibration(char* name){
  strncpy(this->name, name, 5);
}

Calibration::~Calibration(){
}

const unsigned int LINE_SIZE = 64;
boolean Calibration::load(){  
  char filename[16];
  strncpy(filename, this->name, 5);
  strncat(filename, "cal.csv", 7);
  Serial.println(filename);
  
  char buffer[LINE_SIZE];
  ifstream sdin(filename);
  if(!sdin.is_open()){
    this->order = 0;
    return false;
  }
  
  char comma;
  sdin >> this->order;
  Serial.println(this->order);
  for(int i=0; i <= this->order; ++i){
    sdin >> comma;
    sdin >> this->parameters[i];
    Serial.println(this->parameters[i]);
  }
  
  return true;
}

float Calibration::adjustReading(float reading){
  float calibratedValue = 0;
  for(int i=0; i <= this->order; ++i){
    calibratedValue += this->parameters[i] * pow(reading, (float)this->order-i);
  }
  return calibratedValue;
}

