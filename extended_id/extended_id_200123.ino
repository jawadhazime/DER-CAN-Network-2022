#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;

struct message {
  CAN_message_t msg;
} b0, b1, b2, b3;

void canSniff(const CAN_message_t &msg) {
  Serial.print(" MB "); Serial.print(msg.mb);
  Serial.print(" OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print(" LEN: "); Serial.print(msg.len);
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
}

void loop() {
  Can0.events();

  static uint32_t timeout = millis();
  if ( millis() - timeout > 100 ) { // send random frame every 100ms
    
    b0.msg.id = 0x1839F380; // module #1
    b0.msg.flags.extended = 1;
    b1.msg.id = 0x1839F380; // module #1
    b1.msg.flags.extended = 1;

    b0.msg.buf[0] = 0;  // module #1
    b0.msg.buf[1] = 20; // fake temp data
    b0.msg.buf[2] = 50; // fake temp data
    b0.msg.buf[3] = 25; // fake temp data
    b0.msg.buf[4] = 72; // fake temp data
    b0.msg.buf[5] = 0;
    b0.msg.buf[6] = 0;
    b0.msg.buf[7] = 232;  // checksum, we need to figure out how to do this
    
    b1.msg.buf[0] = 1;  // module #1
    b1.msg.buf[1] = 20; // fake temp data
    b1.msg.buf[2] = 50; // fake temp data
    b1.msg.buf[3] = 25; // fake temp data
    b1.msg.buf[4] = 72; // fake temp data
    b1.msg.buf[5] = 0;
    b1.msg.buf[6] = 0;
    b1.msg.buf[7] = 232;  // checksum, we need to figure out how to do this

    //----------------------------------------------------------------------

    b2.msg.id = 0x1839F381; // module #2
    b2.msg.flags.extended = 1;
    b3.msg.id = 0x1839F381; // module #2
    b3.msg.flags.extended = 1;

    b2.msg.buf[0] = 1;  // module #2
    b2.msg.buf[1] = 20; // fake temp data
    b2.msg.buf[2] = 50; // fake temp data
    b2.msg.buf[3] = 25; // fake temp data
    b2.msg.buf[4] = 72; // fake temp data
    b2.msg.buf[5] = 0;
    b2.msg.buf[6] = 0;
    b2.msg.buf[7] = 232;  // checksum, we need to figure out how to do this

    // b3.msg.buf[0] = 2;  // module #2
    // b3.msg.buf[1] = 20; // fake temp data
    // b3.msg.buf[2] = 50; // fake temp data
    // b3.msg.buf[3] = 25; // fake temp data
    // b3.msg.buf[4] = 72; // fake temp data
    // b3.msg.buf[5] = 0;
    // b3.msg.buf[6] = 0;
    // b3.msg.buf[7] = 232;  // checksum, we need to figure out how to do this
    
    Can0.write(b0.msg);
    Can0.write(b1.msg);
    Can0.write(b2.msg);
    Can0.write(b3.msg);
    timeout = millis();
    canSniff(b0.msg);
    canSniff(b1.msg);
    canSniff(b2.msg);
    canSniff(b3.msg);
  }
}
