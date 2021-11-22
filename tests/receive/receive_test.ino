#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

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


void setup(void) {
  Serial.begin(115200); delay(400);
  pinMode(6, OUTPUT); // digitalWrite(6, LOW); // enable transceiver
  Can0.begin();
  Can0.setClock(CLK_60MHz);
  Can0.setBaudRate(250000);
  Can0.setMaxMB(16); // up to 64 max for T4, not important in FIFO mode, unless you want to use additional mailboxes with FIFO
  Can0.enableFIFO();
  Can0.enableFIFOInterrupt();
  Can0.onReceive(canSniff);
  Can0.mailboxStatus();

  // end setup
}

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


//    Can0.read(msg);
//    canSniff(msg);


  }
}
