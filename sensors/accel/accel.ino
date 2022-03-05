#include "FlexCAN_T4.h"
#define MESSAGE_SIZE 8
// Need to check that CAN bus is correctly assigned
FlexCAN_T4 <CAN1, RX_SIZE_256, TX_SIZE_16> Can0; // DATA
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> CanData; // CRITICAL

// create two arrays for the CAN messages
int vehicleDynamics_ID = 0; 
int vehicleDynamics[8]; // goes to critical CAN : contains shock vals, accelerometer,  & steering angle

int FaultMessages_ID = 0;
int FaultMessages[3]; // goes to Data CAN & logs system faults

// Configure digital pins as input
int BSPD = 10;
int BMS = 11;
int IMD = 12;
  

void canSniff(const CAN_message_t &msg) {
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);

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
  pinMode(BSPD, INPUT); // T_BMS
  pinMode(BMS, INPUT); // MOSI?
  pinMode(IMD, INPUT); // MISO?
  Serial.begin(115200); delay(400);
  pinMode(6, OUTPUT); // digitalWrite(6, LOW); // enable transceiver
  // Initialize CAN 1 (data or critical???) 
  Can0.begin();
  Can0.setClock(CLK_60MHz);
  Can0.setBaudRate(250000);
  Can0.setMaxMB(16); // up to 64 max for T4, not important in FIFO mode, unless you want to use additional mailboxes with FIFO
  Can0.enableFIFO();
  Can0.enableFIFOInterrupt();
  Can0.onReceive(canSniff);
  Can0.mailboxStatus();
 
// Initialize CAN 2 (data or critical???) 
  CanData.begin();
  CanData.setClock(CLK_60MHz);
  CanData.setBaudRate(250000);
  Can0.setMaxMB(16);
  CanData.enableFIFO();
  CanData.enableFIFOInterrupt();
  CanData.onReceive(canSniff); // this might cause problems, as Can0 uses this?
  CanData.mailboxStatus(); 
  // end setup



}

void loop() {
  Can0.events();
  CanData.events();

  // Get analog samples from pins
  vehicleDynamics[0] = ten2eight(10); // steering angle
  vehicleDynamics[1] = ten2eight(A6); // FR shock
  vehicleDynamics[2] = ten2eight(A5); // FL shock 
  vehicleDynamics[3] = ten2eight(A4); // RR shock
  vehicleDynamics[4] = ten2eight(A3); // RL shock
  vehicleDynamics[5] = ten2eight(A2); // AccelX
  vehicleDynamics[6] = ten2eight(A1); // AccelY
  vehicleDynamics[7] = ten2eight(A0); // AccelZ
  
  // get digital inputs for faults  
  FaultMessages[0] = digitalRead(BSPD); // BSPD HIGH
  FaultMessages[1] = digitalRead(BMS); // BMS FAULT
  FaultMessages[2] = digitalRead(IMD); // IMD
  delay(250);

  // Send torque values to motor controller from linear potentiometer
    static uint32_t timeout = millis();
    if ( millis() - timeout > 20 ) { // send random frame every 20ms
      CAN_message_t msg;
      msg.id = 0x0FA;
      for ( uint8_t i = 0; i < MESSAGE_SIZE; i++ ) {
       msg.buf[i] = vehicleDynamics[i]; 
	}
  
     Can0.write(msg);
     canSniff(msg);

     msg.id = 0x0FB;
     for (uint8_t i = 0; i < MESSAGE_SIZE; i++) {
      if (i < 3) 
      {
	      msg.buf[i] = FaultMessages[i];
      } 
	    else 
	    {
	    msg.buf[i] = 0;
	    } 
      CanData.write(msg);
      canSniff(msg); 
      timeout = millis();
      // end loop
    }
  }
}
