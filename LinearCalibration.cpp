#include "LinearCalibration.h"

LinearCalibration::LinearCalibration(CALIBRATIONPOINT* minimum, CALIBRATIONPOINT* maximum){
  this->minimum.x = minimum->x;
  this->minimum.y = minimum->y;

  this->maximum.x = maximum->x;
  this->maximum.y = maximum->y;

  this->m = (this->maximum.y - this->minimum.y)/(this->maximum.x - this->minimum.x);
  this->b = this->maximum.y - this->m * this->maximum.x;
}
LinearCalibration::~LinearCalibration(){
}

void LinearCalibration::save(Print* printer){
  printer.print(this->minimum.x);
  printer.print(",");
  printer.print(this->minimum.y);
  printer.print(",");
  printer.print(this->maximum.x);
  printer.print(",");
  printer.print(this->maximum.y);
  printer.print(",");
  printer.print(this->m);
  printer.print(",");
  printer.println(this->b);
}
//static LinearCalibration LinearCalibration::load(Stream* stream);

float LinearCalibration::correct(float independent, float reading){
  return this->m*independent + 
}

