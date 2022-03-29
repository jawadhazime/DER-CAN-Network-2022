 #include <LiquidCrystal.h>
// Initiialize the LCD with the teensy
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;

// Display variables
int temperature;
int speedM;
float SOC;

// Helper variables
int n = 0;
int i = 0;
int start = 0;
const byte interruptPin = 21;
volatile int BFault = LOW;
bool waitReady = false; // VSM Wati state
bool BMS, IMD, BSPD;

//---------------------------------------------------------------------------

void DisplayTempSpeed(int temperature, int speedM) {

   lcd.setCursor(0,1);
   lcd.print ("Temp: ");
   lcd.setCursor(5,1);
   lcd.print(temperature);

   lcd.setCursor(0,0);
   lcd.print("Speed: ");
   lcd.setCursor(6,0);
   lcd.print(speedM);
   i++;
}

//---------------------------------------------------------------------------

void DisplaySOC(float SOC){
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SOC: ");   
  lcd.setCursor(5,0);
  lcd.print(SOC);
}

//---------------------------------------------------------------------------
/*
void BMSfault(){
 BFault = HIGH;
 BMS = true;
}

if(n==1){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Fault");
      }
    while(n==1){
      delay(100);
     }

digitalWrite(14, BFault);
*/
//---------------------------------------------------------------------------

struct potentiometer {
  int val = 0;
  float msg = 0;
} pot;

//---------------------------------------------------------------------------

struct message {
  CAN_message_t msg;
  int num;
} ws;

//---------------------------------------------------------------------------

void canSniff(const CAN_message_t &msg) {

int message_val; //store value sent as int
Serial.print(" MB "); Serial.print(msg.mb);
  Serial.print(" OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print(" LEN: "); Serial.print(msg.len);
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

  // Set vehucle state machine message from the motor controller
  if(msg.id == 0x0AA){
    ws.msg = msg;
    ws.num = message_val;
    if(ws.num == 4){waitReady = true;}
  }


}



//---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200); delay(400);
  pinMode(6, OUTPUT);// digitalWrite(6, LOW); // enable transceiver
  Can0.begin();
  Can0.setClock(CLK_60MHz);
  Can0.setBaudRate(500000);
  Can0.setMaxMB(16); // up to 64 max for T4, not important in FIFO mode, unless you want to use additional mailboxes with FIFO
  Can0.enableFIFO();
  Can0.enableFIFOInterrupt();
  Can0.onReceive(canSniff);
  Can0.mailboxStatus();

  delay(250);

    pinMode(14, OUTPUT); //set pin 14 as output

   // pinMode(interruptPin, INPUT_PULLDOWN);
    //attachInterrupt(digitalPinToInterrupt(interruptPin), BMSfault, RISING);
 
  lcd.begin(8, 2);// Switch on the LCD screen
  
  lcd.print("DashBoard");
  lcd.setCursor(0,1);
  lcd.print("d");
  delay(1500);  
  lcd.clear();
}

//---------------------------------------------------------------------------

void loop() {
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
 Can0.read(msg); 
  //speedM = ws.msg.id;
  canSniff(msg);

  
  temperature = 5;
  speedM= ws.num;
  DisplayTempSpeed(temperature, speedM);
  
}

/* what code needs to do 

display mean wheel speed

display mean battery temperature

display SOC on an interval

fault detection and display



 */
