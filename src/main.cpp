/// Date:
// Assignment:
//
// Description: This file contains a programmatic overall description of the
// program. It should never contain assignments to special function registers
// for the exception key one-line code such as checking the state of the pin.
//
// Requirements:
//----------------------------------------------------------------------//

#include <avr/io.h>
#include <Arduino.h>
//#include "PWM.h"
#include "timer.h" 
//#include "adc.h"
#include "i2c.h"
//#include <Wire.h>
//#include "Adafruit_MCP9808.h"
// Create the MCP9808 temperature sensor object
//Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

int main() {


initI2CMaster();
// Select configuration register  
//Wire.write(0x01);
sendData(0x18, 0x01);  
// Continuous conversion mode, Power-up default  
//Wire.write(0x00);  
//Wire.write(0x00);
sendData(0x18, 0x00);
sendData(0x18, 0x00);  

//Wire.write(0x08);  
sendData(0x18, 0x08);
// Resolution = +0.0625 / C  
//Wire.write(0x03);  
sendData(0x18, 0x03);

Serial.begin(9600);
  while (1) {
   delayMs(1000);
   //ADXL_I2C(0x18, 0x05);
   delayMs(1000);
   //ADXL_I2C(0x18, 0x03);
   
   //Serial.println("hello");
   //sendData(0x18, 0x02);

   //recieveData(0x18, 0x02);
   //delay(1);
   
  
  }


  return 0;
}

