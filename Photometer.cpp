#include "Photometer.h"

Photometer::Photometer(byte xLightPin, byte yLightPin, byte detectorPin){
  this->xLightPin = xLightPin;
  this->yLightPin = yLightPin;
  this->detectorPin = detectorPin;
  
  pinMode(this->xLightPin, OUTPUT);
  pinMode(this->yLightPin, OUTPUT);
  
  this->sample.x = this->sample.y = 0;
  this->blank.x = this->blank.y = 0;
}

Photometer::~Photometer(){
}

int averageSample(byte lightPin, byte detectorPin){
  const int NUM_SAMPLES = 10;
  const int LIGHT_DELAY = 2000;
  
  digitalWrite(lightPin, HIGH);
  delay(LIGHT_DELAY);
  
  int sample = 0;
  for(unsigned short i=0; i < NUM_SAMPLES; ++i){
    sample += analogRead(detectorPin);
  }
  return sample /= NUM_SAMPLES;
  
  digitalWrite(lightPin, LOW);
}

void Photometer::takeBlank(){
  this->blank.x = averageSample(this->xLightPin, this->detectorPin);
  this->blank.y = averageSample(this->yLightPin, this->detectorPin);
}

void Photometer::takeSample(){
  this->sample.x = averageSample(this->xLightPin, this->detectorPin);
  this->sample.y = averageSample(this->yLightPin, this->detectorPin);

  this->absReading.A1 = log((float)this->blank.x/(float)this->sample.x)/(log(10));//calculate the absorbance
  this->absReading.A2 = log((float)this->blank.y/(float)this->sample.y)/(log(10));//calculate the absorbance
  this->absReading.R=(float)this->absReading.A2/(float)this->absReading.A1;
}
    
void Photometer::getBlank(PHOTOREADING* dest){
  dest->x = this->blank.x;
  dest->y = this->blank.y;
}
void Photometer::getSample(PHOTOREADING* dest){
  dest->x = this->sample.x;
  dest->y = this->sample.y;
}

void Photometer::getAbsorbance(ABSREADING* dest){
  dest->A1 = this->absReading.A1;
  dest->A2 = this->absReading.A2;
  dest->R = this->absReading.R;
}
