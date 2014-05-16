#include <LiquidCrystal.h>   //use LCD library
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>
#include <MenuSystem.h>
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
  sampleMenu.add_item(&sampleHistoryItem, &displayHistory);
  
  rootMenu.add_menu(&blankMenu);
  blankMenu.add_item(&blankRecordItem, &recordBlank);
  blankMenu.add_item(&blankCurrentItem, &displayBlank);
  
  rootMenu.add_menu(&settingsMenu);
  
  ms.set_root_menu(&rootMenu);
    
  displayMenu();
}

int potPin = 1; //detector pin
int i;

void setup(){
  Serial.begin(9600);
  while(!Serial);  // Wait for serial
  delay(200);

  lcd.begin(16, 2);  //Initialize LCD

  lcd.print("MiniSpec B.Y.");  //Display Mini Spectrophotometer
  delay(1000); //Delay1000ms
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  
  lcd.clear();
  
  setupMenu();
}

const int NUM_SAMPLES = 10;

int x1a=0;
int y1a=0;
int x2a=0;
int y2a=0;

float A_1 = 0.000;//absorbance
float A_2 = 0.000;//absorbance
float R=0.000;

void calculateBlank(){
  digitalWrite(7,HIGH);

  delay(2000);

  for(i=0; i<NUM_SAMPLES; i++){
    x1a += analogRead(potPin);
  }
    
  x1a /= NUM_SAMPLES; //read the blank

  digitalWrite(7,LOW);
  digitalWrite(6,HIGH);

  delay(2000);

  for(i=0; i<NUM_SAMPLES; i++){
    y1a+=analogRead(potPin);
  }

  y1a /= NUM_SAMPLES ; //read the blank

  digitalWrite(6,LOW);
}

void recordBlank(MenuItem*){
  lcd.clear();
  lcd.print("Recording");
  lcd.setCursor(0, 1);
  lcd.print("Blank...");
  
  calculateBlank();

  lcd.clear(); //clear

  lcd.print("Blank(");
  lcd.print(x1a);
  lcd.print(")");

  lcd.setCursor(0, 1) ;
  lcd.print("Blank(");
  lcd.print(y1a);
  lcd.print(")");
  delay(1000);
}

void displayBlank(MenuItem*){
  
}

void recordSample(MenuItem*){
  lcd.clear();
  lcd.print("Recording");
  lcd.setCursor(0, 1);
  lcd.print("Sample...");
  digitalWrite(7,HIGH);

  delay(2000);

  for(i=0; i<NUM_SAMPLES; i++){
    x2a += analogRead(potPin);
  }
  x2a /= NUM_SAMPLES; //read the sample

  Serial.print(x1a);
    
  digitalWrite(7,LOW);
  digitalWrite(6,HIGH);
  delay(2000);

  for(i=0; i<NUM_SAMPLES; i++){
    y2a += analogRead(potPin);
  }

  y2a /= NUM_SAMPLES; //read the sample

  digitalWrite(6,LOW);

  A_1 = log((float)x1a/(float)x2a)/(log(10));//calculate the absorbance
  A_2 = log((float)y1a/(float)y2a)/(log(10));//calculate the absorbance
  R=(float)A_2/(float)A_1;

  lcd.clear(); 

  lcd.print("A1=");
  lcd.print(A_1,3);
  lcd.print("(");
  lcd.print(x2a);
  lcd.print(")");

  lcd.setCursor(0, 1) ;

  lcd.print("A2=");
  lcd.print(A_2,3);
  lcd.print("(");
  lcd.print(y2a);
  lcd.print(")");

  Serial.print(x1a);
  Serial.print(x2a);

  Serial.print(y1a);
  Serial.print(y2a);
  delay(1000);
}

void displayHistory(MenuItem*){
  
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
  delay(150);
}
