/*  

  BonsaiBox v1 - "Enoki" 
  by Traverse Jurcisin

  BonsaiBox is an Arduino controlled growth chamber.
  The chamber is intended for the growth, maintenance, 
  and study of organsims like plants or fungi.
  The "Enoki" is fitted with hardware and software
  that is more catered to tropical species and fungi.
  
  Components of the BonsaiBox are:
    *1x Sonic fogger.
    *1x Air pump.
    *1x AC wall plug cable.
    *1x 12 VDC PC fan (with a built in 5 VDC ws2812 "NeoPixel" LED's).   
    *1x Liquid Crystal Display (20x4).
    *1x Relay (2 channel).  One channel for the mister, one for the air pump.
    *1x RTC.  For clock and timer related functions.
    *1x BMP 280.  For pressure, temperature, and humidity.
    *1x 120 VAC -> 24 VDC transformer.  Make sure it can produce enough amps!
    *1x 12 VDC buck converter.  Make sure it can produce enough amps!
    *1x 5 VDC buck converter.  Make sure it can produce enough amps!
  

To code:
* Scale light intensity with temperature.
* If RH is ok, but it's too hot, increase fan speed more.
* Scale fan with VPD
*/
//############################################################################################################################################
/*      - - -       INCLUSIONS      - - -      */
#include "DS3231.h"                 // Library needed for timer functions.
#include <Wire.h>                   // Library needed for I2C communcaitons.
#include "SparkFunBME280.h"`        // Library needed for BME280 environmental sensor.
#include <Adafruit_NeoPixel.h>      // library needed for communicating with NeoPixels.     
#include <LiquidCrystal_I2C.h>      // Library needed for using a I2C LCD.
#include <avr/power.h>
//############################################################################################################################################

//############################################################################################################################################
/*      - - -       DEFINITIONS      - - -      */
#define lightPin 6                   // Pin associated with controlling the neopixles. 
#define startHour 6                  // The hour the lights will come on at (24 hr format).
#define endHour 18                   // The hour the lights will turn off at (24 hr format).
#define fanPin 5                     // PWM capable pin associated with the fan.
#define fogPin 4                     // Digital pin attached to a relay contorling a mister.
#define airPin 3                     // The pin the attached to a relay contorling an air bubbler.
#define pulseMillis 500              // The length of a pulse if the fan is to be pulsed to slow it down.
#define displayInterval 1000         // The amount of milliseconds that needs to accrue for the display function to be called.
#define foggerInterval 10000         // The amount of milliseconds that needs to accrue for the fogger function to be called.
#define lightInterval 800           // The amount of milliseconds that needs to accrue for the light function to be called.
#define RH_LP  90                    // The lowest point the humidity should reach.
#define RH_UP  95                    // The maximum the humidity should be.
#define RH_CritLow 80                // The relative humidity that would be considered critically low.
#define RH_CritHigh 98               // The relative humidity that would be considered critically high.
#define temp_LP 70                   // The lowest tollerated temerature.
#define temp_UP 80                   // The highest tollerated temperature.
//###########################################################################################################################################

//############################################################################################################################################
/*      - - -       OBJECTS      - - -      */
LiquidCrystal_I2C lcd(0x27,20,4);
BME280 mySensorB;                    // Object for sensor functions.  Uses I2C address 0x76 (jumper closed)
Adafruit_NeoPixel strip =            // Object for defining NeoPixel parameters.
Adafruit_NeoPixel(60, lightPin, NEO_GRB + NEO_KHZ800);
RTClib RTC;                          // Object needed for timer functions.
//############################################################################################################################################

//###########################################################################################################################################
/*      - - -       GLOABL VARIABLES      - - -      */
bool fogFlag = false;                // Fan for controling fogger settings.
float CFMMax = (30.00*0.8);          // Maximum amount of air delivered by the fan(s) minus ~ 20% for obstructions.  May vary, check yours.
float CFM = 1.00;                    // Variable used in setting the cubic feet of air moved per minute by the fan.
int lightStartHour = 5;              // Hour (24h) that ligths will be active at.
int lightEndHour = (5+12);           // Hour (24h) that ;ights turn off at.
float RH_NOW;                        // The current relative humidity.
float temp_NOW;                      // The air temperature now.
float pressure_NOW;                  // The curreent air pressure.
int fanInterval = 250;               // The amount of milliseconds that needs to accrue for the fan function to be called.
unsigned long int fanLastRun = 0;         // The last time the fan function was ran.
unsigned long int displayLastRun = 0;     // The last time the display function was ran.
unsigned long int foggerLastRun = 0;      // The last time the fogger function was ran.
unsigned long int lightLastRun = 0;       // The last time the light function was ran.
unsigned long int millisNow = 0;          // A storage for the current up time.

String message; 
int messageInterval = 3500;
unsigned long int messageLastDisplayed = 0;
float VPD;
//############################################################################################################################################

//###########################################################################################################################################
/*    - - - CUSTOM FUNCTION : fanRun  - - -      */
void fanRun(){                                                                                 
  
  RH_NOW = mySensorB.readFloatHumidity();  
  temp_NOW = mySensorB.readTempF();
  
  
  if ((RH_NOW > RH_UP) && (CFM < CFMMax)){        // If the RH is too high, increase the fan speed.
    CFM = CFM + 0.001;    
    message = "      RH  high";
    Serial.print("RH high, increasing fan speed to ");
    Serial.print(CFM);
    Serial.println(" CFM. ");
  }
  if ((temp_NOW > temp_UP) && (CFM < CFMMax)){    // If the temperature is too high, increase the fan speed.
    CFM = CFM + 0.001;    
    message = "  Temperature high";
    Serial.print("Temperature high, increasing fan speed to ");
    Serial.print(CFM);
    Serial.println(" CFM. ");
  }
  if ((RH_NOW < RH_LP) && (CFM > 0)){        // If the RH is a little low, lower the fan speed.
    CFM = CFM - 0.002;    
    Serial.print("RH low, reducing fan speed to ");
    Serial.print(CFM);
    Serial.println(" CFM. ");
  }
  if((temp_NOW < temp_LP) && (CFM > 0)){     // If the temperature is low, lower the fan speed.
    CFM = CFM - 0.002;    
    message = " Temperature is low";
    Serial.print("Temperature low, reducing fan speed to ");
    Serial.print(CFM);
    Serial.println(" CFM. ");
  }
  float CFMPWM = (CFM*255.0000)/CFMMax;                 // A temporary holder used in setting low fan CFM's.
  int dutyCounter = 0;                                  // A counter used in setting low fan CFM's. 
  
  while ((CFMPWM < 2.00) &&(CFMPWM > 0)){
    CFMPWM = CFMPWM*2.00;
    Serial.print(" ... ");                                  
    dutyCounter++;
  }
  
  if(CFMPWM >= 2){            
    analogWrite(fanPin,CFMPWM);
    if (dutyCounter > 0){
      delay(pulseMillis);
      analogWrite(fanPin,0);
      fanInterval = pulseMillis * (2^dutyCounter);
    }
  }
}
//###########################################################################################################################################

//###########################################################################################################################################
/*    - - - CUSTOM FUNCTION : displayRun  - - -     */
void displayRun(){
  RH_NOW = mySensorB.readFloatHumidity();
  pressure_NOW = (mySensorB.readFloatPressure()/1000);    // Pa -> kPa (/101.325 -> Atm.)  
  temp_NOW = mySensorB.readTempF();
  DateTime now = RTC.now();
  lcd.clear();
  lcd.setCursor(5,0);                                 // Print hour
  if(now.hour()== 0){
    lcd.print("12");    
  }
  if(now.hour()<10){
    lcd.print("0");
    lcd.print(now.hour());
  }
  if(now.hour() >=13){
    lcd.print((now.hour()-12)); 
  }
  
  lcd.print(":");                                     // Print minute
  if(now.minute()<10){
    lcd.print("0");
  } 
  lcd.print(now.minute()); 
  
  lcd.print(":");                                     // Print second
  if(now.second()<10){
    lcd.print("0");
  }
  lcd.print(now.second());

  lcd.print("");
  if (now.hour()>=12){lcd.print(" PM");}                   // Print AM / PM
  else {lcd.print(" AM");}
  
  lcd.setCursor(1,1);                                 // Print RH
  lcd.print("RH:"); 
  lcd.print(RH_NOW);  
  lcd.print("%");

  lcd.setCursor(11,1);                                 // Print temperature
  lcd.print("T:"); 
  lcd.print(temp_NOW);  
  lcd.print("F");
  
  lcd.setCursor(0,2);                                 // Print fogger status 
  lcd.print("(FOG:");  
  if (fogFlag == false){lcd.print("OFF)");}  
  else{lcd.print(" ON) ");}
    
  lcd.setCursor(10,2);                                // Print fan status
  lcd.print("(CFM:");
  if(CFM >= 0){
    lcd.print(CFM);  
  }
  else{
    lcd.print("0.00");  
  }
  lcd.print(")");
  /*
  lcd.setCursor(0,3);                                 // 
  lcd.print("Pressure: ");
  lcd.print(pressure_NOW);
  lcd.print(" kPa  ");
  */
  lcd.setCursor(0,3);                                 // 
  lcd.print("VPD: ");
  getVPD();
  lcd.print(VPD);
  lcd.print(" kPa  ");

  if (millisNow >= (messageLastDisplayed+messageInterval)){
    lcd.setCursor(0,3);                                 //  
    lcd.print("                    ");
    lcd.setCursor(0,3);                                 //  
    lcd.print(message);
    messageLastDisplayed = millisNow;
  }
}
//###########################################################################################################################################

//###########################################################################################################################################
/*    - - - CUSTOM FUNCTION : foggerRun  - - -     */
void foggerRun(){ 
  if ((RH_NOW <= RH_LP) && (temp_NOW <= temp_UP)){      // If the RH is low and the temp isn't too high...                      
    digitalWrite(fogPin, LOW);                         // turn the foggger on. (For whatever reason, on my board it's backwards.)
    digitalWrite(airPin,LOW);
    fogFlag = true;
  }
  if ((RH_NOW >= RH_UP) || (temp_NOW > temp_UP)){     // If the RH is high enough or the temperature is too high...
    digitalWrite(fogPin, HIGH);                        // turn the fogger off if RH high enough.
    digitalWrite(airPin,HIGH);
    fogFlag = false;  
  }  
}
//###########################################################################################################################################

//###########################################################################################################################################
/*    - - - LIGHT FUNCTION: Wheel  - - -     */
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
//###########################################################################################################################################

//###########################################################################################################################################
/*    - - - LIGHT FUNCTION: runRrograms  - - -     */
void runPrograms(){
  millisNow = millis();
  if (millisNow >= (fanLastRun+fanInterval)){
    Serial.println("RUNNING FAN");
    fanRun();
    fanLastRun = millisNow;
  }
  if (millisNow >= (displayLastRun+displayInterval)){
    Serial.println("RUNNING DISPLAY");
    displayRun();
    displayLastRun = millisNow;
  }
  if (millisNow >= (foggerLastRun+foggerInterval)){
    Serial.println("RUNNING FOGGER");
    foggerRun();
    foggerLastRun = millisNow;
  }
}
//###########################################################################################################################################

//###########################################################################################################################################
/*    - - - CUSTOM FUNCTION : lightRun  - - -     */
void lightRun(){ 
  DateTime now = RTC.now();
  uint16_t n, i , j;
  if(now.hour() < startHour || now.hour() >= endHour){strip.show();}      // Lights off after hours.
  if(now.hour() >= startHour || now.hour() < endHour){                    // Lights on durring on hours.
    if (temp_NOW >= temp_UP){                                             // Turn lights off if too hot.
      return;
    }
    if (RH_NOW < RH_CritLow){                                            // When RH is critically low                               
      message = "RH is Critically Low";
      for(n=0; n<strip.numPixels(); n++) {
        strip.setPixelColor(n,50,0,0);
      }
      strip.show();
    }
    if (RH_NOW >= RH_CritLow && RH_NOW < RH_LP){                           // When RH is acceptable          
      message = "      RH is ok";
      for(n=0; n<strip.numPixels(); n++) {
        strip.setPixelColor(n,50,50,50);
      }
      strip.show();    
    }
  }    
  if (RH_NOW >= RH_LP && RH_NOW <= RH_CritHigh){                            // When RH is optimal                                
    message = "     RH optimal";
    for(j=0; j<256*5; j++) {                                                // 5 cycles of all colors on wheel
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
     }
      strip.show();
      delay(50);   
      runPrograms();
      if (temp_NOW >= temp_UP){     // If temperatures become to hot                                         
        for(n = 0;  n <strip.numPixels(); n++) {
        strip.setPixelColor(n,50,0,0);            // Set the lights to a low red
        }
        strip.show();
        return;                                   // Return to main loop
      }
    }
  if (RH_NOW > RH_CritHigh){                                              // When RH is too high                                
    message = "   RH is too high";
    for(n = 0;  n <strip.numPixels(); n++) {
      strip.setPixelColor(n,50,0,0);
    }
    strip.show(); 
  }
 }
}
//###########################################################################################################################################

//###########################################################################################################################################
/*    - - - CUSTOM FUNCTION : getVPD  - - -     */
void getVPD() {             
  float eS = exp(19.0177 - (5327.00 / (273.25 + (mySensorB.readTempC())))) * (mySensorB.readFloatPressure() / 101325);    // The air pressure that would be exerted by water vapor if the air was saturated.
  float eA = ((mySensorB.readFloatHumidity()) / 100) * eS;    // The actual air pressure of the water vapor.
  VPD = eS - eA;    // Vapor Pressure Deficit; the available water holding capacity of the air, highly correlative with evapotranspiration in plants.   
}
//###########################################################################################################################################

//###########################################################################################################################################
/*    - - - SETUP  - - -     */
void setup(){
  Serial.begin(9600);                 // Begin serial communications.
  Serial.println("Setup intitiated... ");
  
  Serial.println("Booting up LED's... ");
  Wire.begin();                       // Start I2C communications.lcd.init();
  strip.begin();                      // Start communications with the lights.
  strip.show();                       // Clear the lights.
  pinMode (airPin, OUTPUT);
  pinMode (fogPin, OUTPUT);           // Set fogger pin to output a control signal to a relay.
  pinMode (fanPin, OUTPUT);           // Set fan pin to output a PWM control signal to a fanspeed controller circuit. 
  digitalWrite(fogPin,HIGH);            // Turn off fogger.  For some reason its behaving backwards, high is off, low is on.
  analogWrite(fanPin,0);                // Turn off fan.

   
  Serial.println("Booting up LCD... ");                          // Start LCD.
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  Serial.println("Booting up sensors... ");
  mySensorB.setI2CAddress(0x76);        // Connect to the BME 280.
  mySensorB.beginI2C(); 
  DateTime now = RTC.now();

  Serial.println("Setup completed.");
      
}              
//###########################################################################################################################################

//###########################################################################################################################################
/*    - - - LOOP  - - -     */
void loop(){
  DateTime now = RTC.now();
  runPrograms();  
  if (millisNow >= (lightLastRun+lightInterval)){
    Serial.println("RUNNING LIGHTS");
    lightRun();
    lightLastRun = millisNow;
  }
}
//###########################################################################################################################################
