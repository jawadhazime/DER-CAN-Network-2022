#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;

struct message {
  CAN_message_t msg;
  uint8_t checksum;
  uint8_t lowTemp=15, highTemp=20, avgTemp;
} bms;

CAN_message_t therm;

void genTemp(message &msg){
  msg.lowTemp++;
  msg.highTemp++;
  msg.avgTemp = (msg.lowTemp + msg.highTemp)/2;
}

void checkSum(message &sum){
  sum.checksum = 0;
  for (int i = 0; i < 7; i++){
    sum.checksum += sum.msg.buf[i];  // Sum of the data on bytes 1-7
  }
  sum.checksum += (57+8); // Add 0x39 (ID) & Data Length
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

int thermistor_read = 0; //cycles through thermistors #0-71

void loop() {
  Can0.events();

  static uint32_t timeout = millis();
  if ( millis() - timeout > 100 ) { // send frame(s) every 100ms

    //Thermistor Module -> BMS Broadcast : ID 0x1839F380
    bms.msg.id = 0x1839F380; // module #1
    bms.msg.flags.extended = 1;

    genTemp(bms);
    bms.msg.buf[0] = 0;  // module #1
    bms.msg.buf[1] = bms.lowTemp; // random low data
    bms.msg.buf[2] = bms.highTemp; // random high data
    bms.msg.buf[3] = bms.avgTemp; // poor avg calc
    bms.msg.buf[4] = 72; // number of therms
    bms.msg.buf[5] = 71; // Highest thermistor ID (zero based) on module #1
    bms.msg.buf[6] = 0; // Lowest thermistor ID (zero based) on module #1

    checkSum(bms);
    bms.msg.buf[7] = bms.checksum;  // checksum, we need to figure out how to do this

    Can0.write(bms.msg);
    canSniff(bms.msg);

    //Thermistor General Broadcast : ID 0x1838F380
    therm.id = 0x1838F380;
    therm.flags.extended = 1;

    therm.buf[0] = 0;  // Thermistor ID - MSB (>255)
    therm.buf[1] = thermistor_read; // Thermistor ID - LSB (0-255)
    therm.buf[2] = 0; // Thermistor value - TBD
    therm.buf[3] = 72; // number of therms
    therm.buf[4] = bms.lowTemp; // Lowest thermistor value
    therm.buf[5] = bms.highTemp; // Highest thermistor value
    therm.buf[6] = 71; // Highest thermistor ID (zero based) on module #1
    therm.buf[7] = 0; // Lowest thermistor ID (zero based) on module #1
    
    Can0.write(therm);
    canSniff(therm);

    thermistor_read++;
    if (thermistor_read > 71) // therm #71 is the last, reset counter to 0
      thermistor_read = 0;

    timeout = millis();

  }
}
