// base info from GeoMar - http://hobby-geomar.blogspot.com/
// adapted by Nicu FLORICA (niq_ro) - https://nicuflorica.blogspot.com/
// ver.0 - test or real (DS18B20 & RTC 0 DS1307 / DS3231), common cathode or common anode
// 21.12.2018 - Craiova
// ver.1 - similar, change DS18B20 with DHT(22, 11, etc)

#include <Wire.h>  //Included with Arduino IDE
#include "DHT.h"                         // DHT library
#include "RTClib.h"

byte digits[22] = {  //Bit pattern for 7 segment display
// -ABCDEFG            Segments labelled as per datasheet for Kingbright DA56-11EW dual common anode LED display module.
  B10000001,  // 0     1 represents segment OFF, 0 is ON
  B11001111,  // 1     the MSB is not used.
  B10010010,  // 2
  B10000110,  // 3
  B11001100,  // 4
  B10100100,  // 5
  B10100000,  // 6
  B10001111,  // 7
  B10000000,  // 8
  B10000100,  // 9
  B10011100,  // Degree symbol
  B10110001,  // Letter C     
  B11111110,  // - Symbol
  B10011111,  //  Letter E
  B11100011,   // u
  B11111111,   // Blank (OFF)
  B11110000,   // t
  B11101010,   // n
  B11000010,   // d
  B11100010,   // a
  B11111010,   // r
  B11101000    // h
};

int te[4] = {-23, -6, 4, 34};

int anodes[4] = {11,10,9,6};  //Common anodes for 7 segment displays connected to these digital outputs via NPN transistors
                            //Pin 11 is Left digit, 6 is Right digit.
int cathodes[8] = {8,7,12,5,4,3,2,13};  //Cathodes for each segment all tied together and connected to these digital outputs.
                  //Segment to pin assignment
                  //A=8,B=7,C=12,D=5,E=4,F=3,G=2,DP=13 segments
int data[4];  //4 byte array stores value to be displayed.
int startAddress = 0; //Data start address on the DS1307
int rtc[4];

// DHT sensor
#define senzor 14 // A0
#define DHTPIN senzor     // what pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);
// NOTE: For working with a faster chip, like an Arduino Due or Teensy, you
// might need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// Example to initialize DHT sensor for Arduino Due:
//DHT dht(DHTPIN, DHTTYPE, 30);

RTC_DS1307 ds1307;

unsigned long starttime;                   
unsigned long endtime; 

boolean bitState;
float temp;  // value for read temperature (as float);
byte semn = 1;  // 1 for + and  for -
int t = 0;  // temperaturea as integer
byte x = 0;  // for test value
byte p = 2; // btighness
int has;  //value for humidity

              
int maxim = 2000; // maxim brigghtness
int DISPLAY_BRIGHTNESS = 2000;
//Display brightness
//Each digit is on for a certain amount of microseconds
//Then it is off until we have reached a total of 20ms for the function call
//Let's assume each digit is on for 1000us
//Each digit is on for 1ms, there are 4 digits, so the display is off for 16ms.
//That's a ratio of 1ms to 16ms or 6.25% on time (PWM).
//Let's define a variable called brightness that varies from:
//5000 blindingly bright (15.7mA current draw per digit)
//2000 shockingly bright (11.4mA current draw per digit)
//1000 pretty bright (5.9mA)
//500 normal (3mA)
//200 dim but readable (1.4mA)
//50 dim but readable (0.56mA)
//5 dim but readable (0.31mA)
//1 dim but readable in dark (0.28mA)

byte test = 0; // 1 - test if work ok
               //  0 - RTC clock and thermometer

byte tip = 0; // 0 = common cathode
              // 1 = common anode


void setup()
{
  for (int i = 0; i < 4; i++)  //Set anode pins mode to output and put them LOW.
  {
    pinMode(anodes[i], OUTPUT);
if (tip == 1) digitalWrite(anodes[i], LOW);
  else  digitalWrite(anodes[i], HIGH);
  }
  
  for (int i = 0; i < 8; i++)  //Set cathode pins mode to output and put them HIGH.
  {
    pinMode(cathodes[i], OUTPUT);
if (tip == 1) digitalWrite(cathodes[i], HIGH);
 else digitalWrite(cathodes[i], LOW);

  }

dht.begin();  //  DHT sensor
 
  Wire.begin();  // Start the I2C bus.
  ds1307.begin(); //start RTC Clock
  //ds1307.adjust(DateTime(__DATE__, __TIME__));  // sets the RTC to the date & time this sketch was compiled
  DateTime now = ds1307.now();
  readTime();
}

void loop()
{
  
readTime();  // read clock

scrollIn(data);

for (byte i=0; i <= 10; i++)  // show clock
 {
 starttime = millis();                   
 endtime = starttime;                    //Store the internal timer counter value to make this loop run for a set period.  
  while ((endtime - starttime) <=500) // do this loop for 5000mS
  {
  outputDisplay(data);                 //Send to diaplay.
  endtime = millis();                  //Read internal timer counter to see how long this loop has been running.
  }
  starttime = millis();                   
  endtime = starttime;     
  while ((endtime - starttime) <=500) // do this loop for 5000mS
  {
  outputDisplay1(data);                 //Send to diaplay.
  endtime = millis();                  //Read internal timer counter to see how long this loop has been running.
  } 
 }  
 scrollOut(data);
 
readTemp();  // read temperature sensor

  scrollIn(data);
   starttime = millis();                   
  endtime = starttime;                    //Store the internal timer counter value to make this loop run for a set period.  
  while ((endtime - starttime) <=2500) // do this loop for 5000mS
  {
  outputDisplay(data);                 //Send to diaplay.
  endtime = millis();                  //Read internal timer counter to see how long this loop has been running.
  }
  scrollOut(data);
//  delay(750);

readHum();  // read humidity sensor

  scrollIn(data);
   starttime = millis();                   
  endtime = starttime;                    //Store the internal timer counter value to make this loop run for a set period.  
  while ((endtime - starttime) <=2500) // do this loop for 5000mS
  {
  outputDisplay(data);                 //Send to diaplay.
  endtime = millis();                  //Read internal timer counter to see how long this loop has been running.
  }
  scrollOut(data);
//  delay(750);

readData();  // read data

   scrollIn(data);
    starttime = millis();                   
  endtime = starttime;                    //Store the internal timer counter value to make this loop run for a set period.  
  while ((endtime - starttime) <=2500) // do this loop for 5000mS
  {
  outputDisplay1(data);                 //Send to diaplay.
  endtime = millis();                  //Read internal timer counter to see how long this loop has been running.
  }
  scrollOut(data);
//  delay(750);

}  


void outputDigit(int seg)    //Outputs segment data for an individual digit.
{
  for (int s = 0; s < 7; s++)  //Read a bit at a time from the selected digit in the digits array and output it to the correct
  {                            //pin
 if (tip == 1) bitState = bitRead(digits[seg], s);  //Read the current bit.
   else bitState = 1-bitRead(digits[seg], 6-s);  //Read the current bit.
    digitalWrite(cathodes[s], bitState);         //and output it.
  }
}



void outputDisplay(int dig[4])        //Scan the display once with the 4 digit int array passed in.
{
  for (int d = 0; d < 4; d++)
  {
  outputDigit(dig[d]);                //Set up the segment pins with the correct data.
if (tip == 1) digitalWrite(anodes[d], HIGH);      //Turn on the digit.
 else digitalWrite(anodes[d], LOW);      //Turn on the digit.  
//  delay(p);   //Hold it on for 2ms to improve brightness.
 if (tip == 1) digitalWrite(cathodes[7], 1);
  else digitalWrite(cathodes[7], 0); 
 delayMicroseconds(DISPLAY_BRIGHTNESS); 

if (tip == 1) digitalWrite(anodes[d], LOW);       //And turn it off again before looping untl all digits have been displayed.
  else digitalWrite(anodes[d], HIGH);       //And turn it off again before looping untl all digits have been displayed.
  }
}

void outputDisplay1(int dig[4])        //Scan the display once with the 4 digit int array passed in.
{
  for (int d = 0; d < 4; d++)
  {
  outputDigit(dig[d]);                //Set up the segment pins with the correct data.
   if (d ==1) 
   {
  if (tip == 1) digitalWrite(cathodes[7], 0);
  else digitalWrite(cathodes[7], 1); 
   }
   else 
   {
  if (tip == 1) digitalWrite(cathodes[7], 1);
  else digitalWrite(cathodes[7], 0); 
   }  
   if (tip == 1) digitalWrite(anodes[d], HIGH);      //Turn on the digit.
  else digitalWrite(anodes[d], LOW);      //Turn on the digit.   
//  delay(p);                           //Hold it on for 2ms to improve brightness.
 delayMicroseconds(DISPLAY_BRIGHTNESS); 

if (tip == 1)  digitalWrite(anodes[d], LOW);       //And turn it off again before looping untl all digits have been displayed.
 else digitalWrite(anodes[d], HIGH);       //And turn it off again before looping untl all digits have been displayed.
  }
}

void readTemp()
{
  if (test == 0)  // real
  {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  temp = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(has) || isnan(temp)) {
    temp = 99;
    return;     
    }
  }
  else  // test
  {
 x = x+1;
 if (x>3) x =0;
 temp = te[x];
  }

  if (temp >= 0) semn = 1;  // if temperature is positive
  else 
  {
  semn = 0; 
  temp = -temp;  // made positive value
  }
  t = temp;                         //Convert the temperature to int for display

if (semn == 1)
{
if (t >= 10)
{
  data[0] = (t / 10);                  //Calculate and store temperature 10s value
  data[1] = (t % 10);                  //Calculate and store temperature 1s value.
  data[2] = 10;                        //Degree symbol
  data[3] = 11;                        //Letter C for Celcius
}
else 
{
  data[0] = 15;                        //none
  data[1] = t;                       //number.
  data[2] = 10;                        //Degree symbol
  data[3] = 11;                        //Letter C for Celcius
}
}

if (semn == 0)
{
if (t >=10)
{
  data[0] = 12;                        // - (negative sign)
  data[1] = (t / 10);                  //Calculate and store temperature 10s value
  data[2] = (t % 10);                  //Calculate and store temperature 1s value.
  data[3] = 10;                        //Degree symbol (Celcius)
}
else
{
  data[0] = 12;                        //negative sign
  data[1] = t;                       //number.
  data[2] = 10;                        //Degree symbol
  data[3] = 11;                        //Letter C for Celsius)
}
}
}


void readHum()
{
  if (test == 0)  // real
  {
   // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  has = dht.readHumidity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(has) || isnan(temp)) {
    has = 99;
    return;
  }
  data[0] = (has / 10);                  //Calculate and store humidity 10x value
  data[1] = (has % 10);  
  }
  else  // test
  {
//  data[0] = 1;
  data[0] = 3;
  data[1] = 7;

  }
if (data[0] == 0) data[0] = 15; // none
  data[2] = 20;  // letter r
  data[3] = 21;  // letter h
}



void readTime()
{
  if (test == 0)  // real
  {
  DateTime now = ds1307.now();
  rtc[3] = now.hour();
  rtc[4] = now.minute();
  data[0] = (rtc[3] / 10);
  data[1] = (rtc[3] % 10);
  data[2] = (rtc[4] / 10);
  data[3] = (rtc[4] % 10);
  }
  else  // test
  {
//  data[0] = 1;
  data[0] = 0;
  data[1] = 2;
  data[2] = 5;
  data[3] = 6;
  }
if (data[0] == 0) data[0] = 15; // none

}

void readData()
{
  if (test == 0)  // real
  {
  DateTime now = ds1307.now();
  rtc[1] = now.month();
  rtc[2] = now.day();
  data[0] = (rtc[2] / 10);
  data[1] = (rtc[2] % 10);
  data[2] = (rtc[1] / 10);
  data[3] = (rtc[1] % 10);
  }
  else
  {
  data[0] = 2;
  data[1] = 3;
  data[2] = 1;
  data[3] = 2;
  }
}

void scrollIn(int sDig[4])    //Scrolls data on to the display from blank.
{
  int scrollBuffer[8];        //Stores the entire set of data that will be shifted along the display.
  scrollBuffer[0] = 15;       //15 represents a blank digit.
  scrollBuffer[1] = 15;
  scrollBuffer[2] = 15;
  scrollBuffer[3] = 15;
  scrollBuffer[4] = sDig[0];
  scrollBuffer[5] = sDig[1];
  scrollBuffer[6] = sDig[2];
  scrollBuffer[7] = sDig[3];

  for (int st=0; st < 4; st++)  //Loop 4 times and read from next entry in the array each time.
  {
    int sData[4] = {scrollBuffer[(0 + st)], scrollBuffer[(1 + st)], scrollBuffer[(2 + st)], scrollBuffer[(3 + st)]};
    for (int sc=0; sc < 10; sc++)  //Refresh the display 10 times after each shift to give the illusion of scrolling.
    {
      outputDisplay(sData);
    }
  }
}

void scrollOut(int sDig[4])   //Scrolls data off the display to blank
{    
  int scrollBuffer[8];        //Stores the entire set of data that will be shifted along the display.
  scrollBuffer[0] = sDig[0];  
  scrollBuffer[1] = sDig[1];
  scrollBuffer[2] = sDig[2];
  scrollBuffer[3] = sDig[3];
  scrollBuffer[4] = 15;        //15 represents a blank digit.
  scrollBuffer[5] = 15;
  scrollBuffer[6] = 15;
  scrollBuffer[7] = 15;
  
  for (int st=0; st < 4; st++)  //Loop 4 times and read from next entry in the array each time.
  {
    int sData[4] = {scrollBuffer[(0 + st)], scrollBuffer[(1 + st)], scrollBuffer[(2 + st)], scrollBuffer[(3 + st)]};
    for (int sc=0; sc < 10; sc++)  //Refresh the display 10 times after each shift to give the illusion of scrolling.
    {
      outputDisplay(sData);
    }
  }
}
