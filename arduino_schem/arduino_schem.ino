
  
// for battery voltage ↓
/*
  0 - ~16volt voltmeter uses the stable internal 1.1volt reference.
  6k8 resistor from A0 to ground, and 100k resistor from A0 to +batt
  100n capacitor from A0 to ground for stable readings
  */
float Aref = 1.073; // ***calibrate battery voltage here*** | change this to the actual Aref voltage of Arduino
unsigned int total; // can hold max 64 readings
float rBig = 99080.0; //value of large volt divider resistor for battery voltage
float rSmall = 6760.0; //value of Small volt divider resistor for battery voltage
float resRatio = ((rBig + rSmall)/(rSmall));
float voltage; // converted to volt
// for battery voltage ↑


// for resistance measurement ↓
// 314 ohms is maximum measured capability
int analogPin= A9;
int raw= 0;
float Vout= 0;
float R1= 3292;
float R2= 0;
float buffer= 0;
//--NTC calibration
float R0=197.3;    // value of rct in T0 [ohm] (Choose midpoint on curve near operating range or Datasheet correction temperature)
float T0=(273.15 + 50);   // use T0 in Kelvin [K]
// use the datasheet to get this data.
float T1=(273.15 + 40);      // [K] in datasheet 0º C (fill in RHS number with LRV degrees C)
float T2=(273.15 + 120);      // [K] in datasheet 100° C (fill in RHS number with URV degrees C)
float RT1=291.5;   // [ohms]  resistance for T1
float RT2=22.4;    // [ohms]   resistance for T2
float beta=0.0;    // initial parameters [K]
float Rinf=0.0;    // initial parameters [ohm]   
float TempK=0.0;   // variable output
float TempC=0.0;   // variable output
// for resistance measurement ↑

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
  
  //display ↓===================================
 lcd.begin(16, 2);              // start the library
 lcd.setCursor(0,0);
 lcd.print("The vi-Kerry-ous");
  lcd.setCursor(6,1);
 lcd.print("bus!");
delay(2000);
lcd.clear();
 // print a simple message
  // for display ↑===============================

  //==============for ntc log correction ↓
    //parâmetros
  beta=(log(RT1/RT2))/((1/T1)-(1/T2));
  Rinf=R0*exp(-beta/T0);
  //====================for ntc log correction↑
}

void loop() {
  for (int x = 0; x < 64; x++) { // multiple analogue readings for averaging
    total = total + analogRead(A8); // add each value to a total
  }
  voltage = (total / 64) * resRatio * Aref / 1024 ; // convert readings to volt
    
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


// for resistance ↓

raw= analogRead(analogPin);
if(raw) 
{
buffer= raw * Aref;
Vout= (buffer)/1024.0;
buffer= (voltage/Vout);
R2= R1 / buffer;
Serial.print("Vout: ");
Serial.println(Vout);
Serial.print("R2: ");
Serial.print(R2);
Serial.println("ohms");
Serial.print("Raw In: ");
Serial.println(raw);
Serial.println("");
Serial.println("");
Serial.println("");
// lcd.setCursor(0,1);
// lcd.print(R2);
//delay(1000);
}

// for resistance ↑
//for display ↓
//  lcd.setCursor(9,1);            // move cursor to second line "1" and 9 spaces over
 //lcd.print(millis()/1000);      // display seconds elapsed since power-up


// lcd.setCursor(0,1);            // move to the begining of the second line
// lcd_key = read_LCD_buttons();  // read the buttons

 /* switch (lcd_key)               // depending on which button was pushed, we perform an action
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
//for log correction↓
  TempK=(beta/log(R2/Rinf)); // calc for temperature
  TempC=TempK-273.15;

    lcd.setCursor(0,0);
    lcd.print("Water Temp: ");
    lcd.setCursor(0,1);
    lcd.print(TempC);
    lcd.write(0xdf); // to display °
    lcd.print("C  ");
    delay(1200);
    // for log correction↑
}