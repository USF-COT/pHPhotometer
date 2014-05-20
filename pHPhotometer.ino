#include <LiquidCrystal.h>   //use LCD library
#include <SPI.h>
#include <SD.h>
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>
#include <OneWire.h>
#include <MenuSystem.h>
#include "Photometer.h"
#include "ECTShield.h"
#include "Adafruit_MCP23008.h"

Adafruit_MCP23008 mcp;

const byte blueLEDPin = 0;
const byte greenLEDPin = 7;
const byte detectorPin = 1;

void blueLEDControl(int level){
  mcp.digitalWrite(0, level);
}
void greenLEDControl(int level){
  mcp.digitalWrite(7, level);
}
int readLightConverter(){
  return analogRead(1);
}

Photometer photometer(blueLEDControl, greenLEDControl, readLightConverter);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);// LCD pin

MenuSystem ms;

Menu rootMenu("pH Photometer");

Menu sampleMenu("Sample >");
MenuItem sampleRecordItem("Record");
MenuItem sampleHistoryItem("History");

Menu blankMenu("Blank >");
MenuItem blankRecordItem("Record");
MenuItem blankCurrentItem("Current Values");

Menu settingsMenu("Settings >");
MenuItem settingsTimeItem("Time");
MenuItem settingsPhotoCal("Photometer Cal.");
Menu settingsThermCalMenu("Thermometer Cal.");
MenuItem settingsThermCalMin("Calibrate Min");
MenuItem settingsThermCalMax("Calibrate Max");
Menu settingsCondCalMenu("Conductivity Cal.");
MenuItem settingsCondCalMin("Calibrate Min");
MenuItem settingsCondCalMax("Calibrate Max");

void calThermMinSelected(MenuItem*){
  
}

void calThermMaxSelected(MenuItem*){
}

void calCondMinSelected(MenuItem*){
}

void calCondMaxSelected(MenuItem*){
}

void setupMenu(){
  rootMenu.add_menu(&sampleMenu);
  sampleMenu.add_item(&sampleRecordItem, &recordSample);
  sampleMenu.add_item(&sampleHistoryItem, &displaySampleHistory);
  
  rootMenu.add_menu(&blankMenu);
  blankMenu.add_item(&blankRecordItem, &recordBlank);
  blankMenu.add_item(&blankCurrentItem, &displayBlankSelected);
  
  rootMenu.add_menu(&settingsMenu);
  // Thermometer Calibration Sub-menu
  settingsMenu.add_menu(&settingsThermCalMenu);
  settingsThermCalMenu.add_item(&settingsThermCalMin, &calThermMinSelected);
  settingsThermCalMenu.add_item(&settingsThermCalMax, &calThermMaxSelected);
  // Conductivity Calibration Sub-menu
  settingsMenu.add_menu(&settingsCondCalMenu);
  settingsCondCalMenu.add_item(&settingsCondCalMin, &calCondMinSelected);
  settingsCondCalMenu.add_item(&settingsCondCalMax, &calCondMaxSelected);
  
  ms.set_root_menu(&rootMenu);
    
  displayMenu();
}

void print2digits(int number, Print* printer){
  if (number >=0 && number < 10){
    printer->write('0');
  }
  printer->print(number);
}

void writeISO8601(tmElements_t* tm, Print* printer){
  printer->print(tmYearToCalendar(tm->Year));
  printer->print("-");
  print2digits(tm->Month, printer);
  printer->print("-");
  print2digits(tm->Day, printer);
  printer->print("T");
  print2digits(tm->Hour, printer);
  printer->print(":");
  print2digits(tm->Minute, printer);
  printer->print(":");
  print2digits(tm->Second, printer);
  printer->print("Z");
}

void writeLog(char* message){
  File logFile = SD.open("log.txt", FILE_WRITE);
  
  tmElements_t tm;
  if(RTC.read(tm)){
    writeISO8601(&tm, &logFile);
    logFile.print(", ");
  } else {
    logFile.print("None, ");
  }
  logFile.println(message);
  logFile.close();
}

boolean setupSDCard(){
  pinMode(3, OUTPUT);
  pinMode(10, OUTPUT); // SS pin must be configured as an output
  if(!SD.begin(3)){
    Serial.println("SD Initialization Failed!");
    return false;
  }
  writeLog("Photometer Started");
  return true;
}

void setup(){
  Serial.begin(9600);
  while(!Serial);  // Wait for serial
  delay(200);
  
  lcd.begin(16, 2);  //Initialize LCD

  lcd.print("MiniSpec B.Y.");  //Display Mini Spectrophotometer
  delay(1000); //Delay1000ms
  lcd.clear();
  
  // Initialize blue and green outputs
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(7, OUTPUT);
  
  if(!setupSDCard()){
    lcd.print("Error: Check Serial Debugger");
    return;
  }
  
  setupMenu();
}

void writeBlank(PHOTOREADING* blank){
  /*
  File blankFile;
  if(!SD.exists("blank.csv")){
    blankFile = SD.open("blank.csv", FILE_WRITE);
    blankFile.println("Timestamp,Blank Blue, Blank Green");
  } else {
    blankFile = SD.open("blank.csv", FILE_WRITE);
  }
  */
  
  File blankFile = SD.open("blank.csv", FILE_WRITE);
  
  tmElements_t tm;
  if(RTC.read(tm)){
    writeISO8601(&tm, &blankFile);
    blankFile.print(",");
  } else {
    blankFile.print("None,");
  }
  blankFile.print(blank->x);
  blankFile.print(",");
  blankFile.println(blank->y);
  blankFile.close();
}

void displayBlank(PHOTOREADING* blank){  
  lcd.clear(); //clear

  lcd.print("Blank(");
  lcd.print(blank->x);
  lcd.print(")");

  lcd.setCursor(0, 1) ;
  lcd.print("Blank(");
  lcd.print(blank->y);
  lcd.print(")");
  
  delay(1000);
}

void recordBlank(MenuItem*){
  lcd.clear();
  lcd.print("Recording");
  lcd.setCursor(0, 1);
  lcd.print("Blank...");
  
  photometer.takeBlank();
  
  PHOTOREADING blank;
  photometer.getBlank(&blank);
  displayBlank(&blank);
  writeBlank(&blank);
}

void displayBlankSelected(MenuItem*){
  PHOTOREADING blank;
  photometer.getBlank(&blank);
  
  displayBlank(&blank);
}

void writeSample(PHOTOREADING* blank, PHOTOREADING* sample, ABSREADING* absReading){
  /*
  File samplesFile;
  if(!SD.exists("samples.csv")){
    samplesFile = SD.open("samples.csv", FILE_WRITE);
    samplesFile.println("Timestamp,Blank Blue, Blank Green, Sample Blue, Sample Green, Absorbance A1, Absorbance A2, R");
  } else {
    samplesFile = SD.open("samples.csv", FILE_WRITE);
  }
  */
  
  File samplesFile = SD.open("samples.csv", FILE_WRITE);
  
  tmElements_t tm;
  if(RTC.read(tm)){
    writeISO8601(&tm, &samplesFile);
    samplesFile.print(",");
  } else {
    samplesFile.print("None,");
  }
  samplesFile.print(blank->x);
  samplesFile.print(",");
  samplesFile.print(blank->y);
  samplesFile.print(",");
  samplesFile.print(sample->x);
  samplesFile.print(",");
  samplesFile.print(sample->y);
  samplesFile.print(",");
  samplesFile.print(absReading->A1);
  samplesFile.print(",");
  samplesFile.print(absReading->A2);
  samplesFile.print(",");
  samplesFile.println(absReading->R);
  samplesFile.close();
}

void displaySample(PHOTOREADING* blank, PHOTOREADING* sample, ABSREADING* absReading){
  lcd.clear(); 

  lcd.print("A1=");
  lcd.print(absReading->A1,3);
  lcd.print("(");
  lcd.print(sample->x);
  lcd.print(")");

  lcd.setCursor(0, 1) ;

  lcd.print("A2=");
  lcd.print(absReading->A2,3);
  lcd.print("(");
  lcd.print(sample->y);
  lcd.print(")");
  
  delay(1000);
}

void recordSample(MenuItem*){
  lcd.clear();
  lcd.print("Recording");
  lcd.setCursor(0, 1);
  lcd.print("Sample...");
  
  photometer.takeSample();
  
  PHOTOREADING blank, sample;
  ABSREADING absReading;
  photometer.getBlank(&blank);
  photometer.getSample(&sample);
  photometer.getAbsorbance(&absReading);
  
  displaySample(&blank, &sample, &absReading);
  writeSample(&blank, &sample, &absReading);
}

void displaySampleHistory(MenuItem*){
  
}

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

int read_LCD_buttons(){               // read the buttons
    int adc_key_in = analogRead(0);       // read the value from the sensor 

    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    // We make this the 1st option for speed reasons since it will be the most likely result

    if (adc_key_in > 1000) return btnNONE; 

    // For V1.1 us this threshold
    if (adc_key_in < 50)   return btnRIGHT;  
    if (adc_key_in < 250)  return btnUP; 
    if (adc_key_in < 450)  return btnDOWN; 
    if (adc_key_in < 650)  return btnLEFT; 
    if (adc_key_in < 850)  return btnSELECT;

    return btnNONE;                // when all others fail, return this.
}

void displayMenu(){
  lcd.clear();
  lcd.setCursor(0,0);
  
  Menu const* cp_menu = ms.get_current_menu();
  lcd.print(cp_menu->get_name());
  
  lcd.setCursor(0,1);
  
  lcd.print(cp_menu->get_selected()->get_name());
}

void loop(){
  int lcd_key = read_LCD_buttons();
  
  
  switch (lcd_key){
    case btnSELECT:{
      ms.select();
      displayMenu();
      break;
    }
    case btnUP:{
      ms.prev();
      displayMenu();
      break;
    }
    case btnDOWN:{
      ms.next();
      displayMenu();
      break;
    }
    case btnLEFT:{
      ms.back();
      displayMenu();
      break;
    }
    case btnRIGHT:{
      ms.select();
      displayMenu();
      break;
    }
  }
  delay(140);
}
