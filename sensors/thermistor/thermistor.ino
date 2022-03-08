// Testing thermistor values

int sensorPin = A0;
int sensorVal;
float tempVal;
float vOut;

void setup() {
	Serial.begin(115200); 
	delay(400);
}

void loop() {

	//reads, conversions
	sensorVal = analogRead(sensorPin);
	vOut = 3.3 * (sensorVal/255.); //analog to voltage value
	tempVal = (vOut - 0.5)/0.01; //transfer function solving for ambient temp

	//serial prints
	Serial.print(tempVal);
        Serial.print("°C ");
	Serial.print(vOut);
	Serial.print("V ");
	Serial.println(sensorVal);
 
	delay(500); // delays sensor message to make it more readable
}


