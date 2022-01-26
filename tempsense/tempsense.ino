#include "FlexCAN_T4.h"
FlexCAN_T4 <CAN2, RX_SIZE_256, TX_SIZE_16> Can0;

struct message {
  CAN_message_t msg;
  uint8_t checksum;
  uint8_t lowTemp, highTemp, avgTemp=0;
} bms;

CAN_message_t therm;

//array of input pins connected to the multiplexers
int muxInputPin[9] = {A1, A2, A3, A4, A5, A6, A7, A8, A9};

//array of values and pin numbers for the mux control signals
int control[3] = {0,0,0}; 
int controlPin[3] = {5, 4, 3};
                  //{A, B, C} Most significant bit is C

//int fault = 0;
//int faultPin = 14;

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

double adcTemperature[72];
double analogTemperature[72];
double temperature[72];

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
  int data_samples = 100;
  double val = 0.0000;
  
  //read thermistor values and add them to adcTemperature[]
  for(int l = 0; l<data_samples; l++){
    
    for(int j = 0; j < 8; j++)
    {
      //set active mux output
      setMuxControl(j);
      delay(1);
      
      for(int i = 0; i < 9; i++)
      {     
        //read j'th thermistor from i'th mux
        val = analogRead(muxInputPin[i]);
        adcTemperature[therm_index[i][j] - 1] +=  val;
      }
    }
  }
  
  for(int i = 0; i<72; i++){
    adcTemperature[i] = adcTemperature[i]/data_samples;
  }
}

//-------------------------------------------------------------------------------------------------------------------
void calculateTemperature()
{
  for(int i = 0; i<72;i++){

    analogTemperature[i] = (adcTemperature[i]/1023.0) * 3.300; 
    temperature[i] = ((537.11*pow(analogTemperature[i],6)) - (6749*pow(analogTemperature[i],5)) 
                    + (34799*pow(analogTemperature[i],4)) - (94465*pow(analogTemperature[i],3)) 
                    + (142663*pow(analogTemperature[i],2)) - (113923*analogTemperature[i]) + 37735);
                    //Transfer function based on thermistor data

  }
}

//-------------------------------------------------------------------------------------------------------------------
void printTemp()
{
  for(int i = 0; i<72;i++){

    Serial.print("Stack ");
    Serial.print((i/8)+1);
    Serial.print(" ");

    Serial.print("Slot ");
    Serial.print(i + 1);
    Serial.print(": ");
    
    Serial.print("ADC Temperature ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(adcTemperature[i]);
    Serial.print(" ");
    
    Serial.print("Analog Temperature ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(analogTemperature[i],4);
    Serial.print(" ");
    
    Serial.print(" Temperature ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(temperature[i],4);
    Serial.print("\n");
  }
  //if(fault == 1){
  //    Serial.println("VOLTAGE FAULT");
  //    }
  //else {
  //  Serial.println("VOLTAGE OK");
  //}TBD
  Serial.print("\n");
  Serial.print("\n");
  Serial.print("\n");
}

//----------------------------------------------------------------------------------------------------------------------------
void getTemp(message &msg){
  
  double val = 0;

  getMuxValues(); //Obtain ADC values
  calculateTemperature(); //Convert ADC values into temperature readings in temperature[]
  printTemp();

  msg.lowTemp = temperature[0];
  msg.highTemp = temperature[0];
  val = temperature[0];

  for (int i = 1; i < 72; i++){
    if (msg.lowTemp > temperature[i])
      msg.lowTemp = temperature[i];
    
    if (msg.highTemp < temperature[i])
      msg.highTemp = temperature[i];

    val += temperature[i];
  }
  msg.avgTemp = val/72;
}

//-------------------------------------------------------------------------------------------------------------------
/*void checkFault()
{
  int high_cell_voltage; //TBD: Update based on BMS OP on CAN
  int low_cell_voltage; //TBD: Update based on BMS OP on CAN

  if(high_cell_voltage[i] > 4.0 || low_cell_voltage[i] < 3.0){
    fault = 1;
    digitalWrite(faultPin, fault);
    }
  else
    fault = 0;
}*/

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

//-------------------------------------------------------------------------------------------------------------------
void clearValues()
{
  for(int i = 0; i < 72; i++){
    adcTemperature[i] = 0;
    analogTemperature[i] = 0;
    temperature[i] = 0;
  }
}

//----------------------------------------------------------------------------------------------------------------------------
void setup(void) {
  Serial.begin(115200); 
  delay(400);
  
  //set mux output pins as inputs
  for(int i = 0; i < 9; i++)
  {
    pinMode(muxInputPin[i], INPUT);
  }

  //set mux controls as output
  for(int i = 0; i < 3; i++)
  {
    pinMode(controlPin[i], OUTPUT);
  }

  //setup fault LED
  //pinMode(faultPin,OUTPUT);TBD

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

    getTemp(bms);
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
    therm.buf[2] = temperature[thermistor_read]; // Thermistor value
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

    //checkFault(); //based on low and high cell voltage from BMS

    timeout = millis();
    clearValues();
  }
}
