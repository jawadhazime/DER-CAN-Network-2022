#include <ADC.h>

#define VREF 3.300
#define resolution (VREF/((pow(2,16))-1));

ADC *adc = new ADC(); // adc object

//array of input pins from each mux to the teensy
int muxInputPin[9] = {A0, A1, A2, A3, A6, A7, A8, A9, A21}; //location of mux input pins 
                   //{Vm1, Vm2, Vm3, Vm4, Vm5, VM6, Vm7, Vm8, Vm9} NOTE A10 Must be used instead of A20 B/C A20
                   //Does not have 16 bit inputs

int tempMuxInputPin[6] = {A18, A12, A13, A16, A17, A19};
                         //Ts6, Ts5, Ts4, Ts1, Ts2, T3


//array of sorted cell voltages
double Sumvoltage[72];
double voltage[72];
double temp1[72];
double temp2[72];
double temp3[72];
double val = 0.0000;
double avgADC[72];

double adcTemperature[48];
double analogTemperature[48];
double temperature[48];

int fault = 0;
int faultPin = 2;

//array of values and pin numbers for the mux control signals
int control[3] = {0,0,0}; // Most significant bit is C, thus it goes CBA
int controlPin[3] = {30, 29, 28};
                   //{C, B , A }

//matrix of indeces to sort elements
int indexNum[9][8] = 
{
  {19, 18, 17, 20, 53, 56, 54, 55},// Mux 1
  {23, 22, 21, 24, 57, 60, 58, 59},// Mux 2
  {38, 39, 40, 37, 44, 41, 43, 42},// Mux 3
  {35, 34, 33, 36, 45, 48, 46, 47},// Mux 4
  {27, 26, 25, 28, 32, 29, 31, 30},// Mux 5
  {1, 2, 3, 4, 5, 8, 6, 7},// Mux 6
  {10, 11, 12, 9, 72, 69, 71, 70},// Mux 7
  {67, 66, 65, 68, 62, 64, 61, 63},// Mux 8
  {15, 14, 13, 16, 49, 52, 50, 51} // Mux 9
};

double ratio[72] =
{ 1.279166387, 2.577869267, 3.911718537, 5.13434089, 6.479674521, 7.676629071, 9.049502942, 10.3153138,
  11.53157983, 12.85992872, 14.06377432, 15.59734964, 16.87788488, 18.09596216, 19.44077275, 20.78250824,
  22.03112286, 23.08360733, 24.29011107, 25.89912206, 27.11306405, 28.34902318, 29.66027286, 31.05925022,
  32.02502768, 33.4285137, 34.91044523, 35.9014302, 37.11691184, 38.44571927, 40.10124268, 41.10169776,
  42.3472712, 44.06973983, 45.09592367, 46.41209014, 47.54977132, 48.79041473, 49.83902814, 51.8831963,
  52.97795985, 54.438103, 55.7468062, 57.061323, 58.21945678, 58.85856079, 60.18018925, 61.58928151,
  63.42806756, 64.25791246, 66.47874338, 68.02425987, 67.81274086, 69.2457519, 70.83642496, 72.40258641,
  73.910239, 74.41391818, 75.79527877, 77.65845726, 78.01841004, 79.7307757, 81.35707769, 81.75194679,
  83.57947855, 86.4263935, 87.00866316, 87.55094479, 89.39804772, 89.89216971, 91.6191312, 91.92266595};

int indexTempNum[6][8] = 
{
  {1, 2, 3, 4, 5, 6, 7, 8},// Mux 1
  {9, 10, 11, 12, 13, 14, 15, 16}, //Temp Mux 2
  {17, 18, 19, 20, 21, 22, 23, 24},//Temp Mux 3
  {25, 26, 27, 28, 29, 30, 31, 32},//Temp Mux 4
  {33, 34, 35, 36, 37, 38, 39, 40},//Temp Mux 5
  {41, 42, 43, 44, 45, 46, 47, 48},//Temp Mux 6
};

//-------------------------------------------------------------------------------------------------------------------
void setMuxControl(int i)
{
  //set ABC mux control inputs
  control[0] = i % 2; //LSB --- C
  control[1] = (i / 2) % 2; // --- B
  control[2] = (i / 4) % 2; //MSB --- A

  //digital write mux control inputs
  digitalWrite(controlPin[0], control[2]);
  // C = control[0] MOST SIGNIFICANT
  digitalWrite(controlPin[1], control[1]);
  // B = control[1]
  digitalWrite(controlPin[2], control[0]);
  // A = control[2] LEAST SIGNIFICANT
}
//-------------------------------------------------------------------------------------------------------------------
void getMuxValues()
{
  //read in cell voltages and add them to the avgADC
  for(int l = 0; l<500; l++){
    
    for(int j = 0; j < 8; j++)
    {
      //set active mux output
      setMuxControl(j);
      delay(1);
      
      for(int i = 0; i < 9; i++)
      {     
        //read in j'th voltage from i'th mux and convert to voltage
        val = adc->analogRead(muxInputPin[i]);
  
        avgADC[indexNum[i][j] - 1] +=  val;
      }
    }
  }
  
  for(int i = 0; i<72; i++){
    avgADC[i] = avgADC[i]/500.0;
  }
}
//-------------------------------------------------------------------------------------------------------------------
void calculateVoltages()
{
  for(int i = 0; i<72;i++){


    temp2[i] = (avgADC[i]/65535.0) * 3.300; 

    Sumvoltage[i] = temp2[i] * ratio[i];

  }
  //do subtraction to get individual cell voltage
  for(int i = 71; i > 0; i--){
    voltage[i] = Sumvoltage[i] - Sumvoltage[i - 1];
    if(i == 0){
      voltage[i] = Sumvoltage[i];
    }
  }
}
//-------------------------------------------------------------------------------------------------------------------
void clearValues()
{
  for(int i = 0; i<72;i++){
    avgADC[i] = 0;
  }
  for(int i = 0; i<48;i++){
    adcTemperature[i] = 0;
  }
}
//-------------------------------------------------------------------------------------------------------------------
void checkFault()
{
  for(int i = 0; i<72;i++){
    if(voltage[i] > 4.0 || voltage[i] < 3.0){
      fault = 1;
      digitalWrite(faultPin, fault);
      }
  }
}
//-------------------------------------------------------------------------------------------------------------------
void printVoltageOutput()
{
  for(int i = 0; i<72;i++){
    Serial.print("Cell ");
    Serial.print(i + 1);
    Serial.print(": ");
    
    /*Serial.print("ADC ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(avgADC[i]);
    Serial.print(" ");
    
    Serial.print("Analog ADC ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(" ");
    Serial.print(temp2[i],4);
    Serial.print(" ");

    Serial.print("Sum Voltage ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(" ");
    Serial.print(Sumvoltage[i],4);*/
    
    Serial.print(" Voltage ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(" ");
    Serial.println(voltage[i],4);
  }
  if(fault == 1){
      Serial.println("VOLTAGE FAULT");
      }
  else {
    Serial.println("VOLTAGE OK");
  }
  Serial.print("\n");
  Serial.print("\n");
  Serial.print("\n");
}
//-------------------------------------------------------------------------------------------------------------------
void getMuxTempValues()
{
  //read in cell voltages and add them to the avgADC
  for(int l = 0; l<100; l++){
    
    for(int j = 0; j < 8; j++)
    {
      //set active mux output
      setMuxControl(j);
      delay(1);
      
      for(int i = 0; i < 6; i++)
      {     
        //read in j'th voltage from i'th mux and convert to voltage
        val = analogRead(tempMuxInputPin[i]);
  
        adcTemperature[indexTempNum[i][j] - 1] +=  val;
      }
    }
  }
  
  for(int i = 0; i<72; i++){
    adcTemperature[i] = adcTemperature[i]/100.0;
  }
}
//-------------------------------------------------------------------------------------------------------------------
void calculateTemperature()
{
  for(int i = 0; i<48;i++){

    analogTemperature[i] = (adcTemperature[i]/1024.0) * 3.300; 

    temperature[i] = (analogTemperature[i]-0.5)/0.01;

  }
}
//-------------------------------------------------------------------------------------------------------------------
void printTemperatureOutput()
{
  for(int i = 0; i<48;i++){

    Serial.print("Stack ");
    Serial.print((i/8)+1);
  
    Serial.print(" Slot ");
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
    Serial.print(" ");
    Serial.print(analogTemperature[i],4);
    Serial.print(" ");
    
    Serial.print(" Temperature ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(" ");
    Serial.print(temperature[i],4);
    Serial.print("\n");
    
  }
  if(fault == 1){
      Serial.println("VOLTAGE FAULT");
      }
  else {
    Serial.println("VOLTAGE OK");
  }
  Serial.print("\n");
  Serial.print("\n");
  Serial.print("\n");
}

//*******************************************************************************************************************
void setup() 
{
  //begin serial comms
  Serial.begin(9600);
  
  //set voltage mux pins as inputs
  for(int i = 0; i < 9; i++)
  {
    pinMode(muxInputPin[i], INPUT);
  }

  //set temperature mux pins as inputs
    for(int i = 0; i < 6; i++)
  {
    pinMode(tempMuxInputPin[6], INPUT);
  }
  
  //set mux controls as output
  for(int i = 0; i < 3; i++)
  {
    pinMode(controlPin[i], OUTPUT);
  }

  //setup fault LED
  pinMode(faultPin,OUTPUT);

  adc->setResolution(16); // set bits of resolution
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED);
}
//*******************************************************************************************************************
void loop() 
{
  
  //getMuxValues();
  //calculateVoltages();
  //checkFault();
  //printVoltageOutput();
  getMuxTempValues();
  calculateTemperature();
  printTemperatureOutput();
  clearValues();

  
  delay(500);
  
}
//*******************************************************************************************************************
