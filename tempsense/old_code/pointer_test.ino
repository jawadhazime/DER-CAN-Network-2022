char *myVar = "example.txt";
String myString = "example";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  myString = myString+0+".txt";
  //myVar[0] = 'a'; 
}

void loop() {


  //Serial.print(myString);
  for(int i = 0; i<10; i++){
    Serial.print(myVar[i]);
  }
  Serial.print("\n");
  
  // put your main code here, to run repeatedly:

}
