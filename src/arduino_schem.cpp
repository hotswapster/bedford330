#include <Arduino.h>
#include <math.h>
#include <Wire.h> //for lcd menu

void batteryV(int num);
void temperature(int num);
void level(int num);
void pressure(int num);
void myDisplay(int num);
void serialPrint(int num);
void alarms(int num);
void mainMenuDraw();
void drawCursor();
void operateMainMenu();
//int evaluateButton();
void menuItem1();
void menuItem2();
void menuItem3();
void menuItem4();
void menuItem5();
void menuItem6();
void menuItem7();
void menuItem8();
void menuItem9();
void menuItem10();

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

// *** TAGS *** //

  /* ↓ tag: ebv1 | description: Engine Battery Voltage
    functional description: 0 - ~16volt voltmeter uses the stable internal 1.1volt reference. 6k8 resistor from A0 to ground, and 100k resistor from A0 to +batt 100n capacitor from A0 to ground for stable readings
    */

    float Aref = 5.073;         // ***calibrate battery voltage here*** | change this to the actual Aref voltage of Arduino
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
    String wt1_desc = "Engine Water Temp"; //description of tag
    String wt1_displayUnits = "°C"; //engineering units to display in
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

  /* ↓ tag:fl1 | description: Fuel Level
  */
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
//end TAGS



// for display ↓===========================
  #include <LiquidCrystal.h>
  // You can have up to 10 menu items in the menuItems[] array below without having to change the base programming at all. Name them however you'd like. Beyond 10 items, you will have to add additional "cases" in the switch/case
  // section of the operateMainMenu() function below. You will also have to add additional void functions (i.e. menuItem11, menuItem12, etc.) to the program.
  String menuItems[] = {"ITEM 1", "ITEM 2", "ITEM 3", "ITEM 4", "ITEM 5", "ITEM 6"};

  // Navigation button variables
  int readKey;
  int savedDistance = 0;

  // Menu control variables
  int menuPage = 0;
  int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
  int cursorPosition = 0;

  // Creates 3 custom characters for the menu display
  byte downArrow[8] = {
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b10101, // * * *
    0b01110, //  ***
    0b00100  //   *
  };

  byte upArrow[8] = {
    0b00100, //   *
    0b01110, //  ***
    0b10101, // * * *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100  //   *
  };

  byte menuCursor[8] = {
    B01000, //  *
    B00100, //   *
    B00010, //    *
    B00001, //     *
    B00010, //    *
    B00100, //   *
    B01000, //  *
    B00000  //
  };

  // select the pins used on the LCD panel (DF0009)
  LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
  // buttons require 5v refernce with this shield.
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
  }
  // for display ↑==============================

//***Templates***//

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

/*serial print template
 Serial functino Inputs
1. description
2. value
3. unit of measure (display units)
4. alarm bit
*/
/*
    float serialPrinter(String DESC, float VAL, String UOM, int ALARM){
      String tag = DESC + ": " + VAL + " " + UOM;
      String alarmText = " ";

      if (ALARM == 1) {
        alarmText = "*** " + DESC + "ALARM ***";
      }

    return
      Serial.println(alarmText);
      Serial.println(tag);

    }
    //serial print template ↑
*/
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
  float MM = 0.0;
  float IN = 0.0;
  float PERCENT = 0.0;

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



void setup() {

  //analogReference(INTERNAL1V1); // use the internal ~1.1volt reference  | change (INTERNAL) to (INTERNAL1V1) for a Mega
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
  // splash screen ↑===============================

//for menu

// Initializes and clears the LCD screen
lcd.begin(16, 2);
lcd.clear();

// Creates the byte for the 3 custom characters
lcd.createChar(0, menuCursor);
lcd.createChar(1, upArrow);
lcd.createChar(2, downArrow);

//end menu

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
  //myDisplay(5);
  serialPrint(6);
  alarms(7);
  delay(2000);
  mainMenuDraw();//for menu
  drawCursor();//for menu
  operateMainMenu();//for menu
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
/* void myDisplay(int num) {
//taken out fo menu text




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

   switch (lcd_key)
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
    }
  // for display ↑
//}
}
*/
void serialPrint(int num) {
  //
  //Water Temp Sensor wt1
  //

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
// This function will generate the 2 menu items that can fit on the screen. They will change as you scroll through your menu. Up and down arrows will indicate your current menu position.
void mainMenuDraw() {
  Serial.print(menuPage);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}

// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor() {
  for (int x = 0; x < 2; x++) {     // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }

  // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
  // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}


void operateMainMenu() {
int evaluateButton(int readkey);
  int activeButton = 0;
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0: // When button returns as 0 there is no action taken
        break;
      case 1:  // This case will execute if the "forward" button is pressed
        button = 0;
        switch (cursorPosition) { // The case that is selected here is dependent on which menu page you are on and where the cursor is.
          case 0:
            menuItem1();
            break;
          case 1:
            menuItem2();
            break;
          case 2:
            menuItem3();
            break;
          case 3:
            menuItem4();
            break;
          case 4:
            menuItem5();
            break;
          case 5:
            menuItem6();
            break;
          case 6:
            menuItem7();
            break;
          case 7:
            menuItem8();
            break;
          case 8:
            menuItem9();
            break;
          case 9:
            menuItem10();
            break;
        }
        activeButton = 1;
        mainMenuDraw();
        drawCursor();
        break;
      case 2:
        button = 0;
        if (menuPage == 0) {
          cursorPosition = cursorPosition - 1;
          cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        }
        if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
      case 3:
        button = 0;
        if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition + 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
    }
  }
}

// This function is called whenever a button press is evaluated. The LCD shield works by observing a voltage drop across the buttons all hooked up to A0.
int evaluateButton(int x) {
  int result = 0;
  if (x < 50) {
    result = 1; // right
  } else if (x < 195) {
    result = 2; // up
  } else if (x < 380) {
    result = 3; // down
  } else if (x < 790) {
    result = 4; // left
  }
  return result;
}

// If there are common usage instructions on more than 1 of your menu items you can call this function from the sub
// menus to make things a little more simplified. If you don't have common instructions or verbage on multiple menus
// I would just delete this void. You must also delete the drawInstructions()function calls from your sub menu functions.
void drawInstructions() {
  lcd.setCursor(0, 1); // Set cursor to the bottom line
  lcd.print("Use ");
  lcd.write(byte(1)); // Up arrow
  lcd.print("/");
  lcd.write(byte(2)); // Down arrow
  lcd.print(" buttons");
}

void menuItem1() { // Function executes when you select the 2nd item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 2");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem2() { // Function executes when you select the 2nd item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 2");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem3() { // Function executes when you select the 3rd item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 3");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem4() { // Function executes when you select the 4th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 4");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem5() { // Function executes when you select the 5th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 5");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem6() { // Function executes when you select the 6th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 6");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem7() { // Function executes when you select the 7th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 7");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem8() { // Function executes when you select the 8th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 8");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem9() { // Function executes when you select the 9th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 9");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem10() { // Function executes when you select the 10th item from main menu
  int activeButton = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Sub Menu 10");

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 4:  // This case will execute if the "back" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}
