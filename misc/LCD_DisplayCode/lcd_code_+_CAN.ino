// Import the Liquid Crystal library
#include <LiquidCrystal.h>
//Initialise the LCD with the arduino. LiquidCrystal ( rs, enable, d4, d5, d6, d7);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

 int temperature;
 int speedc;
 int n =0;
 const byte interruptPin = 21;
 volatile int BFault = LOW;
 int i = 0;
 bool BMS,IMD,BSPD;

void setup() {

  Serial.begin(115200); delay(400);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW); // enable transceiver
  Can0.begin();
  Can0.setClock(CLK_60MHz);
  Can0.setBaudRate(250000);
  Can0.setMaxMB(16); // up to 64 max for T4, not important in FIFO mode, unless you want to use additional mailboxes with FIFO
  Can0.enableFIFO();
  Can0.enableFIFOInterrupt();
  Can0.onReceive(canSniff);
  Can0.mailboxStatus();
 
  //pinMode(14, OUTPUT); //set pin 14 as output
  
  pinMode(interruptPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(interruptPin), BMSfault, RISING);
 
  lcd.begin(8, 2);// Switch on the LCD screen
  
  lcd.print("DashBoard");
  lcd.setCursor(0,1);
  lcd.print("d");
  delay(1500);
}

void DisplayTempSpeed(int temperature, int speedc){

  lcd.setCursor(0,1);
  lcd.print("TEMP:");
  lcd.setCursor(5, 1);
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

struct potentiometer {
  int val = 0;
  float msg = 0;
} pot;

void canSniff(const CAN_message_t &msg) {
  int message_val; //store value being sent as int 
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
    if (i == 0) {
      message_val = msg.buf[i];
      }
  } Serial.println();
    Serial.println("Your value is: "); Serial.print(message_val, HEX);

}

void loop() {
  lcd.clear();

 temperature = random(0,35);
  speedc = random(20, 100);
    //temperature = 0+i;
  //speedc = 10+i;
  DisplayTempSpeed(temperature, speedc);
 // digitalWrite(14, BFault);
/*
  if(n==1){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Fault");
      }
    while(n==1){
      delay(100);
     }

  //lcd.print("10 Seconds Clock");
  for( int i = 0; i < 10; i++){
    delay(1000);
  lcd.setCursor(7, 1);
  //lcd.setCursor(0, (1*millis()/1000));
   //print the number of seconds since reset:
  lcd.print(i);
  lcd.print(millis()/1000);
  }
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("10");
  lcd.setCursor(1,1);
  lcd.print("seconds");
  delay(750);
  */
  Can0.events();

    CAN_message_t msg;
    if (Can0.read(msg)) {
    Serial.print("CAN1 "); 
    Serial.print("MB: "); Serial.print(msg.mb);
    Serial.print("  ID: 0x"); Serial.print(msg.id, HEX );
    Serial.print("  EXT: "); Serial.print(msg.flags.extended );
    Serial.print("  LEN: "); Serial.print(msg.len);
    Serial.print(" DATA: ");
    for ( uint8_t i = 0; i < 8; i++ ) {
      Serial.print(msg.buf[i]); Serial.print(" ");
    }
    Serial.print("  TS: "); Serial.println(msg.timestamp);
    }
}
