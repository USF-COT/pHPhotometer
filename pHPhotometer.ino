#include <LiquidCrystal.h>   //use LCD library

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);// LCD pin

int potPin = 4; //detector pin
int i;

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

long vala = 0;  //"blank" button value
long valb = 0; //"sample" button value

int apin = 8;//pin8 blank button
int bpin = 9;//pin9 sample button

void setup(){
  Serial.begin(9600);

  lcd.begin(16, 2);  //Initialize LCD

  lcd.print("MiniSpec B.Y");  //Display Mini Spectrophotomete

  delay(1000); //Delay1000ms

  pinMode(apin,INPUT); //setup blank button
  pinMode(bpin,INPUT);//setup sample button
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
}

void loop (){
  vala = digitalRead(apin); //read "blank" button

  if (vala==HIGH){
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

    lcd.clear(); //clear

    lcd.print("Blank(");
    lcd.print(x1a);
    lcd.print(")");

    lcd.setCursor(0, 1) ;
    lcd.print("Blank(");
    lcd.print(y1a);
    lcd.print(")"); 
  }
  
  valb = digitalRead(bpin);//read sample button
  if (valb==HIGH)
  {
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
}
