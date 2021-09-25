#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;

struct potentiometer {
  int val = 0;
  float tx = 0;
} pot;

struct message {
  CAN_message_t msg;
} t, d, e, c;

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

   // Disable the inverter-lockout for 500ms seconds for pre-charge to be able to complete initiation
  for (int t = 0; t<2000;t++){
    d.msg.id = 0x0C0;
    for ( uint8_t i = 0; i < 8; i++ ) {
      d.msg.buf[i] = 0;
    }
    Can0.write(d.msg);
    canSniff(d.msg);
  }
}

void loop() {
  Can0.events();

  pot.val = analogRead(0);
  pot.tx = (int)ten2eight(pot.val);

  static uint32_t timeout = millis();
  if ( millis() - timeout > 8 ) { // send random frame every 8ms
    t.msg.id = 0x0C0;
    for ( uint8_t i = 0; i < 8; i++ ) {
      if(i==0){t.msg.buf[i] = (uint8_t)pot.tx;}
      else if(i==4){t.msg.buf[i] = 1;}
      else if(i==5){t.msg.buf[i] = 1;}
      else{t.msg.buf[i] = 0;}
    }
    e.msg.id = 0x0C1;
    e.msg.buf[0]=148;
    e.msg.buf[1]=0;
    e.msg.buf[2]=1;
    e.msg.buf[3]=0;
    e.msg.buf[4]=255;
    e.msg.buf[5]=255;
    e.msg.buf[6]=255;
    e.msg.buf[7]=255;

    c.msg.id = 0x404;
    c.msg.buf[0]=0;
    c.msg.buf[1]=0;
    c.msg.buf[2]=0;
    c.msg.buf[3]=0;
    c.msg.buf[4]=75;
    c.msg.buf[5]=55;
    c.msg.buf[6]=0;
    c.msg.buf[7]=0;
    
    Can0.write(t.msg);
    Can0.write(e.msg);
    // Can0.write(c.msg);
    timeout = millis();
    canSniff(e.msg);
    canSniff(t.msg);
    canSniff(c.msg);
  }
}
