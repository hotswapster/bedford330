

//////////////////////////////////////////////////////
/*
  0 - ~16volt voltmeter for 3.3volt and 5volt Arduinos
  uses the stable internal 1.1volt reference
  6k8 resistor from A0 to ground, and 100k resistor from A0 to +batt
  100n capacitor from A0 to ground for stable readings
  (100k + 6k8) / 6k8 = 15.70588 | used in formula
*/
float Aref = 1.075; // ***calibrate here*** | change this to the actual Aref voltage of ---YOUR--- Arduino
unsigned int total; // can hold max 64 readings
float voltage; // converted to volt

void setup() {
  analogReference(INTERNAL); // use the internal ~1.1volt reference  | change (INTERNAL) to (INTERNAL1V1) for a Mega
  Serial.begin(9600); // set serial monitor to this value
}

void loop() {
  for (int x = 0; x < 64; x++) { // multiple analogue readings for averaging
    total = total + analogRead(A0); // add each value to a total
  }
  voltage = (total / 64) * 15.70588 * Aref / 1024 ; // convert readings to volt
  // print to serial monitor
  if (total == (1023 * 64)) { // if overflow
    Serial.print("voltage too high");
  }
  else {
    Serial.print("The battery is ");
    Serial.print(voltage);
    Serial.println(" volt");
  }
  total = 0; // reset value
  delay(1000); // one second between measurements
}

//////////////////////////////////////////////////////////////