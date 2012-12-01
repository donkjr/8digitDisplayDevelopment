// DisplayDemo
//
// This is a simple example of how to drive the ShieldStudio.com 4-digit Shield
// More information and a  library with much more functionality is available from
// shieldstudio.com - click downloads
//
// This scetch is also available for download
//
// Changes:
// 2010 July 27  - Created demo scetch
//
// Author: ShieldStudio (info@shieldstudio.com)

#include <Wire.h>                           //Include I2C library.

#define deviceaddress 0x50                  //this is the default address for an out of box 4-digit Shield

int ledPin = 13;

//define the digits
int dig3 = 0x23;                          
int dig2 = 0x22;
int dig1 = 0x21;
int dig0 = 0x20;
int startdigit = 0;

void setup()
{
  Serial.begin(9600);	// Debugging only
  Serial.println("Display Shield V1.0");

  
  pinMode(ledPin, OUTPUT);
  
  Wire.begin();                           // join i2c bus (address optional for master)
  Wire.beginTransmission(deviceaddress);  // transmit to device 
  Wire.send(0x04);                        // select configuration register MAX6953 Table 6
  Wire.send(0x01);                        // disable shutdown;
  Wire.endTransmission();

  //MAX6953 Table 23
  Wire.beginTransmission(deviceaddress); // transmit to device 
  Wire.send(0x01);                       // Set intensity for Digit 0 and 2
  Wire.send(0x33);                       //all segments 10 ma
  Wire.endTransmission();

  //MAX6953 Table 24
  Wire.beginTransmission(deviceaddress); // transmit to device 
  Wire.send(0x02);                       //Set intensity for Digit 1 and 3
  Wire.send(0x33);                       //all segments 10 ma
  Wire.endTransmission();

  //Turn on all LEDs in test mode.
  //MAX6953 Table 22
  Serial.println("Starting Display Test");
  Wire.beginTransmission(deviceaddress);
  Wire.send(0x07);                     
  Wire.send(0x01);
  Wire.endTransmission();
  
  delay(1000);             
  
  //disable test mode
  //MAX6953 Table 22
  Wire.beginTransmission(deviceaddress);
  Wire.send(0x07);              // sends one byte  
  Wire.send(0x00);
  Wire.endTransmission();
  
  Serial.println("Starting Self Test");
}


void writeChar(byte value,byte disp)
{
    Wire.beginTransmission(0x50);
    Wire.send(disp);              // sends one byte  
    if (value == '\0') Wire.send(' ');
    else Wire.send(value);
    Wire.endTransmission();
}



void loop()
{
  writeChar('S',dig3);
  writeChar('h',dig2);
  writeChar('i',dig1);
  writeChar('e',dig0);

  delay(400);
  writeChar('l',dig3);
  writeChar('d',dig2);
  writeChar('S',dig1);
  writeChar('t',dig0);

  delay(400);
  writeChar('u',dig3);
  writeChar('d',dig2);
  writeChar('i',dig1);
  writeChar('o',dig0);
  
  delay(400);
  writeChar('.',dig3);
  writeChar('c',dig2);
  writeChar('o',dig1);
  writeChar('m',dig0);  
  
  delay(500);
    
}  
  

