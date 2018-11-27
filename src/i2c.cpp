


#include "i2c.h"
#include "Arduino.h"
#include <avr/io.h>

 

typedef enum i2c_internal_state_enum{
  start, i2c_address,i2c_addressW, i2c_addressR, data0, stop, i2c_reg, repeatStart, NACK
} i2c_internal_state;

volatile unsigned int count = 0;
volatile unsigned char slaveData = 0x00;


void initI2CMaster(){
  //Make the baud rate as low as possible
  TWSR |= (1 << TWPS0) | (1 << TWPS1);
  TWBR = 255;

  // enable TWI
  TWCR |= (1 << TWINT | 1 << TWEN);

}

void recieveData(unsigned char address, unsigned char data) {
  Serial.begin(9600);
 

    i2c_internal_state i2c_state = start;

  unsigned char twi_status;

  while(i2c_state != stop){

    twi_status = TWSR & 0xF8;

    switch(i2c_state){

      case start:

        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

        i2c_state = i2c_address;

        break;

      case i2c_address:

        // start or repeated start

        if(twi_status != 0x10 && twi_status != 0x08){

          i2c_state = start;

        }

        else{
      
          TWDR = (address | 0x01); //0 at end for write

          TWCR = (1 << TWINT) | (1 << TWEN);

          i2c_state = data0;

        }

        break;

      case data0:

        if(twi_status == 0x48){ // SLA_R sent and NOT ACK Received

          i2c_state = start;

        }

        else{
          //Serial.println(TWDR);
          slaveData = TWDR;
          //Serial.println(TWDR);

          TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

          i2c_state = stop;

        }

        break;

      case stop:

        if(twi_status == 0x58){ // Data byte has been received NACK has been returned
          
          Serial.println("0x58: Data byte has been recieved and NOT ACK Received");
          i2c_state = start;

        }

        else{
          Serial.print("Data Byte: "+slaveData);
          TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

        }

        break;

    }

    if(i2c_state != stop){

      while (!(TWCR & (1 << TWINT)));

      if(twi_status == 0x38 || twi_status == 0x00){ // Arbitration loss

        // Serial.println("Arbitration loss or invalid state");

        i2c_state = start;

      }

    }

  }

}


void sendData(unsigned char address, unsigned char data){

  Serial.begin(9600);
 

  i2c_internal_state i2c_state = start;

  unsigned char twi_status;

  while(i2c_state != stop){

    twi_status = TWSR & 0xF8;

    switch(i2c_state){

      case start:

        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

        i2c_state = i2c_address;

        break;

      case i2c_address:

        // start or repeated start

        if(twi_status != 0x10 && twi_status != 0x08){

          i2c_state = start;

        }

        else{
      
          TWDR = (address << 1); //0 at end for write

          TWCR = (1 << TWINT) | (1 << TWEN);

          i2c_state = data0;

        }

        break;

      case data0:

        if(twi_status == 0x20){ // SLA_W sent and NOT ACK Received

          i2c_state = start;

        }

        else{
          //Serial.println(TWDR);
          TWDR = data;
          //Serial.println(TWDR);
          TWCR = (1 << TWINT) | (1 << TWEN);

          i2c_state = stop;

        }

        break;

      case stop:

        if(twi_status == 0x30){ // data sent and NOT ACK Received
          
          Serial.println("0x30: data sent and NOT ACK Received");
          i2c_state = start;

        }

        else{

          TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

        }

        break;

    }

    if(i2c_state != stop){

      while (!(TWCR & (1 << TWINT)));

      if(twi_status == 0x38 || twi_status == 0x00){ // Arbitration loss

        // Serial.println("Arbitration loss or invalid state");

        i2c_state = start;

      }

    }

  }

}
 
/** This code interfaces with the ADXL 345 that was in our sunfounder kit. The data sheet
 * specifies that in order to read data, we first need to start-> SLA+W -> regAddr ->stop/start->
 * SLA+R->Stop
 * 
 * **/
void ADXL_I2C(unsigned char address, unsigned char data){
  i2c_internal_state i2c_state = start;

  unsigned char twi_status;
  
  while(i2c_state != stop){
    twi_status = TWSR & 0xF8;
    //int count = 0;
    switch(i2c_state){
      case start:
        //Serial.println("start");

        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); //send start maybe missing a bit TWIE

        i2c_state = i2c_addressW;

        break;

      //after register address is sent this repeated start should send us to
      //read mode
      case repeatStart:
        //Serial.println("reStart");

        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); //send start
        
        i2c_state = i2c_addressR;

        break;


      case i2c_addressW:
        //Serial.println("add write");
        // start or repeated start

        if(twi_status != 0x10 && twi_status != 0x08){

          i2c_state = start;

        }

        else{
      
          TWDR = (address << 1); //0 at end for write

          TWCR = (1 << TWINT) | (1 << TWEN);

          i2c_state = i2c_reg;

        }

        break;


    case i2c_reg:
        if(twi_status == 0x20){ // SLA_W sent and NOT ACK Received, start over

          Serial.println("0x20: SLA_W sent and NOT ACK Received, start over");
          i2c_state = start; 

        }

        else{
          //Serial.println(TWDR);
          TWDR = data;
          //Serial.println(TWDR);
          TWCR = (1 << TWINT) | (1 << TWEN);

          i2c_state = repeatStart;

        }

        break;


    case i2c_addressR:
        // start or repeated start
        //Serial.println(twi_status);
        if(twi_status != 0x10 && twi_status != 0x08){
          Serial.println("Error here, starting over");
          i2c_state = start;

        }

        else{
      
          TWDR = (address << 1) | 0x01; //1 at end for Read

          TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); //TWIE?
          
          i2c_state = data0;

        }
        break;

      case data0:
        
        if(twi_status == 0x48){ // SLA_R sent and NOT ACK Received
          Serial.println("0x48: SLA_R sent and NOT ACK Received");
          //Serial.println(twi_status);
          i2c_state = repeatStart;

        }

        else{
          //Serial.println(twi_status);
          slaveData = TWDR;
          //Serial.println(TWDR);
          
          TWCR = (1 << TWINT) | (1 << TWEN);//AWK sent   sendStop //TWSTO, noEA  IE?
          //Serial.println(twi_status);
          //Serial.println(slaveData);
          //Serial.println(TWDR);
          i2c_state = NACK;

        }

        break;

      case NACK:
        //Serial.println("NACK");
        //Serial.println(twi_status);
        if(twi_status == 0x58){ // Data byte has been received NACK has been returned
          
          Serial.println("0x58: Data byte has been recieved and NOT ACK Received");
          //i2c_state = stop;
          Serial.println(slaveData);
          Serial.println(TWDR);

        }

        else{
          //Serial.println(slaveData);
          //TWCR = (1 << TWINT) | (1 << TWEN); //stephanie has misgivings about this interrupt
          
          

        }
        Serial.println("stop");
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
        i2c_state = stop;

        break;

      case stop:
        Serial.println("stop");
        Serial.println(twi_status);
        if(twi_status == 0x50){ // Data byte has been received NACK has been returned
          
          Serial.println("0x58: Data byte has been recieved and NOT ACK Received");
          i2c_state = start;

        }

        else{
          Serial.println(slaveData);
          
          TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

        }

        break;
    }

    //Serial.println(i2c_state);

    if(i2c_state != stop){
      //Serial.println(twi_status);
      while (!(TWCR & (1 << TWINT)));

      if(twi_status == 0x38 || twi_status == 0x00){ // Arbitration loss

        Serial.println("Arbitration loss or invalid state");

        i2c_state = start;

      }

    }
  }
}
/*
#include <Arduino.h>
#include "i2c.h"
#include <avr/io.h>

typedef enum i2c_internal_state_enum {
    start, i2c_address, data0, stop
} i2c_internal_state;

void initI2CMaster() {
    //Baud rate as low as possible
    TWSR |= (1 << TWPS0) | (1 << TWPS1);
    TWBR = 255;

    //enable TWI
    TWCR |= (1 << TWINT | 1 << TWEN);
}
void sendData(unsigned char address, unsigned char data) {

    i2c_internal_state i2c_state = start;
    unsigned char twi_status;

    while (i2c_state != stop) {
        twi_status = TWSR & 0xF8;
        switch(i2c_state) {
            case start:
                
                Serial.println("start");
                TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
                i2c_state = i2c_address;
                Serial.println(twi_status);
                break;
            case i2c_address:
                Serial.println("address");
                if (twi_status != 0x10 && twi_status != 0x08) {
                    //Serial.println(twi_status);
                    i2c_state = start;
                    //i2c_state = stop;
                }
                else {
                    Serial.println("else");
                    TWDR = (address << 1);
                    TWDR |= 0b00000001;
                    TWCR = (1 << TWINT) | (TWEN);
                    i2c_state = data0;
                }
                break;
            case data0:
                Serial.println("data0");
                
                TWDR = data;
                TWCR = (1 << TWINT) | (1 << TWEN);
                
                Serial.println("TWDR");
                i2c_state = stop;
                break;
            case stop:
                Serial.println("stop");
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

                break;


            //Serial.println("test");
        
        }
            if (i2c_state != stop) {
                Serial.println("if");
            while(!(TWCR & (1 << TWINT)));
            if (twi_status == 0x38 || twi_status == 0x00) {
                Serial.println("Arbitration loss or invalid state");
                i2c_state = start;

            }
        }
        
    }
           
}*/


