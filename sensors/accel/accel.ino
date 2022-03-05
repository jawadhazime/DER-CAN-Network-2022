// reading 3-axis accelerometer inputs
void setup() {
	Serial.begin(115200);
	delay(400); // delay for some misc reason
}

void loop() {
	// sample values
	int accelX = analogRead(A0);
	int accelY = analogRead(A1);
	int accelZ = analogRead(A2);
	// print out every ~ 20 ms
	Serial.print(accelX);
	Serial.print("\t");
	Serial.print(accelY);
	Serial.print("\t");
	Serial.print(accelZ);
	Serial.println();
	delay(20);
}
