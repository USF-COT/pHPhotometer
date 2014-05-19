#include <LiquidCrystal.h>   //use LCD library
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>
#include <MenuSystem.h>
#include "Photometer.h"

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
MenuItem settingsThermCal("Thermometer Cal.");
MenuItem settingsCondCal("Conductivity Cal.");

void setupMenu(){
  rootMenu.add_menu(&sampleMenu);
  sampleMenu.add_item(&sampleRecordItem, &recordSample);
  sampleMenu.add_item(&sampleHistoryItem, &displaySampleHistory);
  
  rootMenu.add_menu(&blankMenu);
  blankMenu.add_item(&blankRecordItem, &recordBlank);
  blankMenu.add_item(&blankCurrentItem, &displayBlankSelected);
  
  rootMenu.add_menu(&settingsMenu);
  
  ms.set_root_menu(&rootMenu);
    
  displayMenu();
}

Photometer photometer(7, 6, 1);

void setup(){
  Serial.begin(9600);
  while(!Serial);  // Wait for serial
  delay(200);

  lcd.begin(16, 2);  //Initialize LCD

  lcd.print("MiniSpec B.Y.");  //Display Mini Spectrophotometer
  delay(1000); //Delay1000ms
  
  lcd.clear();
  
  setupMenu();
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
}

void displayBlankSelected(MenuItem*){
  PHOTOREADING blank;
  photometer.getBlank(&blank);
  
  displayBlank(&blank);
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

  Serial.print(blank->x);
  Serial.print(sample->x);

  Serial.print(blank->y);
  Serial.print(sample->y);
  
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
  delay(100);
}
