#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;

struct message {
  CAN_message_t msg;
  uint8_t checksum;
  uint8_t lowTemp=15, highTemp=20, avgTemp;
} bms;

CAN_message_t therm;

//array of input pins connected to the multiplexers
int muxInputPin[9] = {A1, A2, A3, A4, A5, A6, A7, A8, A9};

//array of values and pin numbers for the mux control signals
int control[3] = {0,0,0}; 
int controlPin[3] = {5, 4, 3};
                  //{A, B, C} Most significant bit is C

//matrix of indices to sort thermistor values
int therm_index[9][8] = 
{
  {6, 7, 8, 5, 1, 4, 2, 3},// Mux 1
  {14, 9, 13, 10, 12, 15, 16, 11},// Mux 2
  {22, 23, 24, 21, 17, 20, 18, 19},// Mux 3
  {27, 26, 25, 28, 32, 29, 31, 30},// Mux 4
  {38, 39, 40, 37, 33, 36, 34, 35},// Mux 5
  {43, 42, 41, 44, 48, 45, 47, 46},// Mux 6
  {53, 55, 56, 54, 49, 52, 50, 51},// Mux 7
  {62, 57, 61, 58, 60, 63, 64, 59},// Mux 8
  {70, 71, 72, 69, 65, 68, 66, 67} // Mux 9
};

double val = 0.0000;
double avgADC[72];

int thermistor_read = 0; //cycles through thermistors #0-71

//----------------------------------------------------------------------------------------------------------------------------
void setMuxControl(int i)
{
  //set ABC mux control inputs
  control[0] = i % 2; //LSB
  control[1] = (i / 2) % 2;
  control[2] = (i / 4) % 2; //MSB

  //digital write mux control inputs
  digitalWrite(controlPin[0], control[0]);
  // A = control[0] LEAST SIGNIFICANT
  digitalWrite(controlPin[1], control[1]);
  // B = control[1]
  digitalWrite(controlPin[2], control[2]);
  // C = control[2] MOST SIGNIFICANT
}

//----------------------------------------------------------------------------------------------------------------------------
void getMuxValues()
{
  //read in cell voltages and add them to avgADC
  for(int l = 0; l<500; l++){
    
    for(int j = 0; j < 8; j++)
    {
      //set active mux output
      setMuxControl(j);
      delay(1);
      
      for(int i = 0; i < 9; i++)
      {     
        //read in j'th voltage from i'th mux and convert to voltage
        val = analogRead(muxInputPin[i]);
  
        avgADC[therm_index[i][j] - 1] +=  val;
      }
    }
  }
  
  for(int i = 0; i<72; i++){
    avgADC[i] = avgADC[i]/500.0;
  }
}

//----------------------------------------------------------------------------------------------------------------------------
void getTemp(message &msg){
  
  getMuxValues();
  
  
  msg.lowTemp++;
  msg.highTemp++;
  msg.avgTemp = (msg.lowTemp + msg.highTemp)/2;
}

//----------------------------------------------------------------------------------------------------------------------------
void checkSum(message &sum){
  sum.checksum = 0;
  for (int i = 0; i < 7; i++){
    sum.checksum += sum.msg.buf[i];  // Sum of the data on bytes 1-7
  }
  sum.checksum += (57+8); // Add 0x39 (ID) & Data Length
}

//----------------------------------------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(115200); 
  delay(400);
  
  //set voltage mux pins as inputs
  for(int i = 0; i < 9; i++)
  {
    pinMode(muxInputPin[i], INPUT);
  }

  //set mux controls as output
  for(int i = 0; i < 3; i++)
  {
    pinMode(controlPin[i], OUTPUT);
  }

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

//----------------------------------------------------------------------------------------------------------------------------
void loop() {
  Can0.events();

  static uint32_t timeout = millis();
  if ( millis() - timeout > 100 ) { // send frame(s) every 100ms

    //Thermistor Module -> BMS Broadcast : ID 0x1839F380
    bms.msg.id = 0x1839F380; // module #1
    bms.msg.flags.extended = 1;

    getTemp(bms); //TBD
    bms.msg.buf[0] = 0;  // module #1
    bms.msg.buf[1] = bms.lowTemp; // Lowest thermistor value
    bms.msg.buf[2] = bms.highTemp; // Highest thermistor value
    bms.msg.buf[3] = bms.avgTemp; // Average thermistor value
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
