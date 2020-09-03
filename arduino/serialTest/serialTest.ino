
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
 int counter=0;
void setup() {
  // initialize serial:
  Serial.begin(115200);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop() {
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.print("Get data ");
    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }else {
      Serial.print("No data # ");
      Serial.println(counter);
      counter++;
      delay(300);
    }
    
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
