#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;


void canSniff(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}

void setup(void) {
  Serial.begin(115200); delay(400);
  pinMode(6, OUTPUT); digitalWrite(6, LOW); // enable transceiver
  Can0.begin();
  Can0.setClock(CLK_60MHz);
  Can0.setBaudRate(500000);
  Can0.setMaxMB(16); // up to 64 max for T4, not important in FIFO mode, unless you want to use additional mailboxes with FIFO
  Can0.enableFIFO();
  Can0.enableFIFOInterrupt();
  Can0.onReceive(canSniff);
  Can0.mailboxStatus();

  // end setup
}

void loop() {
  Can0.events();

  // Send inverter disable msg over CAN constantly
  static uint32_t timeout = millis();
  if ( millis() - timeout > 20 ) { // send random frame every 20ms
    CAN_message_t msg;
    msg.id = 0x0C0;
    for ( uint8_t i = 0; i < 8; i++ ) {
      for ( uint8_t i = 0; i < 8; i++ ) {
        msg.buf[i] = 0;
      }
    }
    
    Can0.write(msg);
    timeout = millis();
    
    canSniff(msg);

    // end loop
  } 
}
