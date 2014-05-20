#include "Photometer.h"

Photometer::Photometer(PinControlFunPtr xLightControl, PinControlFunPtr yLightControl, DetectorReadFunPtr detectorRead){
  this->xLightControl = xLightControl;
  this->yLightControl = yLightControl;
  this->detectorRead = detectorRead;
  
  this->sample.x = this->sample.y = 0;
  this->blank.x = this->blank.y = 0;
}

Photometer::~Photometer(){
}

int averageSample(PinControlFunPtr lightControl, DetectorReadFunPtr detectorRead){
  const int NUM_SAMPLES = 10;
  const int LIGHT_DELAY = 2000;
  
  lightControl(HIGH);
  delay(LIGHT_DELAY);
  
  int sample = 0;
  for(unsigned short i=0; i < NUM_SAMPLES; ++i){
    sample += detectorRead();
  }
  return sample /= NUM_SAMPLES;
  
  lightControl(LOW);
}

void Photometer::takeBlank(){
  this->blank.x = averageSample(this->xLightControl, this->detectorRead);
  this->blank.y = averageSample(this->yLightControl, this->detectorRead);
}

void Photometer::takeSample(){
  this->sample.x = averageSample(this->xLightControl, this->detectorRead);
  this->sample.y = averageSample(this->yLightControl, this->detectorRead);

  this->absReading.Abs1 = log((float)this->blank.x/(float)this->sample.x)/(log(10));//calculate the absorbance
  this->absReading.Abs2 = log((float)this->blank.y/(float)this->sample.y)/(log(10));//calculate the absorbance
  this->absReading.R=(float)this->absReading.Abs2/(float)this->absReading.Abs1;
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
  dest->Abs1 = this->absReading.Abs1;
  dest->Abs2 = this->absReading.Abs2;
  dest->R = this->absReading.R;
}
