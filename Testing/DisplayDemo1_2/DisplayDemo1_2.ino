// DisplayDemo & Test

// This is a comprehensive tester to test serial 8 digit displays
// A library for 4 digit displays is available from
// shieldstudio.com - click downloads
//
// 
//
// Changes:
// 2013 Novemnber 18
// changed the "wsend" to Wire.send to be compatible with new wire librarires
// first setup of 8 digits by duplicating 4 digit display code for another 4 digits.
// started with DisplayDemo from Sheild Studios
// Author: don_kleinschnitz@hotmail.com
// The TWI is implemented in the Atmel processor "Wire" library interfaces with that hardware 
//http://arduino.cc/playground/Main/WireLibraryDetailedReference
//

//Library inclusions
#include <Wire.h>                           //Include I2C library.

//compliler directives
#define deviceAdrDU1 0x50                  //device address for sheild #1 left most on breadboard
#define deviceAdrDU2 0x51                  //this is address for sheild #2 right most on breadboad

//Define I/O
int ledPin = 13;

//define DU internal register addresses
byte intensity10RegAdr = 0x01; //intensity for digit one and zero
byte intensity32RegAdr = 0x02; //intensity for digit three and two
byte scanLimitRegAdr = 0x03; //scan limit register
byte configRegAdr = 0x04; //configuration register
byte userDefinedFontRegAdr = 0x05; //user defined font register
// NOTE:factory reserves address 0x06.... don't write or read
byte displayTestRegAdr = 0x07; //display test register

//Each DU has 4 digits with 2 planes each
//define the Plane 0 display digits addresses
int dig3 = 0x23;                          
int dig2 = 0x22;
int dig1 = 0x21;
int dig0 = 0x20;
//define the Plane 1 diplay digit addresses
int dig3P1 = 0x43;                          
int dig2P1 = 0x42;
int dig1P1 = 0x41;
int dig0P1 = 0x40;
//Misc defintions
byte conFigRegRead; //configuration read buffer
int _scrollspeed = 300; //scroll speed, time between display shifts in ms
int startdigit = 0;
int DU1 = 0x01; //    DU1 number identifier
int DU2 = 0x02;//
//temp config reg value registers
byte tempConfigDU1;//
byte tempConfigDU2;//
//Bit definitions (mostly matching the config register
byte noBit = 0x00; //no bits
byte sBit = 0x01; //bit 0 
byte oneBit =0x02; //bit 1
byte bBit = 0x04; //bit 2
byte eBit = 0x08; //bit 3
byte tBit = 0x10; //bit 4
byte rBit = 0x20; //bit 5
byte sixBit = 0x40; //bit 6
byte pBit = 0x80; //bit 7



//// -------SETUP THE DIPSPLAY

void setup()
{
  Serial.begin(9600);	// Debugging only
  Serial.println("...Display 8 digit Shield Test V1_2....");
  pinMode(ledPin, OUTPUT);//light for debugging only
  
  //The following stuff was not made as functions because you shouldnt have to call this stuff
  //Messing with the display intensity can result in damage to the power system
  //Here we go...initializing
  Wire.begin();                           // join i2c bus (address optional for master)
  Serial.println("Starting Up.........................");
  //Check the registers
  readControlRegisters();
  Serial.println("Initializing DU1");
  ///SETUP DU#1///
  Wire.begin();                           // join i2c bus (address optional for master)
  //Write sequence
  Wire.beginTransmission(deviceAdrDU1);  // transmit to device 
  Wire.write(0x04);                        // select configuration register MAX6953 Table 6
  Wire.write(0x01);                        // disable shutdown by setting the "s" bits
  Wire.endTransmission();//Disconnect (stop) from the I2C buss 
  //Write sequence
  Wire.beginTransmission(deviceAdrDU1); // transmit to device 
  Wire.write(0x01);                       // Adress intensity reg for Digit 0 and 2,//MAX6953 Table 23
  Wire.write(0x33);                       //set all segments 10 ma
  Wire.endTransmission();
  //Write sequence
  Wire.beginTransmission(deviceAdrDU1); // transmit to device 
  Wire.write(0x02);                       //Adress intensity reg for Digit 1 and 3, MAX6953 Table 24
  Wire.write(0x33);                       //set all segments 10 ma
  Wire.endTransmission();
  ////SETUP DU#2////
  Serial.println("Initializing DU2");
  Wire.begin();                           // join i2c bus (address optional for master)
   //Write sequence
  Wire.beginTransmission(deviceAdrDU2);  // transmit to device 
  Wire.write(0x04);                        // select configuration register MAX6953 Table 6
  Wire.write(0x01);                        // disable shutdown by setting the "s" bits
  Wire.endTransmission();
  //Write sequence
  Wire.beginTransmission(deviceAdrDU2); // transmit to device 
  Wire.write(0x01);                       // Set intensity for Digit 0 and 2
  Wire.write(0x33);                       //all segments 10 ma
  Wire.endTransmission();
  //Write sequence
  Wire.beginTransmission(deviceAdrDU2); // transmit to device 
  Wire.write(0x02);                       //Adr intensity reg for Digit 1 and 3
  Wire.write(0x33);                       //set all segments 10 ma
  Wire.endTransmission();
  tempConfigDU1 = 0x01; // intialize the temp config register with shutdown disabled
  tempConfigDU2 = 0x01; // intialize the temp config register with shutdown disabled
  writeConfig();//Write it to the DU's
  write8Digits("BootDone",0);
  delay(400);
  //We are done and DU's should be ready....
}


/// MAIN LOOP/////
void loop()
{
  Serial.println();
  Serial.println("......Start main test routine.......................");
  readControlRegisters();
  readDisplayRegisters();
  clrDU();// clear the displays
  //Test LEDs
  write8Digits("Test-LED",0);
  delay(800);
  testLedDU1();//test all diodes on DU#1's first
  testLedDU2();//Then DU#2 
  //Test scroll
  setScroll(300);
  clrDU();
  write8Digits("Test num",0);
  delay(1000);
  writeString("12345678",0);
  delay(400);
  clrDU();
  write8Digits("Tst alpha",0);
  delay(1000);
  writeString("ABCDEFGHIJKLMNOPQRSTUVWXYZ",0);
  delay (400);
  //Test single digit writing
  clrDU();
  write8Digits("Tst Digs",0);
  delay(1000);
  clrDU();
  Serial.println("Display ABCD On DU1");
  writeCharDU1('A',dig3);
  writeCharDU1('B',dig2);
  writeCharDU1('C',dig1);
  writeCharDU1('D',dig0);
  Serial.println("Display EFGH On DU2");
  writeCharDU2('E',dig3);
  writeCharDU2('F',dig2);
  writeCharDU2('G',dig1);
  writeCharDU2('H',dig0);
  readDisplayRegisters();//verify what is written 
  delay(400);
  Serial.println("Set blink ON");
  blinkFast();
  delay(4000); // wait to see them blink
  Serial.println("Turn blink OFF");
  blinkOff();
  //Test single digit erasing
  Serial.println("ERASE ABCD On DU1");
  writeCharDU1('\0',dig3);
  writeCharDU1('\0',dig2);
  writeCharDU1('\0',dig1);
  writeCharDU1('\0',dig0);
  Serial.println("ERASE EFGH On DU2");
  writeCharDU2('\0',dig3);
  writeCharDU2('\0',dig2);
  writeCharDU2('\0',dig1);
  writeCharDU2('\0',dig0);  
  delay(500); 
 }  

//------------------FUNCTIONS-----------------------//

//Write characters to display 1
//value: character to write
//disp: digit address
void writeCharDU1(byte value,byte disp)
{
  Wire.beginTransmission(deviceAdrDU1);
  Wire.write(disp);              // sends digit adr  
  if (value == '\0') Wire.write(' ');
  else Wire.write(value);//write the character
  Wire.endTransmission();
  delay(400);
}

//Write characters to display 2
//value: character to write
//disp: digit address
void writeCharDU2(byte value,byte disp)
{
  Wire.beginTransmission(deviceAdrDU2);
  Wire.write(disp);              // sends one byte  
  if (value == '\0') Wire.write(' ');
  else Wire.write(value);
  Wire.endTransmission();
  delay(400);
}

//Test the LEDs by putting controller in test mode, waiting and then reseting back to normal.
//TURN ON/OFF DU#1 TEST MODE
//no params
void testLedDU1()
{
  Serial.println("Turning ON/OFF DU#1 LEDs");
  //Turn on DU1 test mode
  Wire.beginTransmission(deviceAdrDU1);
  Wire.write(0x07);                     
  Wire.write(0x01);//on bit
  Wire.endTransmission();
  delay(1000);             
  //disable DU1 test mode
  //MAX6953 Table 22
  Wire.beginTransmission(deviceAdrDU1 );
  Wire.write(0x07);              // sends one byte  
  Wire.write(0x00);//off bit
  Wire.endTransmission();
}

 //TURN ON/OFF DU#2 TEST MODE
 //no parama
void testLedDU2()
 {
  Serial.println("Turning ON/OFF DU#2 LED's");
  Wire.beginTransmission(deviceAdrDU2);
  Wire.write(0x07);                     
  Wire.write(0x01);
  Wire.endTransmission();
  delay(1000);             
  //disable DU2 test mode
  Wire.beginTransmission(deviceAdrDU2);
  Wire.write(0x07);              // sends one byte  
  Wire.write(0x00);
  Wire.endTransmission();
}

//Clear both planes on displays
//Clear both DU's
//no params
void clrDU() 
{
  write8Digits("        ",0);
}

/* THIS VERSION DID NOT WORK... did nothing
{
  tempConfigDU1 |= rBit;	//set the p bit clearing all displays  
  writeConfig();	//write config to controller
  tempConfigDU1 &= ~rBit;	//reset the r bit turning off  bit disabling global blink
  writeConfig();
}
*/
// Read the entire control register set and print out on display
// Read and display DU1 then DU2
//no params
void readControlRegisters()
{
  //local intializations
  byte regValue = 0;//intialize the control register value buffer
  //intitialize an array of strings for the register name fields
  char* name[] ={"Intensity10","Intensity32","Scan Limit","Configuration","Usr Fonts Adr","Test All LEDs"};
  //Start reading registers
  //Print header
  printTableHeader();// Print the register table header
  // Read DU#1 registers
  //read digit 0&1 intensity register
  regValue=readAreg(deviceAdrDU1,intensity10RegAdr);//read intensity control register value
  printTableRow(deviceAdrDU1, name[0], intensity10RegAdr, regValue); //print a row in the table for the first intensity register
  //Read digit 2&3 intensity register
  regValue=readAreg(deviceAdrDU1,intensity32RegAdr);//read intensity control register value
  printTableRow(deviceAdrDU1, name[1],intensity32RegAdr, regValue); //print a row in the table for the second intensity register
  //Read scan limit register
  regValue=readAreg(deviceAdrDU1,scanLimitRegAdr);//read scan limit control register value
  printTableRow(deviceAdrDU1, name[2],scanLimitRegAdr, regValue); //print a row in the table
  //read configuration register
  regValue=readAreg(deviceAdrDU1,configRegAdr);//read configuration control register value
  printTableRow(deviceAdrDU1, name[3], configRegAdr, regValue); //print a row in the table
  //Read user defined font adr register
  regValue=readAreg(deviceAdrDU1,userDefinedFontRegAdr);//read user font adr register value
  printTableRow(deviceAdrDU1, name[4],userDefinedFontRegAdr, regValue); //print a row in the table
  //Read digit 2&3 intensity register
  regValue=readAreg(deviceAdrDU1,displayTestRegAdr);//read display test register value
  printTableRow(deviceAdrDU1, name[5], displayTestRegAdr, regValue); //print a row in the table
 
  // Read DU#2 control registers
  //read digit 0&1 intensity register
  regValue=readAreg(deviceAdrDU2,intensity10RegAdr);//read intensity control register value
  printTableRow(deviceAdrDU2, name[0], intensity10RegAdr, regValue); //print a row in the table for the first intesity register
  //Read digit 2&3 intensity register
  regValue=readAreg(deviceAdrDU2,intensity32RegAdr);//read intensity control register value
  printTableRow(deviceAdrDU2, name[1],intensity32RegAdr, regValue); //print a row in the table for the second intensity register
  //Read scan limit register
  regValue=readAreg(deviceAdrDU2,scanLimitRegAdr);//read scan limit control register value
  printTableRow(deviceAdrDU2, name[2],scanLimitRegAdr, regValue); //print a row in the table
  //read configuration register
  regValue=readAreg(deviceAdrDU2,configRegAdr);//read configuration control register value
  printTableRow(deviceAdrDU2, name[3], configRegAdr, regValue); //print a row in the table
  //Read user defined font adr register
  regValue=readAreg(deviceAdrDU2,userDefinedFontRegAdr);//read user font adr register value
  printTableRow(deviceAdrDU2, name[4],userDefinedFontRegAdr, regValue); //print a row in the table
  //Read digit 2&3 intensity register
  regValue=readAreg(deviceAdrDU2,displayTestRegAdr);//read display test register value
  printTableRow(deviceAdrDU2, name[5], displayTestRegAdr, regValue); //print a row in the table
}

//Read the display char registers
//n params
void readDisplayRegisters()
{
 //local intializations
  byte regValue = 0;//intialize the control register value buffer
  //intitialize an array of strings for the register name fields
  char* name[] ={"Digit0-Plane0","Digit1-Plane0","Digit2-Plane0","Digit3-Plane0","Digit0-Plane1","Digit1-Plane1","Digit2-Plane1","Digit3-Plane1"};
  //Start reading registers
  //Print header
  printTableHeader();// Print the register table header
  // Read DU#1 Plane 0 registers
  //read digit 0 "leftmost"
  regValue=readAreg(deviceAdrDU1,dig0);//read digit 0-Plane 0- DU#1
  printTableRow(deviceAdrDU1, name[0],dig0, regValue); //print a row in the table for the first intensity register
  //Read digit 1
  regValue=readAreg(deviceAdrDU1,dig1);//
  printTableRow(deviceAdrDU1, name[1],dig1, regValue); //print a row in the table for the second intensity register
  //Read digit 2
  regValue=readAreg(deviceAdrDU1,dig2);// 
  printTableRow(deviceAdrDU1, name[2],dig2, regValue); //print a row in the table
  //read digit 3
  regValue=readAreg(deviceAdrDU1,dig3);//
  printTableRow(deviceAdrDU1, name[3], dig3, regValue); //print a row in the table
  //Read DU #1 Plane 1 Registers
 //read digit 0 "leftmost"
  regValue=readAreg(deviceAdrDU1,dig0P1);//
  printTableRow(deviceAdrDU1, name[4], dig0P1, regValue); //print a row in the table for the first intensity register
  //Read digit 1
  regValue=readAreg(deviceAdrDU1,dig1P1);//
  printTableRow(deviceAdrDU1, name[5],dig1P1, regValue); //print a row in the table for the second intensity register
  //Read digit 2
  regValue=readAreg(deviceAdrDU1,dig2P1);//
  printTableRow(deviceAdrDU1, name[6],dig2P1, regValue); //print a row in the table
  //read digit 3
  regValue=readAreg(deviceAdrDU1,dig3P1);//
  printTableRow(deviceAdrDU1, name[7], dig3P1, regValue); //print a row in the table
 //Now DU#2 display registers
 // Read DU#2 Plane 0 registers
  //read digit 0 "leftmost"
  regValue=readAreg(deviceAdrDU2,dig0);//
  printTableRow(deviceAdrDU2, name[0], dig0, regValue); //print a row in the table 
  //Read digit 1
  regValue=readAreg(deviceAdrDU2,dig1);//
  printTableRow(deviceAdrDU2, name[1],dig1, regValue); //print a row in the table 
  //Read digit 2
  regValue=readAreg(deviceAdrDU2,dig2);//
  printTableRow(deviceAdrDU2, name[2],dig2, regValue); //print a row in the table
  //read digit 3
  regValue=readAreg(deviceAdrDU2,dig3);//
  printTableRow(deviceAdrDU2, name[3], dig3, regValue); //print a row in the table
  //Read DU #2 Plane 1 Registers
 //read digit 0 "leftmost"
  regValue=readAreg(deviceAdrDU2,dig0P1);//
  printTableRow(deviceAdrDU2, name[4], dig0P1, regValue); //print a row in the table 
  //Read digit 1
  regValue=readAreg(deviceAdrDU2,dig1P1);//read intensity control register value
  printTableRow(deviceAdrDU2, name[5],dig1P1, regValue); //print a row in the table 
  //Read digit 2
  regValue=readAreg(deviceAdrDU2,dig2P1);//
  printTableRow(deviceAdrDU2, name[6],dig2P1, regValue); //print a row in the table
  //read digit 3
  regValue=readAreg(deviceAdrDU2,dig3P1);//
  printTableRow(deviceAdrDU2, name[7], dig3P1, regValue); //print a row in the table 
} 
  

//Read a single register
//DUadr: address of Display Unit
//regAdr: address of the register to ready 
char readAreg(int DUAdr, int regAdr)
{
  byte registerValue=0;
  Wire.beginTransmission(DUAdr);//get the I2C buss and address a specific display controller
  Wire.write(regAdr); //set the DU command register with the address of requested register  
  Wire.endTransmission(); //get off the buss, now the DU is waiting for the read of the register just addressed
  Wire.requestFrom(DUAdr,1);//request WIRE libary to read a data byte into Wire's buffer from the display controller
  int noOfBytes = Wire.available(); //as a check, read back how many bytes were transferred.
  registerValue= Wire.read(); //read the byte from Wire's buffer, it  will be the value of the register who address was written to command register
  return(registerValue);
}

//Print the read register table header
//no params
void printTableHeader()
{
Serial.println("-------- DU 1&2 Register Values-------");
  Serial.print("DU Adr");
  Serial.print("\t");
  Serial.print("Reg Name");
  Serial.print("\t");
  Serial.print("regAdr");
  Serial.print("\t");
  Serial.print("DEC");
  Serial.print("\t");
  Serial.print("HEX");
  Serial.print("\t");
  Serial.print("Char");
  Serial.print("\t");
  Serial.println("BIN");
}

//Print a row in a table (used for reading register to display)
//DUAdr:the value in the "DU Adr" column
//name: the string in the name column
//registerAdr: address of the control register in regAdr column
//registerValue: the value read from the register that is diplayed in multiple formats
void printTableRow(byte DUAdr, char name[], byte registerAdr, byte registerValue)
{
 Serial.print(DUAdr,HEX);
 Serial.print("\t");
 Serial.print(name);
 Serial.print("\t");
 Serial.print(registerAdr,HEX); //display in HEX
 Serial.print("\t");
 Serial.print(registerValue,DEC);//Displaly in DEC
 Serial.print("\t");
 Serial.print(registerValue,HEX);//Display in HEX
 Serial.print("\t");
 Serial.print(char(registerValue));//recast the value so its ASCI char can be displayed
 Serial.print("\t");
 Serial.println(registerValue,BIN); //Display in BIN 
}


//write a sting of 8 characters to the display, end of string is "\0"
//str[]: array containing string
//plane: plane 0 or 1
void writeString(const char str[], const char plane)
{
char digarr[8] = {' ',' ',' ',' ',' ',' ',' ',' '};//create scroll buffer, set 8 blanks into buffer
Serial.println("Scrolling characters");
while (*str != '\0')	//go until there is a line feed
{
    
        
        for (int i = 0; i <= 7; i++)
	digarr[i] = digarr[i+1];	//shift left
    	digarr[7] = *str; 	//new value in last digit
   	Serial.println (digarr);//Display the scroll
        str++;		//increment source pointer
	write8Digits(&digarr[0],plane); //write scroll buffer to display 
        delay(_scrollspeed);
        }
}

//writes each of the 8 digits with characters from array
void write8Digits(char* str,char plane){
  char theplane = 0;			//define a plane buffer to assemble address
  if (plane == 0) theplane = 0x20;	//plane 0
  else theplane = 0x40;			//plane 1
  Wire.beginTransmission(deviceAdrDU1);	//connect to DU1
  Wire.write(theplane);			//set plane starting address
  Wire.write(*(str+3));			//send data for digit 3, 0x20
  Wire.write(*(str+2));			//send data for digit 2			
  Wire.write(*(str+1));			//send data for digit 1			
  Wire.write(*str);			//send data for digit 0
  Wire.endTransmission();
  Wire.beginTransmission(deviceAdrDU2);	//connect to DU2
  Wire.write(theplane);			//set plane starting address
  Wire.write(*(str+7));			//send data for digit 7
  Wire.write(*(str+6));			//send data for digit 6			
  Wire.write(*(str+5));			//send data for digit 5			
  Wire.write(*(str+4));			//send data for digit 4
  Wire.endTransmission();
}

//set scrolling speed 
//value: scrolling speed in milliseconds,300 is normal
void setScroll(int value)
{
  _scrollspeed = value;
}
//writes the temp config register to the controllers
//To avoid overwriting the configuration register when various single bits are set
//a separate  temp configuration register is kept and single bit set/reset changes are made to that register without overwriting anything else
//after setting/resetting a particular bit, this function should be called to write it
//DO's
//Write to the temp configuration register
//Always call writeConfig()after you change the temp configuration register
//Don't
//Write directly to the DU's configuration registers
//Note:
//a safer way may be to read the actual configuraiton register and then and-or the bits and rewrite it directly to the contoller
//it has the disatvantage of lots of buss traffic to the controller
//The blink functions set the tBit and then call writeConfig which writes to both DU the same configuration value this syncs the blinking of the separate displays
//separating the writing of the tBits may be timing sensitive ...... it works like it is below.
void writeConfig()
{
  //write DU1 config first
  Wire.beginTransmission(deviceAdrDU1); //connect to DU1 
  Wire.write(configRegAdr);		//address the config register
  Wire.write(tempConfigDU1);	//write the config reg values to controller
  Wire.endTransmission();
  tempConfigDU2=tempConfigDU1;//FOR NOW make the DUI and DU2 configuration the same
  //Then write DU2 config
  Wire.beginTransmission(deviceAdrDU2); //connect to DU2 
  Wire.write(configRegAdr);		//address the config register
  Wire.write(tempConfigDU2);	//write the values
 Wire.endTransmission();
 Serial.print("Wrote 0x");
 Serial.print(tempConfigDU1,HEX);
 Serial.print(" into configuration register 0x0");
 Serial.println(configRegAdr,HEX);
 
}
//set controller to blink to slow or fast, blink mode will cause the controller to alternately display plane 0 then 1. 
//The phase can be detected by reading bit 7 of the config register
//ERROR: IN THE BLINK FUNCTION DU1&DU2 ARE OUT OF SYNC
void displayBlink()
{
  tempConfigDU1 |= tBit; //Set t bit to sync blinking (when both DU get the tBit set at the same time they will sync
  tempConfigDU1 &= ~bBit; //reset the B bit setting "slow" blink rate 0x04
  tempConfigDU1 |= eBit; 	//enable global blink 0x08
  writeConfig();	//write config to controller
}

void blinkFast()
{
  tempConfigDU1 |= tBit;// Set t bit to sync blinking, setting the tBit on both displays syncs the blinking
  tempConfigDU1 |= bBit;	//set the B bit enabling fast blink rate
  tempConfigDU1 |= eBit;	//set global blink enable
  writeConfig();	//write config to controller
}

void blinkOff(){
  tempConfigDU1 &= ~eBit;	//reset the E bit disabling global blink
  //And the temp register with the compliment (opposite) (IE: oxF7)of the value of the comparing bit
  //the eBit will be reset and all others will be left alone.
  writeConfig();
}
