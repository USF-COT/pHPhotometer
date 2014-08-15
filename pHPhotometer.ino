#include <Wire.h>
#include <OneWire.h>
#include <DS1307RTC.h>
#include <Time.h>
#include <LiquidCrystal.h>   //use LCD library
#include <MenuSystem.h>
#include <SdFat.h>
#include "Photometer.h"
#include "ECTShield.h"
#include "Adafruit_MCP23008.h"
#include "Calibration.h"

SdFat sd;

Adafruit_MCP23008 mcp;

// Photometer Setup
void blueLEDControl(int level){
  mcp.digitalWrite(7, level);
}
void greenLEDControl(int level){
  mcp.digitalWrite(0, level);
}
int readLightConverter(){
  return analogRead(A1);
}
Photometer photometer(blueLEDControl, greenLEDControl, readLightConverter);

// ECT Shield Setup
void condControl(int setting){
  mcp.digitalWrite(4, setting);
}
unsigned long condRead(int setting, unsigned int frequency){
  return pulseIn(A2, setting, frequency);
}
ECTShield ect(condControl, condRead, 2);
Calibration condCal("cond");

// Liquid Crystal Setup
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);// LCD pin

// Menu Handler Setup
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
//MenuItem sampleHistoryItem("History");

Menu blankMenu("Blank >");
MenuItem blankRecordItem("Record");
MenuItem blankCurrentItem("Current Values");

Menu calibrationMenu("Calibration >");
MenuItem calibrationEC("Read EC Raw");
MenuItem readEC("Read EC Cal");

// Setup Functions
void setupMenu(){
  rootMenu.add_menu(&blankMenu);
  blankMenu.add_item(&blankRecordItem, &recordBlank);
  blankMenu.add_item(&blankCurrentItem, &displayBlankSelected);
  
  rootMenu.add_menu(&sampleMenu);
  sampleMenu.add_item(&sampleRecordItem, &recordSample);
  //sampleMenu.add_item(&sampleHistoryItem, &displaySampleHistory);
  
  rootMenu.add_menu(&calibrationMenu);
  calibrationMenu.add_item(&calibrationEC, &readECRaw);
  calibrationMenu.add_item(&readEC, &readECCal);
  
  ms.set_root_menu(&rootMenu);
    
  displayMenu();
}

void print2digits(int number, Print& printer){
  if (number >=0 && number < 10){
    printer.write('0');
  }
  printer.print(number);
}

void writeISO8601(tmElements_t* tm, Print& printer){
  printer.print(tmYearToCalendar(tm->Year));
  printer.print("-");
  print2digits(tm->Month, printer);
  printer.print("-");
  print2digits(tm->Day, printer);
  printer.print("T");
  print2digits(tm->Hour, printer);
  printer.print(":");
  print2digits(tm->Minute, printer);
  printer.print(":");
  print2digits(tm->Second, printer);
  printer.print("Z");
}

void writeLog(char* message){
  SdFile logFile;
  
  if(logFile.open("LOG.TXT", O_RDWR | O_CREAT | O_AT_END)){
    tmElements_t tm;
    if(RTC.read(tm)){
      writeISO8601(&tm, logFile);
      logFile.print(",");
    } else {
      logFile.print("None,");
    }
    logFile.println(message);
    logFile.close();
  } else {
    sd.errorHalt("Opening log.txt failed");
  }
}

void fileDateTime(uint16_t* date, uint16_t* time){
  tmElements_t tm;
  if(RTC.read(tm)){
    *date = FAT_DATE(tmYearToCalendar(tm.Year), tm.Month, tm.Day);
    *time = FAT_TIME(tm.Hour, tm.Minute, tm.Second);
  }
}
boolean setupSDCard(){
  SdFile::dateTimeCallback(fileDateTime);
  pinMode(A3, OUTPUT);
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
  mcp.digitalWrite(4, LOW);
  pinMode(A2, INPUT);
}

void setup(){
  Serial.begin(9600);
  while(!Serial);  // Wait for serial
  
  lcd.begin(16, 2);  //Initialize LCD

  lcd.print("MiniSpec B.Y.");  //Display Mini Spectrophotometer
  delay(1000); //Delay1000ms
  lcd.clear();
  
  // Initialize blue and green outputs
  mcp.pinMode(0, OUTPUT);
  mcp.digitalWrite(0, LOW);
  mcp.pinMode(7, OUTPUT);
  mcp.digitalWrite(0, LOW);
  
  // Initialize hardware SS pin, drive high for lcd backlight
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  
  // Setup Photometer Read Pin
  pinMode(A1, INPUT);
  
  setupSDCard();
  setupECTShield();
  condCal.load();
  setupMenu();
}

#define BLANKFILEPATH "blank.csv"
void writeBlank(PHOTOREADING* blank){
  SdFile blankFile;
  //boolean existed = sd.exists(BLANKFILEPATH);
  
  if(blankFile.open(BLANKFILEPATH, O_RDWR | O_CREAT | O_AT_END)){
    /*
    if(!existed){
      blankFile.print("Timestamp,");
      blankFile.print("Blank Blue,");
      blankFile.println("Blank Green");
    }
    */
    
    tmElements_t tm;
    if(RTC.read(tm)){
      writeISO8601(&tm, blankFile);
      blankFile.print(",");
    } else {
      blankFile.print("None,");
    }
    char floatBuffer[24];
    dtostrf(blank->blue, 8, 3, floatBuffer);
    blankFile.print(floatBuffer);
    blankFile.print(",");
    dtostrf(blank->green, 8, 3, floatBuffer);
    blankFile.println(floatBuffer);
    blankFile.close();
  } else {
    lcd.clear();
    sd.errorHalt("Opening blank.csv failed!");
  }
}

void displaySample(PHOTOREADING* sample){  
  lcd.clear(); //clear

  lcd.print("B(");
  lcd.print(sample->blue);
  lcd.print(")");

  lcd.setCursor(0, 1) ;
  lcd.print("G(");
  lcd.print(sample->green);
  lcd.print(")");
  delay(2000);
}

#define CONVTOL 2
boolean hasConverged(PHOTOREADING* last, PHOTOREADING* current){
  float blueDiff = abs(last->blue - current->blue);
  float greenDiff = abs(last->green - current->green);
  
  if(blueDiff <= CONVTOL && greenDiff <= CONVTOL){
    lcd.clear();
    lcd.print("Converged!");
    delay(1500);
    displaySample(current);
    return true;
  } else {
    lcd.clear();
    lcd.print("Not Converged...");
    delay(1000);
    lcd.clear();
    
    lcd.print("Bdiff ");
    lcd.print(blueDiff);
    lcd.setCursor(0, 1);
    lcd.print("Gdiff ");
    lcd.print(greenDiff);
    
    delay(1500);
    return false;
  }
}

void recordingBlank(int lcd_key){
  displayRecording();
  
  PHOTOREADING lastBlank;
  photometer.getBlank(&lastBlank);
  
  PHOTOREADING currentBlank;
  photometer.takeBlank();
  photometer.getBlank(&currentBlank);
  
  if(hasConverged(&lastBlank, &currentBlank)){
    writeBlank(&currentBlank);
    returnToMainMenu();
  } else if(lcd_key != btnNONE){
    returnToMainMenu();
  }
}

void displayRecording(){
  lcd.clear();
  lcd.print("Recording...");  
}

void recordBlank(MenuItem*){
  displayRecording();
  photometer.takeBlank();
  currentDisplayHandler = recordingBlank;
}

void displayBlankSelected(MenuItem*){
  PHOTOREADING blank;
  photometer.getBlank(&blank);
  
  displaySample(&blank);
}

#define SAMPLEFILEPATH "samples.csv"
void writeSample(PHOTOREADING* blank, PHOTOREADING* sample, ABSREADING* absReading){
  SdFile samplesFile;
  //boolean existed = sd.exists(SAMPLEFILEPATH);
  
  if(samplesFile.open(SAMPLEFILEPATH, O_RDWR | O_CREAT | O_AT_END)){
    /*
    if(!existed){
      samplesFile.print("Timestamp,");
      samplesFile.print("Blank Blue,");
      samplesFile.print("Blank Green,");
      samplesFile.print("Sample Blue,");
      samplesFile.print("Sample Green,");
      samplesFile.print("Absorbance A1,");
      samplesFile.print("Absorbance A2,");
      samplesFile.println("R");
    }
    */
    
    tmElements_t tm;
    if(RTC.read(tm)){
      writeISO8601(&tm, samplesFile);
      samplesFile.print(",");
    } else {
      samplesFile.print("None,");
    }
    
    char floatBuffer[24];
    dtostrf(blank->blue, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    samplesFile.print(",");
    dtostrf(blank->green, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    
    samplesFile.print(",");
    dtostrf(sample->blue, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    samplesFile.print(",");
    dtostrf(sample->green, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    samplesFile.print(",");
    
    dtostrf(absReading->Abs1, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    samplesFile.print(",");
    dtostrf(absReading->Abs2, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    samplesFile.print(",");
    dtostrf(absReading->R, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    samplesFile.print(",");
    
    float temperature = ect.getTemperature();
    unsigned long frequency = ect.getConductivityFrequency();
    
    dtostrf(temperature, 8, 3, floatBuffer);
    samplesFile.print(floatBuffer);
    samplesFile.print(",");
    samplesFile.print(frequency);
    samplesFile.print(",");
    dtostrf(condCal.adjustReading(frequency), 8, 3, floatBuffer);
    samplesFile.println(floatBuffer);
    
    samplesFile.close();
  } else {
    sd.errorHalt("Opening samples.csv failed!");
  }
}

void displayAbsorbance(PHOTOREADING* blank, PHOTOREADING* sample, ABSREADING* absReading){
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
  
  delay(5000);
}

void recordSample(MenuItem*){
  displayRecording();
  photometer.takeSample();
  currentDisplayHandler = recordingSample;
}

void recordingSample(int lcd_key){
  PHOTOREADING blank;
  photometer.getBlank(&blank);
  
  if(blank.blue == 0 && blank.green == 0){
    lcd.clear();
    lcd.print("ERROR:");
    lcd.setCursor(0,1);
    lcd.print("NO BLANK");
    delay(2000);
    returnToMainMenu();
    return;
  }
  
  displayRecording();
  
  PHOTOREADING lastSample;
  photometer.getSample(&lastSample);
  
  PHOTOREADING currentSample;
  photometer.takeSample();
  photometer.getSample(&currentSample);
  
  if(hasConverged(&lastSample, &currentSample)){
    ABSREADING absReading;
    photometer.getAbsorbance(&absReading);
    
    displayAbsorbance(&blank, &currentSample, &absReading);
    writeSample(&blank, &currentSample, &absReading);
    returnToMainMenu();
  } else if(lcd_key != btnNONE){
    returnToMainMenu();
  }
}

void displayTemperature(){
  float temperature = ect.getTemperature();
  
  lcd.print("T: ");
  lcd.print(temperature);
}

// ECT Shield Display Handler
void rawECHandler(int lcd_key){
  unsigned long frequency = ect.getConductivityFrequency();
  lcd.clear();
  displayTemperature();
  lcd.setCursor(0, 1);
  
  lcd.print("Cf: ");
  //char floatBuffer[24];
  //dtostrf(condCal.adjustReading(frequency), 8, 3, floatBuffer);
  //dtostrf(frequency, 8, 3, floatBuffer);
  lcd.print(frequency);
    
  if(lcd_key != btnNONE){
    returnToMainMenu();
  }
}

void calECHandler(int lcd_key){
  unsigned long frequency = ect.getConductivityFrequency();
  lcd.clear();
  displayTemperature();
  lcd.setCursor(0,1);
  
  lcd.print("C: ");
  char floatBuffer[24];
  dtostrf(condCal.adjustReading(frequency), 8, 3, floatBuffer);
  lcd.print(floatBuffer);
  
  if(lcd_key != btnNONE){
    currentDisplayHandler = mainMenuHandler;
    displayMenu();
  }
}

void readECRaw(MenuItem*){
  currentDisplayHandler = rawECHandler;
  lcd.clear();
  lcd.print("Reading ECT...");
}

void readECCal(MenuItem*){
  currentDisplayHandler = calECHandler;
  lcd.clear();
  lcd.print("Reading ECT...");
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

void returnToMainMenu(){
  currentDisplayHandler = mainMenuHandler;
  displayMenu();
}

void loop(){
  int lcd_key = read_LCD_buttons();
  
  currentDisplayHandler(lcd_key);
  
  delay(200);
}
