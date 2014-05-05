#include <LiquidCrystal.h>   //use LCD library
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);// LCD pin

struct MENU{
  unsigned int selectedIndex;
  char** menuItems;
  MenuFuncPtr* functions;
}

const int NUM_MENU_OPTIONS = 2;
typedef void (* MenuFuncPtr) ();
char* sampleStrings[NUM_MENU_OPTIONS] = {
  "Record Blank",
  "Record Sample"
};

MenuFuncPtr sampleFunctions[NUM_MENU_OPTIONS] = {
  recordBlank,
  recordSample
};

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
}

int x1[10];   //blank
int y1[10];  //blank

int x2[10];//sample
int y2[10];//sample

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

  for(i=1; i<11; i++){
    x1[i]=analogRead(potPin);
  }
    
  x1a=(x1[1]+x1[2]+x1[3]+x1[4]+x1[5]+x1[6]+x1[7]+x1[8]+x1[9]+x1[10])/10 ; //read the blank

  digitalWrite(7,LOW);
  digitalWrite(6,HIGH);

  delay(2000);

  for(i=1; i<11; i++){
    y1[i]=analogRead(potPin);
  }

  y1a=(y1[1]+y1[2]+y1[3]+y1[4]+y1[5]+y1[6]+y1[7]+y1[8]+y1[9]+y1[10])/10 ; //read the blank

  digitalWrite(6,LOW);
}

void recordBlank(){
  lcd.clear();
  lcd.print("Recording");
  lcd.setCursor(0, 1);
  lcd.print("Blank...");
  
  calculateBlank();

  lcd.clear(); //clear
  
  const int NUM_RESULT_LINES;
  char* resultLines[NUM_RESULT_LINES];
  
  resultLines[0] = "Blank(" + x1a + ")";
  resultLines[1] = "Blank(" + y1a + ")";
  
  navigateResultLines(resultLines, NUM_RESULT_LINES);

  lcd.print("Blank(");
  lcd.print(x1a);
  lcd.print(")");

  lcd.setCursor(0, 1) ;
  lcd.print("Blank(");
  lcd.print(y1a);
  lcd.print(")"); 
}

void recordSample(){
  lcd.clear();
  lcd.print("Recording");
  lcd.setCursor(0, 1);
  lcd.print("Sample...");
  digitalWrite(7,HIGH);

  delay(2000);

  for(i=1; i<11; i++){
    x2[i]=analogRead(potPin);
  }
  x2a=(x2[1]+x2[2]+x2[3]+x2[4]+x2[5]+x2[6]+x2[7]+x2[8]+x2[9]+x2[10])/10 ; //read the sample

  Serial.print(x1a);
    
  digitalWrite(7,LOW);
  digitalWrite(6,HIGH);
  delay(2000);

  for(i=1; i<11; i++){
    y2[i]=analogRead(potPin);
  }

  y2a=(y2[1]+y2[2]+y2[3]+y2[4]+y2[5]+y2[6]+y2[7]+y2[8]+y2[9]+y2[10])/10 ; //read the sample

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

int selectedMenuIndex = 0;



MENU currentMenu

char** menuItems = sampleStrings;
MenuFuncPtr* functions = sampleFunctions;

void updateMenu(int offset){
  
  lcd.print(menuStrings[selectedMenuIndex]);
}

void loop(){
  int lcd_key = read_LCD_buttons();
  
  switch (lcd_key){
    case btnSELECT:{
      functions[selectedMenuIndex]();
      break;
    }
    case btnUP:{
      if (++selectedMenuIndex >= NUM_MENU_OPTIONS)
        selectedMenuIndex = 0;
      updateMenu();
      break;
    }
    case btnDOWN:{
      if (--selectedMenuIndex < 0)
        selectedMenuIndex = NUM_MENU_OPTIONS - 1;
      updateMenu();
      break;
    }
    case btnLEFT:{
      menuItems = sampleStrings;
      functions = sampleFunctions;
    }
  }
  delay(150);
}
