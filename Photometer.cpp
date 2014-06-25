#include "Photometer.h"

Photometer::Photometer(PinControlFunPtr blueLightControl, PinControlFunPtr greenLightControl, DetectorReadFunPtr detectorRead){
  this->blueLightControl = blueLightControl;
  this->greenLightControl = greenLightControl;
  this->detectorRead = detectorRead;
  
  this->sample.blue = this->sample.green = 0;
  this->blank.blue = this->blank.green = 0;
}

Photometer::~Photometer(){
}

float averageSample(PinControlFunPtr lightControl, DetectorReadFunPtr detectorRead){
  const float NUM_SAMPLES = 10;
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
  this->prevBlank.blue = this->blank.blue;
  this->prevBlank.green = this->blank.green;
  
  this->blank.blue = averageSample(this->blueLightControl, this->detectorRead);
  this->blank.green = averageSample(this->greenLightControl, this->detectorRead);
}

void Photometer::takeSample(){
  this->prevSample.blue = this->sample.blue;
  this->prevSample.green = this->sample.green;
  
  this->sample.blue = averageSample(this->blueLightControl, this->detectorRead);
  this->sample.green = averageSample(this->greenLightControl, this->detectorRead);

  this->prevAbsReading.Abs1 = this->absReading.Abs1;
  this->prevAbsReading.Abs2 = this->absReading.Abs2;
  this->prevAbsReading.R = this->absReading.R;
  
  this->absReading.Abs1 = log((float)this->blank.blue/(float)this->sample.blue)/(log(10));//calculate the absorbance
  this->absReading.Abs2 = log((float)this->blank.green/(float)this->sample.green)/(log(10));//calculate the absorbance
  this->absReading.R=(float)this->absReading.Abs2/(float)this->absReading.Abs1;
}
    
void Photometer::getBlank(PHOTOREADING* dest, PHOTOREADING* prev){
  dest->blue = this->blank.blue;
  dest->green = this->blank.green;
  
  if(prev){
    prev->blue = this->prevBlank.blue;
    prev->green = this->prevBlank.green;
  }
}

void Photometer::getSample(PHOTOREADING* dest, PHOTOREADING* prev){
  dest->blue = this->sample.blue;
  dest->green = this->sample.green;
  
  if(prev){
    prev->blue = this->prevSample.blue;
    prev->green = this->prevSample.green;
  }
}

void Photometer::getAbsorbance(ABSREADING* dest, ABSREADING* prev){
  dest->Abs1 = this->absReading.Abs1;
  dest->Abs2 = this->absReading.Abs2;
  dest->R = this->absReading.R;
  
  if(prev){
    prev->Abs1 = this->prevAbsReading.Abs1;
    prev->Abs2 = this->prevAbsReading.Abs2;
    prev->R = this->prevAbsReading.R;
  }
}
