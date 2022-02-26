#include "FlexCAN_T4.h"
#define MESSAGE_SIZE 8
FlexCAN_T4 <CAN1, RX_SIZE_256, TX_SIZE_16> Can0;
FlexCAN_T4 <CAN2, RX_SIZE_265, TX_SIZE_16> Can1;

// create two arrays for the CAN messages
int VehicleDynamics_ID = NULL; 
int VehicleDynamics[8]; // goes to critical CAN : contains shock vals, accelerometer,  & steering angle

int FaultMessages_ID = NULL;
int FaultMessages[3]; // goes to Data CAN & logs system faults

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

int ten2eight(int tenBit) {
  int eightBit = tenBit >> 2;
  return eightBit;
}

// NEEDS TO BE DONE: setting digital pins as input & outputs 
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

  // Get analog samples from pins
  vehicleDynamics[0] = ten2eight(%INSERT NUMBER HERE); // steering angle
  vehicleDynamics[1] = ten2eight(%INSERT NUMBER HERE); // FR shock
  vehicleDynamics[2] = ten2eight(%INSERT NUMBER HERE); // FL shock 
  vehicleDynamics[3] = ten2eight(%INSERT NUMBER HERE); // RR shock
  vehicleDynamics[4] = ten2eight(%INSERT NUMBER HERE); // RL shock
  vehicleDynamics[5] = ten2eight(%INSERT NUMBER HERE); // AccelX
  vehicleDynamics[6] = ten2eight(%INSERT NUMBER HERE); // AccelY
  vehicleDynamics[7] = ten2eight(%INSERT NUMBER HERE); // AccelZ
  
  // get digital inputs for faults  
  FaultMessages[0] =  // BSPD HIGH
  FaultMessages[1] = // BMS FAULT
  FaultMessages[2] = // IMD
  delay(250);

  //  // Send torque values to motor controller from linear potentiometer
  //  static uint32_t timeout = millis();
  //  if ( millis() - timeout > 20 ) { // send random frame every 20ms
  //    CAN_message_t msg;
  //    msg.id = 0x0C0;
  //    for ( uint8_t i = 0; i < 8; i++ ) {
  //      if (i == 0) {
  //        msg.buf[i] = (uint8_t)pot.msg;
  //        // test case for sending a constant value is msg.buf[i] = 5;
  //      }
  //      else if (i == 4) {
  //        msg.buf[i] = 1;
  //      }
  //      else if (i == 5) {
  //        msg.buf[i] = 1;
  //      }
  //      else {
  //        msg.buf[i] = 0;
  //      }
  //    }
  //
  //    Can0.write(msg);
  //    timeout = millis();
  //    canSniff(msg);
  //
  //    // end loop
  //  }
}
