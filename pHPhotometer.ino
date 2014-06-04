#include <SdFat.h>
#include <Wire.h>
#include <OneWire.h>
#include <DS1307RTC.h>
#include <Time.h>
#include <LiquidCrystal.h>   //use LCD library
#include <MenuSystem.h>
#include <MemoryFree.h>
#include <stdlib.h>
#include "Photometer.h"
#include "ECTShield.h"
#include "Adafruit_MCP23008.h"

SdFat sd;

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

void condControl(int setting){
  mcp.digitalWrite(4, setting);
}

unsigned long condRead(int setting, unsigned int frequency){
  return pulseIn(A2, HIGH, frequency);
}

ECTShield ect(condControl, condRead, 2);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);// LCD pin

typedef void (*displayHandler)(int);
displayHandler currentDisplayHandler = mainMenuHandler;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

MenuSystem ms;
Menu rootMenu("pH Photometer");

Menu sampleMenu("Sample >");
MenuItem sampleRecordItem("Record");
MenuItem sampleHistoryItem("History");

Menu blankMenu("Blank >");
MenuItem blankRecordItem("Record");
MenuItem blankCurrentItem("Current Values");


Menu calibrationMenu("Calibration >");
MenuItem calibrationEC("Read EC Raw");

void rawECHandler(int lcd_key){
  float temperature = ect.getTemperature();
  unsigned long frequency = ect.getConductivityFrequency();
  
  lcd.clear();
  lcd.print(" T: ");
  lcd.print(temperature);
  lcd.setCursor(0, 1);
  
  lcd.print("Cf: ");
  lcd.print(frequency);
  
  if(lcd_key != btnNONE){
    currentDisplayHandler = mainMenuHandler;
    displayMenu();
  }
}

void readECRaw(MenuItem*){
  lcd.clear();
  lcd.print("Reading ECT...");
  currentDisplayHandler = rawECHandler;
}

void setupMenu(){
  rootMenu.add_menu(&sampleMenu);
  sampleMenu.add_item(&sampleRecordItem, &recordSample);
  //sampleMenu.add_item(&sampleHistoryItem, &displaySampleHistory);
  
  rootMenu.add_menu(&blankMenu);
  blankMenu.add_item(&blankRecordItem, &recordBlank);
  blankMenu.add_item(&blankCurrentItem, &displayBlankSelected);
  
  rootMenu.add_menu(&calibrationMenu);
  calibrationMenu.add_item(&calibrationEC, &readECRaw);
  
  ms.set_root_menu(&rootMenu);
    
  displayMenu();
}

void dateTime(uint16_t* date, uint16_t* time){
  tmElements_t tm;
  if(RTC.read(tm)){
    *date = FAT_DATE(tmYearToCalendar(tm.Year), tm.Month, tm.Day);
    *time = FAT_TIME(tm.Hour, tm.Minute, tm.Second);
  }
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
  SdFile logFile;
  
  if(logFile.open("LOG.TXT", O_RDWR | O_CREAT | O_AT_END)){
    tmElements_t tm;
    if(RTC.read(tm)){
      writeISO8601(&tm, &logFile);
      logFile.print(",");
    } else {
      logFile.print("None,");
    }
    logFile.println(message);
    logFile.close();
  } else {
    lcd.print("Error writing");
    lcd.setCursor(0, 1);
    lcd.print("log file!");
    sd.errorHalt("Opening log.txt failed");
  }
}

boolean setupSDCard(){
  SdFile::dateTimeCallback(dateTime);
  if(!sd.begin(A3, SPI_FULL_SPEED)){
    lcd.print("Card Error:");
    lcd.setCursor(0,1);
    lcd.print("Check Serial");
    sd.initErrorHalt();
    return false;
  }
  writeLog("Photometer Started");
  return true;
}

void setupECTShield(){
  mcp.pinMode(4, OUTPUT);
  pinMode(A2, INPUT);
}

void setup(){
  Serial.begin(9600);
  while(!Serial);  // Wait for serial
  
  lcd.begin(16, 2);  //Initialize LCD
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  lcd.print("MiniSpec B.Y.");  //Display Mini Spectrophotometer
  delay(1000); //Delay1000ms
  lcd.clear();
  
  // Initialize blue and green outputs
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(7, OUTPUT);
  
  setupSDCard();
  setupECTShield();
  
  setupMenu();
}

void writeBlank(PHOTOREADING* blank){
  SdFile blankFile;
  
  if(blankFile.open("BLANK.CSV", O_RDWR | O_CREAT | O_AT_END)){
    FatPos_t pos;
    blankFile.getpos(&pos);
    if(pos.position == 0){
      blankFile.print("Timestamp,");
      blankFile.print("Blank Blue,");
      blankFile.println("Blank Green");
    }
    
    tmElements_t tm;
    if(RTC.read(tm)){
      writeISO8601(&tm, &blankFile);
      blankFile.print(",");
    } else {
      blankFile.print("None,");
    }
    blankFile.print(blank->blue);
    blankFile.print(",");
    blankFile.println(blank->green);
    blankFile.close();
  } else {
    lcd.clear();
    lcd.print("Error writing");
    lcd.setCursor(0, 1);
    lcd.print("blank reading!");
    sd.errorHalt("Opening blank.csv failed!");
  }
}

void displayBlank(PHOTOREADING* blank){  
  lcd.clear(); //clear

  lcd.print("Blank(");
  lcd.print(blank->blue);
  lcd.print(")");

  lcd.setCursor(0, 1) ;
  lcd.print("Blank(");
  lcd.print(blank->green);
  lcd.print(")");
  delay(2000);
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
  SdFile samplesFile;
  
  if(samplesFile.open("SAMPLES.CSV", O_RDWR | O_CREAT | O_AT_END)){
    FatPos_t pos;
    samplesFile.getpos(&pos);
    if(pos.position == 0){
      samplesFile.print("Timestamp,");
      samplesFile.print("Blank Blue,");
      samplesFile.print("Blank Green,");
      samplesFile.print("Sample Blue,");
      samplesFile.print("Sample Green,");
      samplesFile.print("Absorbance A1,");
      samplesFile.print("Absorbance A2,");
      samplesFile.println("R");
    }
    
    tmElements_t tm;
    if(RTC.read(tm)){
      writeISO8601(&tm, &samplesFile);
      samplesFile.print(",");
    } else {
      samplesFile.print("None,");
    }
    samplesFile.print(blank->blue);
    samplesFile.print(",");
    samplesFile.print(blank->green);
    
    samplesFile.print(",");
    samplesFile.print(sample->blue);
    samplesFile.print(",");
    samplesFile.print(sample->green);
    samplesFile.print(",");
    
    char floatBuffer[16];
    dtostrf(absReading->Abs1, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    samplesFile.print(",");
    dtostrf(absReading->Abs2, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    samplesFile.print(",");
    dtostrf(absReading->R, 8, 3, floatBuffer);
    samplesFile.println(floatBuffer);
    
    samplesFile.close();
  } else {
    lcd.clear();
    lcd.print("Error writing");
    lcd.setCursor(0, 1);
    lcd.print("sample reading!");
    sd.errorHalt("Opening samples.csv failed!");
  }
}

void displaySample(PHOTOREADING* blank, PHOTOREADING* sample, ABSREADING* absReading){
  lcd.clear(); 

  lcd.print("A1=");
  lcd.print(absReading->Abs1,3);
  lcd.print("(");
  lcd.print(sample->blue);
  lcd.print(")");

  lcd.setCursor(0, 1) ;

  lcd.print("A2=");
  lcd.print(absReading->Abs2,3);
  lcd.print("(");
  lcd.print(sample->green);
  lcd.print(")");
  
  delay(2000);
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

void displayMenu(){  
  lcd.clear();
  lcd.setCursor(0,0);
  Menu const* cp_menu = ms.get_current_menu();
  lcd.print(cp_menu->get_name());
  
  lcd.setCursor(0,1);
  
  lcd.print(cp_menu->get_selected()->get_name());
}

void mainMenuHandler(int lcd_key){
  switch (lcd_key){
    case btnSELECT:{
      ms.select();
      displayMenu();
      break;
    }
    case btnUP:{
      ms.prev(true);
      displayMenu();
      break;
    }
    case btnDOWN:{
      ms.next(true);
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
}

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

void loop(){
  int lcd_key = read_LCD_buttons();
  
  currentDisplayHandler(lcd_key);
  
  delay(150);
}
