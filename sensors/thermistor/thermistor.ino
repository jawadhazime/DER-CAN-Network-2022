// Testing thermistor values
void setup() {
	Serial.begin(115200); 
	delay(400);
}

void loop() {
	int sensor = analogRead(A0);
	Serial.println(sensor);
	delay(500); // delays sensor message to make it more readable
}


