#include <Arduino.h>
#include <math.h>

void batteryV(int num);
void temperature(int num);
void level(int num);
void pressure(int num);
void myDisplay(int num);
void serialPrint(int num);


String myVersion = "0.01";
String authour = "J. Bracken";

//analog input pin assignment
const byte ebv1_pin = A8; //Engine Battery Voltage
const byte wt1_pin = A9;  //Engine water temp
const byte ot1_pin = A14; //Engine oil temp
const byte fl1_pin = A12; //Fuel Tank 1 level
const byte fl2_pin = A11; //Fuel Tank 2 level
const byte bp1_pin = A10; //Brake Air Pressure Reservior
const byte op1_pin = A15; //Engine oil pressure

//analog output pin assignment
const byte wg1_pin = A7; //Water Gauge Dash
const byte fg1_pin = A8; //Fuel Gauge Dash

//high speed digital input pin assignment
const byte rs1_pin = 2; //Road speed
const byte es1_pin = 3; //Engine Speed

//digital input pin assignment
const byte opl1_pin = 24; //Oil P switch input
const byte bpl1_pin = 25; //Brake Pressure Low switch

//digital output pin assignment
const byte dl1_pin = 22; //Oil Light Dash
const byte dl2_pin = 23; //Other dash light


/* ↓
tag: ebv1 | description: Engine Battery Voltage
functional description: 0 - ~16volt voltmeter uses the stable internal 1.1volt reference. 6k8 resistor from A0 to ground, and 100k resistor from A0 to +batt 100n capacitor from A0 to ground for stable readings
*/

float Aref = 1.073;       // ***calibrate battery voltage here*** | change this to the actual Aref voltage of Arduino
unsigned int ebv1_total;       // can hold max 64 readings
float ebv1_R1 = 99080.0;  //value of large volt divider resistor for engine battery voltage
float ebv1_R2 = 6760.0;   //value of Small volt divider resistor for engine battery voltage
float ebv1_resRatio = ((ebv1_R1 + ebv1_R2) / (ebv1_R2)); //ratio of voltage divider
float ebv1_volts;         // converted to volt
int ebv1_voltsHH = 0;     //0 = healthy, 1 = alarm
// ↑

//thermistor template ↓
enum {
  T_KELVIN=0,
  T_CELSIUS,
};

/* Temperature function inputs
1.AnalogInputNumber - analog input to read from
2.OuputUnit - output in celsius, kelvin or fahrenheit
3.Thermistor B parameter - found in datasheet
4.Manufacturer T0 parameter - found in datasheet (kelvin)
5. Manufacturer R0 parameter - found in datasheet (ohms)
6. Your balance resistor resistance in ohms
↑*/

float Thermistor(int AnalogInputNumber,int OutputUnit,float B,float T0,float R0,float R_Balance){
  float R,T;

  R=(R_Balance*((float(analogRead(AnalogInputNumber)))/(1024.0f-1.0f)))*(Aref/ebv1_volts); //required correction for use of different voltage references

  T=1.0f/(1.0f/T0+(1.0f/B)*log(R/R0));

  switch(OutputUnit) {
    case T_CELSIUS :
      T-=273.15f;
    break;
    default:
    break;
  };

  return T;
}
//thermistor template ↑



/* ↓ tag:wt1 | description: Engine Water Temperature
functional description: Voltage divider to measure the resistance of the sensor
note: 314 ohms is maximum measured capability */
float wt1_R1 = 3292; //actual resistance of resistor between 12v line and the sensor
//--NTC calibration
float wt1_R0 = 197.3;  // value of rct in T0 [ohm] (Choose midpoint on curve near operating range or Datasheet correction temperature)
float wt1_T0 = (273.15 + 50); // datasheet temperature at R0 (above)
// use the datasheet to get this data.
float wt1_T1 = (273.15 + 40);    // [K] in datasheet 0º C (fill in RHS number with LRV degrees C)
float wt1_T2 = (273.15 + 120);    // [K] in datasheet 100° C (fill in RHS number with URV degrees C)
float wt1_RT1 = 291.5; // [ohms]  resistance for T1
float wt1_RT2 = 22.4;  // [ohms]   resistance for T2
float wt1_beta = 0.0;  // initial parameters [K]
float wt1_TempC = 0.0; // variable output
int wt1_TAH =0;
// ↑

/* ↓
tag:ot1 | description: Engine Oil Temperature
functional description: Voltage divider to measure the resistance of the sensor
note: 314 ohms is maximum measured capability */
float ot1_R1 = 3292; //actual resistance of resistor between 12v line and the sensor
//--NTC calibration
float ot1_R0 = 197.3;  // value of rct in T0 [ohm] (Choose midpoint on curve near operating range or Datasheet correction temperature)
float ot1_T0 = (273.15 + 50); // datasheet temperature at R0 (above)
// use the datasheet to get this data.
float ot1_T1 = (273.15 + 40);    // [K] in datasheet 0º C (fill in RHS number with LRV degrees C)
float ot1_T2 = (273.15 + 120);    // [K] in datasheet 100° C (fill in RHS number with URV degrees C)
float ot1_RT1 = 291.5; // [ohms]  resistance for T1
float ot1_RT2 = 22.4;  // [ohms]   resistance for T2
float ot1_beta = 0.0;  // initial parameters [K]
float ot1_TempC = 0.0; // variable output
int ot1_TAH =0;
// ↑

// for display ↓===========================
#include <LiquidCrystal.h>

// select the pins used on the LCD panel (DF0009)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
/* buttons require 5v refernce with this shield.
  // define some values used by the panel and buttons
  // buttons don't work because different voltage reference is used!!
  int lcd_key     = 0;
  int adc_key_in  = 0;
  #define btnRIGHT  0
  #define btnUP     1
  #define btnDOWN   2
  #define btnLEFT   3
  #define btnSELECT 4
  #define btnNONE   5

  // read the buttons
  int read_LCD_buttons()
  {
  adc_key_in = analogRead(0);      // read the value from the sensor on analog pin 0
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 195)  return btnUP;
  if (adc_key_in < 380)  return btnDOWN;
  if (adc_key_in < 555)  return btnLEFT;
  if (adc_key_in < 790)  return btnSELECT;

  return btnNONE;  // when all others fail, return this...
  }*/
// for display ↑==============================

//Fuel level
int fl1;
int fl2;
//Fuel calibration
int fl1_LRV = 200; //counts on input at 0% level
int fl1_URV = 950; //counts on input at 100% level
int fl2_LRV = 200; //counts on input at 0% level
int fl2_URV = 950; //counts on input at 100% level

//Oil and Brake pressure
int op1;
int bp1;

//Pressure calibration
int op1_LRV = 200; //counts on input at 0% level
int op1_URV = 950; //counts on input at 100% level
int bp1_LRV = 200; //counts on input at 0% level
int bp1_URV = 950; //counts on input at 100% level

void setup() {

  analogReference(INTERNAL1V1); // use the internal ~1.1volt reference  | change (INTERNAL) to (INTERNAL1V1) for a Mega
  Serial.begin(9600); // set serial monitor to this value
  Serial.println("Booting.");
  Serial.println("Booting..");
  Serial.println("Booting...");
  Serial.print("Version: ");
  Serial.println(myVersion);
  Serial.print("By: ");
  Serial.println(authour);
  Serial.println("");
  //splash screen ↓===================================
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("The vi-Kerry-ous");
  lcd.setCursor(6, 1);
  lcd.print("bus!");
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Version: ");
  lcd.setCursor(10, 0);
  lcd.print(myVersion);
  lcd.setCursor(0, 1);
  lcd.print("by ");
  lcd.setCursor(4, 1);
  lcd.print(authour);
  delay(500);
  lcd.clear();
  // splash screen ↑===============================


  //==for ntc thermistor correction ↓
  // if beta is known, comment out calculation line and uncomment from datasheet line
  wt1_beta = (log(wt1_RT1 / wt1_RT2)) / ((1 / wt1_T1) - (1 / wt1_T2)); //by calculation
  //wt1_beta = 0; //from datasheet

}

void loop() {
  batteryV(1);
  temperature(2);
  level(3);
  pressure(4);
  myDisplay(5);
  serialPrint(6);
  delay(2000);
}

void batteryV(int num) {
  for (int x = 0; x < 64; x++) { // multiple analogue readings for averaging
    ebv1_total = ebv1_total + analogRead(ebv1_pin); // add each value to a total
  }
  ebv1_volts = (ebv1_total / 64) * ebv1_resRatio * Aref / 1024 ; // convert readings to volt


  if (ebv1_total == (1023 * 64)) {
    ebv1_voltsHH = 1;
  }
  else {
    ebv1_voltsHH = 0;
  }
  ebv1_total = 0; // reset value
  delay(1000); // one second between measurements
}
void temperature(int num) {
  //tag: wt1
  wt1_TempC = Thermistor(wt1_pin,T_CELSIUS,wt1_beta,wt1_T0,wt1_R0,wt1_R1);

  if(wt1_TempC > 105){
    wt1_TAH = 1;
  }
  else{
    wt1_TAH = 0;
  }

  //tag: ot1
  ot1_TempC = Thermistor(ot1_pin,T_CELSIUS,ot1_beta,ot1_T0,ot1_R0,ot1_R1);

  if(ot1_TempC > 105){
    ot1_TAH = 1;
  }
  else{
    ot1_TAH = 0;
  }
}
void level(int num) {
  int sensorValue1 = analogRead(fl1_pin);
  int sensorValue2 = analogRead(fl2_pin);
  fl1 = map(sensorValue1, fl1_LRV, fl1_URV, 0, 100); //assumes linear sensor
  fl2 = map(sensorValue2, fl2_LRV, fl2_URV, 0, 100); //assumes linear sensor
  if (fl1 < 0) {
    fl1 = 0;
  }
  if (fl2 < 0) {
    fl2 = 0;
  }
  if (fl1 > 100) {
    fl1 = 100;
  }
  if (fl2 > 100) {
    fl2 = 100;
  }
}
void pressure(int num) {
  int sensorValue3 = analogRead(op1_pin);
  int sensorValue4 = analogRead(bp1_pin);
  op1 = map(sensorValue3, op1_LRV, op1_URV, 0, 100); //assumes linear sensor
  bp1 = map(sensorValue4, bp1_LRV, bp1_URV, 0, 100); //assumes linear sensor
  if (op1 < 0) {
    op1 = 0;
  }
  if (bp1 < 0) {
    bp1 = 0;
  }
  if (op1 > 100) {
    op1 = 100;
  }
  if (bp1 > 100) {
    bp1 = 100;
  }
}
void myDisplay(int num) {
//battery voltage alarm
 if(ebv1_voltsHH ==1){
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Voltage too");
    lcd.setCursor(5, 1);
    lcd.print("HIGH!!");

//water temp alarm

 }

  //for display ↓

  // lcd.setCursor(0,1);            // move to the begining of the second line
  // lcd_key = read_LCD_buttons();  // read the buttons

  /* switch (lcd_key)
     need to fix the input for hte buttons before implementing some code here to display different screens
    // depending on which button was pushed, we perform an action
    {
    case btnRIGHT:
      {
      lcd.print("RIGHT ");
      break;
      }
    case btnLEFT:
      {
      lcd.print("LEFT   ");
      break;
      }
    case btnUP:
      {
      lcd.print("UP    ");
      break;
      }
    case btnDOWN:
      {
      lcd.print("DOWN  ");
      break;
      }
    case btnSELECT:
      {
      lcd.print("SELECT");
      break;
      }
      case btnNONE:
      {
      lcd.print("NONE  ");
      break;
      }
    }*/
  // for display ↑
//}
}
void serialPrint(int num) {
  //
  //Water Temp Sensor wt1
  //
  Serial.print("Engine Water Temp: ");
  Serial.print(wt1_TempC);
  Serial.println("C");
  //
  //Oil Temp Sensor ot1
  //
  Serial.print("Engine Oil Temp: ");
  Serial.print(ot1_TempC);
  Serial.println("C");

  //Fuel Level Tank 1 fl1
  //
  Serial.print("Diesel Tank 1: ");
  Serial.print(fl1);
  Serial.println("%");
  //Fuel Level Tank 2 fl2
  //
  Serial.print("Diesel Tank 2: ");
  Serial.print(fl1);
  Serial.println("%");

  // Battery Voltage Alarm
 if(ebv1_voltsHH ==1){
    Serial.println("Voltage High Alarm");
 }
 else{
     Serial.print("Battery volts: ");
     Serial.println(ebv1_volts);
 }
 Serial.println("");
 delay(800);
}
