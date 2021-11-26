#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

// something sus: "empty" message of all zeroes sends between full messages
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

float ten2eight(int tenBit) {
  int eightBit = tenBit >> 2;
  return eightBit;
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

  // Convert potentiometer reads from Pin0 from 10bit to 8bit to send via CAN
 // pot.val = analogRead(A6);
 // pot.msg = (int)ten2eight(pot.val);
 // Serial.print(pot.msg);
  delay(250);

  // Send torque values to motor controller from linear potentiometer
  static uint32_t timeout = millis();
  if ( millis() - timeout > 20 ) { // send random frame every 20ms
    CAN_message_t msg;
    if ( Can0.read(msg) ) {
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

    msg.id = 0xD0;

    Can0.read(msg);
    timeout = millis();
    canSniff(msg);

    // end loop

  }
}
