// Import the Liquid Crystal library
#include <LiquidCrystal.h>
//Initialise the LCD with the arduino. LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);


 const byte interruptPin = 21;
 volatile int BFault = LOW;
 int i = 0;
 bool BMS,IMD,BSPD;

 
void setup() {
 // pinMode(14, OUTPUT); //set pin 14 as output
  
  //pinMode(interruptPin, INPUT_PULLDOWN);
  //attachInterrupt(digitalPinToInterrupt(interruptPin), BMSfault, RISING);
 
  lcd.begin(8, 2);// Switch on the LCD screen
  lcd.print("hello world");
  
  
}






void loop() {
  lcd.clear();
  lcd.print("DashBoard");
  lcd.setCursor(0,1);
  // lcd.print("d");
  delay(1500);
  lcd.autoscroll();



}
