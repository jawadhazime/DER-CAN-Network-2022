
// Import the Liquid Crystal library
#include <LiquidCrystal.h>
//Initialise the LCD with the arduino. LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

 int temperature;
 int speedc;
 int n =0;
 const byte interruptPin = 21;
 volatile int BFault = LOW;
 int i = 0;
 bool BMS,IMD,BSPD;

 
void setup() {
 // pinMode(14, OUTPUT); //set pin 14 as output
  
  //pinMode(interruptPin, INPUT_PULLDOWN);
  //attachInterrupt(digitalPinToInterrupt(interruptPin), BMSfault, RISING);
 
  lcd.begin(8, 2);// Switch on the LCD screen
  
  lcd.print("DashBoard");
  lcd.setCursor(0,1);
  lcd.print("d");
  delay(1500);
}



void DisplayTempSpeed(int temperature, int speedc){

  
  lcd.setCursor(0,1);
  lcd.print("TEMP: ");
  lcd.setCursor(6, 1);
  lcd.print(temperature);
 
   
  lcd.setCursor(0,0);
  lcd.print("SPEED: ");
  lcd.setCursor(6, 0);
  lcd.print(speedc);
  i++;
  
   

delay(1500);
  return;
}


void BMSfault(){
 BFault = HIGH;
 BMS = true;

}

void loop() {
  lcd.clear();

 temperature = random(0,35);
  speedc = random(20, 100);
    temperature = 0+i;
  speedc = 10+i;
  DisplayTempSpeed(temperature, speedc);
  digitalWrite(14, BFault);


}

if(n==1){

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Fault");
}
while(n==1){
  delay(100);
}


  
  lcd.print("10 Seconds Clock");
  for( int i = 0; i < 10; i++){
    delay(1000);
  lcd.setCursor(i, 1);
  lcd.setCursor(0, (1*millis()/1000));
   print the number of seconds since reset:
  lcd.print(i);
  lcd.print(millis()/1000);
  }
  lcd.clear();
  lcd.print("10 Seconds");
  delay(1000);
  

void TempDisplay(int temperature){
  lcd.setCursor(0,1);
  lcd.print("TEMP: ");
  lcd.setCursor(6, 1);
  lcd.print(temperature);
  delay(100);
  return;
}

void SpeedDisplay(int speedc){
  lcd.setCursor(0,0);
  lcd.print("SPEED: ");
  lcd.setCursor(6, 0);
  lcd.print(speedc);
  delay(100);
  return;
}
