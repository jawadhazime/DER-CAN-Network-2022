#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;

struct message {
  CAN_message_t msg;
  uint8_t checksum;
  uint8_t lowTemp=15, highTemp=20, avgTemp;
} bms;

void genTemp(message &msg){
  msg.lowTemp++;
  msg.highTemp++;
  msg.avgTemp = (msg.lowTemp + msg.highTemp)/2;
}

void checkSum(message &sum){
  sum.checksum = 0;
  for (int i = 0; i < 6; i++){
    sum.checksum += sum.msg.buf[i];  // 1. Sum of the data
  }
  sum.checksum += (57+8); // Add 0x39 & Data Length
}

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
  if ( millis() - timeout > 2000 ) { // send random frame every 2000ms

    bms.msg.id = 0x1839F380; // module #2
    bms.msg.flags.extended = 1;

    genTemp(bms);
    bms.msg.buf[0] = 0;  // module #2
    bms.msg.buf[1] = bms.lowTemp; // random low data
    bms.msg.buf[2] = bms.highTemp; // random high data
    bms.msg.buf[3] = bms.avgTemp; // poor avg calc
    bms.msg.buf[4] = 72; // number of therms
    bms.msg.buf[5] = 0;
    bms.msg.buf[6] = 0;

    checkSum(bms);
    bms.msg.buf[7] = bms.checksum;  // checksum, we need to figure out how to do this

    
    Can0.write(bms.msg);
    timeout = millis();
    canSniff(bms.msg);
  }
}
