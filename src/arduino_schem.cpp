#include <Arduino.h>
#include <math.h>

void batteryV(int num);
void waterTemp(int num);
void myDisplay(int num);
void serialPrint(int num);

String myVersion = "0.01";
String authour = "J. Bracken";

const byte ebv1_pin = A8;
const byte wt1_pin = A9;

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

//attempting to use therminstor template ↓
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

float Temperature(int AnalogInputNumber,int OutputUnit,float B,float T0,float R0,float R_Balance)
{
  float R,T;

//  R=1024.0f*R_Balance/float(analogRead(AnalogInputNumber)))-R_Balance;
  R=R_Balance*(1024.0f/float(analogRead(AnalogInputNumber))-1);

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



/* ↓
tag:wt1 | description: Engine Water Temperature
functional description: Voltage divider to measure the resistance of the sensor
note: 314 ohms is maximum measured capability */
int wt1_raw = 0;
float wt1_Vout = 0;
float wt1_R1 = 3292;
float wt1_R2 = 0;
float wt1_buffer = 0;
//--NTC calibration
float wt1_R0 = 197.3;  // value of rct in T0 [ohm] (Choose midpoint on curve near operating range or Datasheet correction temperature)
float wt1_T0 = (273.15 + 50); // use T0 in Kelvin [K]
// use the datasheet to get this data.
float wt1_T1 = (273.15 + 40);    // [K] in datasheet 0º C (fill in RHS number with LRV degrees C)
float wt1_T2 = (273.15 + 120);    // [K] in datasheet 100° C (fill in RHS number with URV degrees C)
float wt1_RT1 = 291.5; // [ohms]  resistance for T1
float wt1_RT2 = 22.4;  // [ohms]   resistance for T2
float wt1_beta = 0.0;  // initial parameters [K]
float wt1_Rinf = 0.0;  // initial parameters [ohm]
float wt1_TempK = 0.0; // variable output
float wt1_TempC = 0.0; // variable output
int wt1_TAH =0;
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

void setup() {

  analogReference(INTERNAL1V1); // use the internal ~1.1volt reference  | change (INTERNAL) to (INTERNAL1V1) for a Mega
  Serial.begin(9600); // set serial monitor to this value

  //splash screen ↓===================================
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("The vi-Kerry-ous");
  lcd.setCursor(6, 1);
  lcd.print("bus!");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Version: ");
  lcd.setCursor(10, 0);
  lcd.print(myVersion);
  lcd.setCursor(0, 1);
  lcd.print("by ");
  lcd.setCursor(4, 1);
  lcd.print(authour);
  delay(5000);
  lcd.clear();
  // splash screen ↑===============================


  //==for ntc thermistor correction ↓
  wt1_beta = (log(wt1_RT1 / wt1_RT2)) / ((1 / wt1_T1) - (1 / wt1_T2));
  wt1_Rinf = wt1_R0 * exp(-wt1_beta / wt1_T0);
  //==for ntc thermistor correction ↑
}

void loop() {
  delay(10);
}

void batteryV() {
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

void waterTemp() {
  //tag: wt1
  wt1_raw = analogRead(wt1_pin);
  if (wt1_raw)
  {
    wt1_buffer = wt1_raw * Aref;
    wt1_Vout = (wt1_buffer) / 1024.0;
    wt1_buffer = (ebv1_volts / wt1_Vout);
    wt1_R2 = wt1_R1 / wt1_buffer;
  }

  //calculate and print water temperature↓
  wt1_TempK = (wt1_beta / log(wt1_R2 / wt1_Rinf)); // calc for temperature
  wt1_TempC = wt1_TempK - 273.15;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wtr T Old: ");
  lcd.setCursor(12, 0);
  lcd.print(wt1_TempC);
  lcd.print(Temperature(wt1_pin,T_CELSIUS,wt1_beta,wt1_T0,wt1_R0,wt1_R2));
  delay(1200);

if(wt1_TempC > 105){
  wt1_TAH = 1;
}
else{ wt1_TAH = 0;
}
}

void myDisplay() {
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
}

void serialPrint() {
  Serial.print("wt1_Vout: ");
  Serial.print(wt1_Vout);
  Serial.println("volts");
  //
  Serial.print("Measured Resistance: ");
  Serial.print(wt1_R2);
  Serial.println("ohms");//Water Temp Sensor wt1
  //
  Serial.println("");
  Serial.println("");
  Serial.println("");

  // Battery Voltage Alarm
 if(ebv1_voltsHH ==1){
    Serial.println("Voltage High Alarm");
 }
 else{
    Serial.print("Battery volts: ");
    Serial.print(ebv1_volts);
}
}
