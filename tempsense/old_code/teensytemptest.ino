
void setup() {
  Serial.begin(9600);
  pinMode (A16, INPUT); // function with two arguments
  pinMode (A17, INPUT);
  pinMode (A18, INPUT);
  pinMode (A19, INPUT);
}

void loop() {
  Serial.print("Pin A16: ");
  Serial.print(analogRead(A16));
  Serial.print("\n");
  Serial.print("Pin A17: ");
  Serial.print(analogRead(A17));
  Serial.print("\n");
  Serial.print("Pin A18: ");
  Serial.print(analogRead(A18));
  Serial.print("\n");
  Serial.print("Pin A19: ");
  Serial.print(analogRead(A19));
  Serial.print("\n");
  delay(500);
}
