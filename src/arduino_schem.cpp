#include <Arduino.h>
#include <math.h>

void batteryV(int num);
void temperature(int num);
void level(int num);
void pressure(int num);
void myDisplay(int num);
void serialPrint(int num);
void alarms(int num);

String myVersion = "0.01";
String authour = "J. Bracken";

//analog input pin assignment
  const byte ebv1_pin = A9; //Engine Battery Voltage
  const byte wt1_pin = A13;  //Engine water temp
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






/* ↓ tag: ebv1 | description: Engine Battery Voltage
  functional description: 0 - ~16volt voltmeter uses the stable internal 1.1volt reference. 6k8 resistor from A0 to ground, and 100k resistor from A0 to +batt 100n capacitor from A0 to ground for stable readings
  */

  float Aref = 1.073;         // ***calibrate battery voltage here*** | change this to the actual Aref voltage of Arduino
  unsigned int ebv1_total;    // can hold max 64 readings
  float ebv1_R1 = 99080.0;    //value of large volt divider resistor for engine battery voltage
  float ebv1_R2 = 6760.0;     //value of Small volt divider resistor for engine battery voltage
  float ebv1_resRatio = ((ebv1_R1 + ebv1_R2) / (ebv1_R2)); //ratio of voltage divider
  float ebv1_volts;           // converted to volt
  int ebv1_VAH = 0;           //0 = healthy, 1 = alarm
  float ebv1_VAH_SP = 14.7;   //voltage alarm high setpoint
  int ebv1_VAL = 0;           //0 = healthy, 1 = alarm
  float ebv1_VAL_SP = 10.7;   //voltage alarm low setpoint
  String ebv1_desc = "Engine Battery Volts"; //description of tag
  String ebv1_displayUnits = "volts"; //engineering units to display in


//thermistor template ↓
  enum {
    KELVIN=0,
    CELSIUS,
    FAHRENHEIT,
    };

  /* Temperature function inputs
  1.AnalogInputNumber - analog input to read from
  2.OuputUnit - output in celsius, kelvin or fahrenheit
  3.Thermistor B parameter - found in datasheet
  4.Manufacturer T0 parameter - found in datasheet (kelvin)
  5. Manufacturer R0 parameter - found in datasheet (ohms)
  6. Your balance resistor resistance in ohms
  ↑*/

  float Thermistor(int AnalogInputNumber,int displayUnit,float B,float T0,float R0,float R_Balance){
  float R,T;

  R=(R_Balance*((float(analogRead(AnalogInputNumber)))/(1024.0f-1.0f)))*(Aref/ebv1_volts); //required correction for use of different voltage references

  T=1.0f/(1.0f/T0+(1.0f/B)*log(R/R0));

  switch(displayUnit) {
    case CELSIUS :
      T-=273.15f;
    break;
    case FAHRENHEIT :
    T=9.0f*(T-273.15f)/5.0f+32.0f;
    break;
    default:
    break;
  };

  return T;
}
  //thermistor template ↑

//high alarm template
  float highAlarm(int input, float alarmSP){
  float A;
  if (input >= alarmSP) {
    A = 1;
  }
  else {
    A = 0;
  }
  return A;
}
  //high alarm template ↑

//low alarm template
  float lowAlarm(int input, float alarmSP){
  float A;
  if (input <= alarmSP) {
    A = 1;
  }
  else {
    A = 0;
  }
  return A;
}
  //low alarm template ↑

//serial print template
/* Serial functino Inputs
1. description
2. value
3. unit of measure (display units)
4. alarm bit */

  float serialPrinter(String DESC, float VAL, String UOM, int ALARM){
    Serial.print(DESC);
    Serial.print(" ");
    Serial.print(VAL);
    Serial.print(" ");
    Serial.println(UOM);
    if (ALARM == 1) {
      Serial.print("*** ");
      Serial.print(DESC);
      Serial.println("ALARM ***");
    } else {
      Serial.println(' ');
    }
  return 1.0;
  }
    //lserial print template ↑

/* Level template - linearLevel
Inputs:
  1. pin assignment
  3. input count for LRV calibrataion
  4. input count for URV calibrataion
  5. instrument calibrated span in engineering units
  6. instrument calibrated engineering units
  */
  float linearLevel(byte pin, int LRV, int URV, int SPAN, String CALUNITS, String DISPLAYUNITS) {
  float L = map(analogRead(pin), LRV, URV, 0, 100);

  //make value within limits
  if (L < 0) {
     L = 0;
  };
  if (L > SPAN) {
    L = SPAN;
  };

  //create different UOM
  float MM = 0;
  float IN = 0;
  float PERCENT = 0;

  //calculate pressure units
  if (CALUNITS == "mm") {
    MM = L;
    IN = L / 25.4;
    PERCENT = L / SPAN;
  };
  if (CALUNITS == "inches") {
    MM = L * 25.4;
    IN = L;
    PERCENT = L / SPAN;
  };
  if (CALUNITS == "percent") {
    PERCENT = L;
  };

  //return unit selected
  if (DISPLAYUNITS == "mm") {
    L = MM;
  };
  if (DISPLAYUNITS == "inches") {
    L = IN;
  }
  if (DISPLAYUNITS == "percent") {
    L = L;
  }

//add volume later ;)

    return L;
}

/* Pressure template - linearPressure
Inputs:
  1. pin assignment
  2. input count for LRV calibration
  3. input count for URV calibration
  4. instrument calibrated span in engineering units (e.g. 150)
  5. engineering units of calibrated span (e.g. psi)
  6. desired display/program engineering units (if 5. is in psi, 6. can be kPa)
  */
  float linearPressure(byte pin, int LRV, int URV, int SPAN, String CALUNITS, String DISPLAYUNITS) {
  float P = map(analogRead(pin), LRV, URV, 0, SPAN);

  //make value within limits
  if (P < 0) {
     P = 0;
  };
  if (P > SPAN) {
    P = SPAN;
  };

  //create different UOM
  float KPA = 0;
  float BAR = 0;
  float PSI = 0;

  //calculate pressure units
  if (CALUNITS == "psi") {
    KPA = P * 6.89;
    PSI = P;
    BAR = P / 14.7;
  };
  if (CALUNITS == "bar") {
    KPA = P * 101.325;
    PSI = P * 14.7;
    BAR = P;
  };
  if (CALUNITS == "kPa") {
    KPA = P;
    PSI = P * 6.89;
    BAR = P / 101.325;
  };

  //return unit selected
  if (DISPLAYUNITS == "psi") {
    P = PSI;
  };
  if (DISPLAYUNITS == "kPa") {
    P = KPA;
  }
  if (DISPLAYUNITS == "bar") {
    P = BAR;
  }

    return P;

}

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
  //alarm holding and setpoint
  int wt1_TAH = 0;       //0 = healthy, 1 = alarm
  float wt1_TAH_SP = 105;   // alarm high setpoint
  // ↑

/* ↓ tag:ot1 | description: Engine Oil Temperature
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
  //alarm holding and setpoint
  int ot1_TAH = 0;
  float ot1_TAH_SP = 105;   // alarm high setpoint
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
  float fl1;
  float fl2;
  //Fuel calibration - put instrument information here
  int fl1_LRV = 200; //counts on input at 0% level
  int fl1_URV = 950; //counts on input at 100% level
  int fl1_span = 95; //span in engineering units
  String fl1_calUnits = "mm"; //engineering units - mm, inches or percent
  String fl1_displayUnits = "%"; //engineering units - mm, inches or percent
  int fl2_LRV = 200; //counts on input at 0% level
  int fl2_URV = 950; //counts on input at 100% level
  int fl2_span = 100; //span in engineering units
  String fl2_calUnits = "mm"; //engineering units - mm, inches or percent
  String fl2_displayUnits = "%"; //engineering units - mm, inches or percent

  //fuel alarm holding and setpoint
  int fl1_LAL = 0;       //0 = healthy, 1 = alarm
  float fl1_LAL_SP = 20;   // alarm high setpoint in display UOM
  int fl2_LAL = 0;       //0 = healthy, 1 = alarm
  float fl2_LAL_SP = 20;   // alarm high setpoint in display UOM

//Pressure
  float op1;
  float bp1;
  //Pressure calibration - instrument information here
  int op1_LRV = 200;  //counts on input at 0% pressure
  int op1_URV = 950;  //counts on input at 100% pressure
  int op1_span = 5;   //URV in pressure units
  String op1_calUnits = "bar"; //units from instrument calibration. psi, kPa or bar
  String op1_displayUnits = "bar"; //units to use in program. psi, kPa or bar
  int bp1_LRV = 200;  //counts on input at 0% pressure
  int bp1_URV = 950;  //counts on input at 100% pressure
  int bp1_span = 10;  //URV in pressure units
  String bp1_calUnits = "bar"; //units from instrument calibration. psi, kPa or bar
  String bp1_displayUnits = "bar"; //units to use in program. psi, kPa or bar

  //pressure alarm holding and setpoint
  int op1_PAL = 0;       //0 = healthy, 1 = alarm
  float op1_PAL_SP = 1;   // alarm high setpoint in display UOM
  int bp1_PAL = 0;       //0 = healthy, 1 = alarm
  float bp1_PAL_SP = 5;   // alarm high setpoint in display UOM

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
  alarms(7);
  delay(2000);
}

void batteryV(int num) {
  for (int x = 0; x < 64; x++) { // multiple analogue readings for averaging
    ebv1_total = ebv1_total + analogRead(ebv1_pin); // add each value to a total
  }
  ebv1_volts = (ebv1_total / 64) * ebv1_resRatio * Aref / 1024 ; // convert readings to volt

  ebv1_VAL = lowAlarm(ebv1_volts, ebv1_VAL_SP);
  ebv1_VAH = highAlarm(ebv1_volts, ebv1_VAH_SP);

  ebv1_total = 0; // reset value
  delay(1000); // one second between measurements
}
void temperature(int num) {
  //tag: wt1
  wt1_TempC = Thermistor(wt1_pin,CELSIUS,wt1_beta,wt1_T0,wt1_R0,wt1_R1);

  wt1_TAH = highAlarm(wt1_TempC, wt1_TAH_SP); // 0 = healthy, 1 = alarm

  //tag: ot1
  ot1_TempC = Thermistor(ot1_pin,CELSIUS,ot1_beta,ot1_T0,ot1_R0,ot1_R1);

  ot1_TAH = highAlarm(ot1_TempC, ot1_TAH_SP); // 0 = healthy, 1 = alarm
}
void level(int num) {
  fl1 = linearLevel(fl1_pin,fl1_LRV,fl1_URV,fl1_span,fl1_calUnits,fl1_displayUnits);
  fl2 = linearLevel(fl2_pin,fl2_LRV,fl2_URV,fl2_span,fl2_calUnits,fl2_displayUnits);

  fl1_LAL = lowAlarm(fl1, fl1_LAL_SP); //input, setpoint
  fl2_LAL = lowAlarm(fl2, fl2_LAL_SP); //input, setpoint
}
void pressure(int num) {
  op1 = linearPressure(op1_pin,op1_LRV,op1_URV,op1_span,op1_calUnits,op1_displayUnits);
  bp1 = linearPressure(bp1_pin,bp1_LRV,bp1_URV,bp1_span,bp1_calUnits,bp1_displayUnits);

  op1_PAL = lowAlarm(op1, op1_PAL_SP);
  bp1_PAL = lowAlarm(bp1, bp1_PAL_SP);
}
void myDisplay(int num) {
//battery voltage alarm
 if(ebv1_VAH ==1){
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
  serialPrinter(ebv1_desc, ebv1_volts, ebv1_displayUnits, ebv1_VAL);//need to add an OR function for multiple alarms

  Serial.print("Engine Water Temp: ");
  Serial.print(wt1_TempC, 1);
  Serial.println("C");
  if (wt1_TAH == 1) {
    Serial.println("***Water Temp High Alarm***");
  }
  //
  //Oil Temp Sensor ot1
  //
  Serial.print("Engine Oil Temp: ");
  Serial.print(ot1_TempC, 1);
  Serial.println("C");
  if (ot1_TAH == 1) {
    Serial.println("***Oil Temp High Alarm***");
  }

  //Fuel Level Tank 1 fl1
  //
  Serial.print("Diesel Tank 1: ");
  Serial.print(fl1, 0); //0 decimal places
  Serial.println(fl1_displayUnits);
  if (fl1_LAL == 1) {
    Serial.println("***Fuel Tank 1 Low Alarm***");
  }
  //Fuel Level Tank 2 fl2
  //
  Serial.print("Diesel Tank 2: ");
  Serial.print(fl2, 0); //0 decimal places
  Serial.println(fl2_displayUnits);
  if (fl2_LAL == 1) {
    Serial.println("***Fuel Tank 2 Low Alarm***");
  }
  //Engine Oil Pressure op1
  //
  Serial.print("Engine Oil Pressure: ");
  Serial.print(op1, 1);
  Serial.println(op1_displayUnits);
  if (op1_PAL == 1) {
    Serial.println("***Engine Oil Pressure Low Alarm***");
  //Brake Pressure bp1
  //
  Serial.print("Brake Pressure: ");
  Serial.print(bp1, 1);
  Serial.println(bp1_displayUnits);
  if (bp1_PAL == 1) {
    Serial.println("***Brake Air Pressure Low Alarm***");
}
  // Battery Voltage Alarm
  Serial.print("Battery volts: ");
  Serial.print(ebv1_volts);
  Serial.println("volts");
  if(ebv1_VAH ==1){
    Serial.println("***Voltage High Alarm***");
 }
 Serial.println("");
 delay(800);
}
}
void alarms(int num) {
delay(10);
}
