#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;

struct hall {
  int val = 0;
  float msg = 0;
  bool up = false;
  bool down = false;
  float count = 0;
  float spd = 0;
} ws;

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

float ten2eight(int tenBit){
  float eightBit = 0;
  float num = (float)tenBit;
  eightBit = (num/1023)*255;
  return eightBit;
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

  // Convert potentiometer reads from Pin0 from 10bit to 8bit to send via CAN
  ws.val = analogRead(0);
  ws.msg = (int)ten2eight(ws.val);

  // Send wheel speed via critical bus every 1s.
  static uint32_t timeout = millis();

  if ( millis() - timeout > 10 ) { // count teeth every 10ms
    if (ws.msg == 255) {ws.up = true;}
    if (ws.msg == 0) {ws.down = true;}
    if (ws.up == true && ws.down == true){
      ws.count++;
      ws.up = false;
      ws. down = false;
    }
  }
  
  if ( millis() - timeout > 1000 ) { // send wheel speed ever second
    CAN_message_t msg;
    msg.id = 111;
    Serial.print("Value: ");
    Serial.print(ws.msg);
    Serial.print("\n");
    Serial.print("Tooth Count: ");
    Serial.print(ws.count);
    Serial.print("\n");
    Serial.print("Tooth Up: ");
    Serial.print(ws.up);
    Serial.print("\n");
    Serial.print("Tooth Down: ");
    Serial.print(ws.down);
    Serial.print("\n");
    ws.spd = (ws.count/52) * 8.27; // divide count by the number of teeth, and then multplie by the mph ratio
    for ( uint8_t i = 0; i < 8; i++ ) {
      if(i==0){msg.buf[i] = (uint8_t)ws.spd;}
      else if(i==4){msg.buf[i] = 1;}
      else if(i==5){msg.buf[i] = 1;}
      else{msg.buf[i] = 0;}
    }
    ws.count = 0;
    Serial.print("Wheel Speed: ");
    Serial.print(ws.spd);
    Serial.print("\n");

    Can0.write(msg);
    timeout = millis();
    canSniff(msg);

    // end loop
  } 
}
