#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;

struct message {
  CAN_message_t msg;
  int num;
} b, d, e, t, v; // broadcast, disable, enable, torque, vehicleStateMachine

//-----------------------------------------

// Helper Variables
int start = 0; // Ensures Enable Message is sent only once
int potVal = 0; // Initial Analog Value from Sensor
float potMsg = 0; // Can Value from Sensor packaged for CAN
bool waitReady = false; // VSM Wait State
bool torqReady = false; // Ready to send Torque Values
static uint32_t timeout = 0; // Initial timeout value

//-----------------------------------------

// Converts analog input from 10bit to 8bit for packaging as a CAN Message
float ten2eight(int tenBit){
  float eightBit = 0;
  float num = (float)tenBit;
  eightBit = (num/1023)*255;
  return eightBit;
}

//=====================================================================================================================================

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

  // Set Vehicle State Machine Message from the Motor Controllers
  if(msg.id == 0x0AA){
    v.msg = msg;
    v.num = msg.buf[0];
    if(v.num == 4){waitReady = true;}
  }
}

//=====================================================================================================================================

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
  
  delay(250);
  
  //-----------------------------------------
    // Enable broadcasts
    b.msg.id = 0x0C1;
    b.msg.buf[0] = 148;
    b.msg.buf[1] = 0;
    b.msg.buf[2] = 1;
    b.msg.buf[3] = 0;
    b.msg.buf[4] = 255;
    b.msg.buf[5] = 255;
    b.msg.buf[6] = 255;
    b.msg.buf[7] = 255;
    Can0.write(b.msg);
    canSniff(b.msg);

  //-----------------------------------------
  
  // end setup
}

//=====================================================================================================================================

void loop() {
  Can0.events();
  
  // send frame every 20ms
  timeout = millis();
  if ( millis() - timeout > 20 ) {
    Serial.print(timeout);
    //-----------------------------------------
  
    // Disable the inverter waits for Vehicle State Machine to send waitReady (as 4)
    if (waitReady == false) {
        d.msg.id = 0x0C0;
        for ( uint8_t i = 0; i < 8; i++ ) {
          d.msg.buf[i] = 0;
        }
        Can0.write(d.msg);
        canSniff(d.msg);
    }
    
    //-----------------------------------------
    
    // Send the enable command to the inverter before sending torque values in the loop
    if (start == 0 && waitReady == true){
      e.msg.id = 0x0C0;
      for ( uint8_t i = 0; i < 8; i++ ) {
        if(i==5){e.msg.buf[i]=1;}
        else{e.msg.buf[i] = 0;}
      }
      Can0.write(e.msg);
      canSniff(e.msg);
      start = 1;
      torqReady = true;
    }
  
    //-----------------------------------------
  
    
    // Send torque values to motor controller from linear potentiometer
    if (torqReady == true){
      // Convert potentiometer reads from Pin0 from 10bit to 8bit to send via CAN
      potVal = analogRead(0);
      potMsg = (int)max((ten2eight(potVal)/51),1); // Limit torque to 1 to 5Nm
      t.msg.id = 0x0C0;
      for ( uint8_t i = 0; i < 8; i++ ) {
        if(i==0){t.msg.buf[i] = (uint8_t)potMsg;}
        else if(i==4){t.msg.buf[i] = 1;} // reverse mode is 0, forward mode is 1
        else if(i==5){t.msg.buf[i] = 1;}
        else{t.msg.buf[i] = 0;}
      }
      Can0.write(t.msg);
      timeout = millis();
      canSniff(t.msg);
    }
    
    //-----------------------------------------
    // end loop
  
  }
}
