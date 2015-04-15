/*******************************************************************************
*		  File:		24C16.c
*		 Title:		complete TWI example code for a 24C16 EEPROM
*	also needs:		lcd.h
*	  Compiler:		AVR-GCC 3.3.1
*	   Version:		1.0
*  last update:		30-03-2004
*		Target:		ATmega8
*		Author:		Christoph Redecker
*	   Company:		www.avrbeginners.net
*******************************************************************************/

#include <avr\io.h>
#include <avr\delay.h>
#include <inttypes.h>
#include "lcd.h"

/*******************************************************************************
* TWI_init: sets bitrate in TWBR and prescaler in TWSR
* TWI_action: writes command to TWCR and takes care of TWINT and TWEN setting
*	waits for TWI action to be completed and returns status code
* TWI_start: generates start condition and returns status code
* TWI_stop: generate stop condition and returns status code
* TWI_write_data: writes data to TWI bus and returns status code
* TWI_read_data: reads data from bus to TWDR and returns status code.
*	if put_ack > 0, an ACK will be sent after the data has been received.
* TWI_wait: waits for a given slave address to be ACKed. Only use if slave has
*	ACKed its address before. Loops forever if slave not present!
* EE_read_data: reads one data byte from a given address
*	returns different error codes (zero if success) and data in TWDR
* EE_write_data: write one data byte to a given address
*	returns different error codes (zero if success)
*******************************************************************************/
void TWI_init(char bitrate, char prescaler);
char TWI_action(char command);
char TWI_start(void);
void TWI_stop(void);
char TWI_write_data(char data);
char TWI_read_data(char put_ack);
void TWI_wait(char slave);
char EE_write_byte(const int address, const char data);
char EE_read_byte(const int address);

#define EE_ADDR 0xA0

/******************************************************************************/	
void main (void)
{
	//init LCD
	LCD_init();
	//set TWBR = 32 for 100kHz SCL @ 8MHz
	TWI_init(32, 0);
	
	//write 0x55 @ 513 and print return value on LCD
	LCD_puthex(EE_write_byte(513, 0x55));
	//send stop
	TWI_stop();
	LCD_wait();
	LCD_putchar(' ');
	//wait for the EEPROM to finish the write operation
	TWI_wait(EE_ADDR);
	//read the write location again and print return code on LCD
	LCD_puthex(EE_read_byte(513));
	LCD_wait();
	LCD_putchar(' ');
	//print the value read from the EEPROM on the LCD
	LCD_puthex(TWDR);
	TWI_stop();
	//LCD should now show "0x00 0x00 0x55_"
	//where the _ is the blinking cursor.
}

/******************************************************************************/
void TWI_init(char bitrate, char prescaler)
//sets bitrate and prescaler
{
	TWBR = bitrate;
	//mask off the high prescaler bits (we only need the lower three bits)
	TWSR = prescaler & 0x03;
}

/******************************************************************************/
char TWI_action(char command)
//starts any TWI operation (not stop), waits for completion and returns the status code
//TWINT and TWEN are set by this function, so for a simple data transfer just use TWI_action(0);
{	//make sure command is good
	TWCR = (command|(1<<TWINT)|(1<<TWEN));
	//wait for TWINT to be set after operation has been completed
	while(!(TWCR & (1<<TWINT)));
	//return status code with prescaler bits masked to zero
	return (TWSR & 0xF8);
}

/******************************************************************************/
char TWI_start(void)
//uses TWI_action to generate a start condition, returns status code
{	//TWI_action writes the following command to TWCR: (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)
	return TWI_action(1<<TWSTA);
	//return values should be 0x08 (start) or 0x10 (repeated start)
}

/******************************************************************************/
void TWI_stop(void)
//generates stop condition
{	//as TWINT is not set after a stop condition, we can't use TWI_action here!
	TWCR = ((1<<TWINT)|(1<<TWSTO)|(1<<TWEN));
	//status code returned is 0xF8 (no specific operation)
}

/******************************************************************************/
char TWI_write_data(char data)
//loads data into TWDR and transfers it. Works for slave addresses and normal data
//waits for completion and returns the status code.
{	//just write data to TWDR and transmit it
	TWDR = data;
	//we don't need any special bits in TWCR, just TWINT and TWEN. These are set by TWI_action.
	return TWI_action(0);
	//status code returned should be:
	//0x18 (slave ACKed address)
	//0x20 (no ACK after address)
	//0x28 (data ACKed by slave)
	//0x30 (no ACK after data transfer)
	//0x38 (lost arbitration)
}

/******************************************************************************/
char TWI_read_data(char put_ack)
{	//if an ACK is to returned to the transmitting device, set the TWEA bit
	if(put_ack)
		return(TWI_action(1<<TWEA));
	//if no ACK (a NACK) has to be returned, just receive the data
	else
		return(TWI_action(0));
	//status codes returned:
	//0x38 (lost arbitration)
	//0x40 (slave ACKed address)
	//0x48 (no ACK after slave address)
	//0x50 (AVR ACKed data)
	//0x58 (no ACK after data transfer)
}

/******************************************************************************/
void TWI_wait(char slave)
{
	//send slave address until a slave ACKs it. Good for checking if the EEPROM
	//has finished a write operation. Use carefully! If the wrong slave address
	//is being waited for, this function will end in an infinite loop.
	do {
		TWI_start();
	} while(TWI_write_data(slave) != 0x18);
	TWI_stop();
}

/******************************************************************************/
char EE_write_byte(const int address, const char data)
{
	char dummy;
	//we need this for the first if()
	dummy = TWI_start();
	//if the start was successful, continue, otherwise return 1
	if((dummy != 0x08) && (dummy != 0x10))
		return TWSR;
	//now send the EEPROM slave address together with the address bits 8..10 for page select
	//address format:
	//|bit7	|bit6 |bit5 |bit4 |bit3 |bit2 |bit1 |bit0 |
	//|			EE_ADDR		  |  page select	| R/W |
	if(TWI_write_data(EE_ADDR|((address>>7) & 0x000E)) != 0x18)
		return TWSR;
	//now send the word address byte
	if(TWI_write_data((char)(address)) != 0x28)
		return TWSR;
	//now send the data byte
	if(TWI_write_data(data) != 0x28)
		return TWSR;
	//if everything was OK, return zero.
	return 0;
}

/******************************************************************************/
char EE_read_byte(const int address)
{
	char dummy;
	//we need this for the first if()
	dummy = TWI_start();
	//as in EE_write_byte, first send the page address and the word address
	if((dummy != 0x08) && (dummy != 0x10))
		return TWSR;
	if(TWI_write_data(EE_ADDR|((address>>7) & 0x000E)) != 0x18)
		return TWSR;
	if(TWI_write_data((char)(address)) != 0x28)
		return TWSR;
	//send a repeated start for entering master receiver mode
	if(TWI_start() != 0x10)
		return TWSR;
	//send slave address, now with the read bit set
	if(TWI_write_data((EE_ADDR|1)|((address>>7) & 0x000E)) != 0x40)
		return TWSR;
	//now get the data from the EEPROM, don't return ACK
	if(TWI_read_data(0) != 0x58)
		return TWSR;
	//if everything was OK, return zero
	return 0;
}