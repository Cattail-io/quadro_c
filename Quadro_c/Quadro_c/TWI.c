/*
 * lis331.c
 *
 * Created: 15.05.2014 17:08:43
 *  Author: Vitaliy
 */ 
#include <avr/io.h>
#include "TWI.h"

signed int GET_GYRO_VALUE(char ADDR);

void TWI_init()
{
	TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
	TWSR = 0;
}

//starts any TWI operation (not stop), waits for completion and returns the status code
//TWINT and TWEN are set by this function, so for a simple data transfer just use TWI_action(0);
char TWI_action(char command)
{	
	TWCR = (command|(1<<TWINT)|(1<<TWEN));
	while(!(TWCR & (1<<TWINT)));
	return (TWSR & 0xF8);
}


//uses TWI_action to generate a start condition, returns status code
char TWI_start(void)
{	//TWI_action writes the following command to TWCR: (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)
	return TWI_action(1<<TWSTA);
	//return values should be 0x08 (start) or 0x10 (repeated start)
}


void TWI_stop(void)
{	//as TWINT is not set after a stop condition, we can't use TWI_action here!
	TWCR = ((1<<TWINT)|(1<<TWSTO)|(1<<TWEN));
	//status code returned is 0xF8 (no specific operation)
}


char TWI_write_data(char data)
{	
	TWDR = data;
	return TWI_action(0);
	//status code returned should be:
	//0x18 (slave ACKed address)
	//0x20 (no ACK after address)
	//0x28 (data ACKed by slave)
	//0x30 (no ACK after data transfer)
	//0x38 (lost arbitration)
}


char TWI_read_data(char put_ack)
{	
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

char TWI_writeToReg(const char SlaveAddr, const char RegAddr, const char DataByte)
{
	char res;
	TWI_init();
	
	res = TWI_start();
	if((res != 0x08) && (res != 0x10))
		return TWSR;
	if(TWI_write_data(SlaveAddr) != 0x18)
		return TWSR;
	if(TWI_write_data(RegAddr) != 0x28)
		return TWSR;
	if(TWI_write_data(DataByte) != 0x28) 
		return TWSR;
	
	TWI_stop();
	
	return 0;//if ok
}

char TWI_readReg(const char SlaveAddr, const char RegAddr)
{
	signed char res;
	TWI_start();
	TWI_write_data(SlaveAddr);
	TWI_write_data(0x80|RegAddr);
	TWI_start();
	TWI_write_data(SlaveAddr|TWI_READ_BIT);
	TWI_read_data(0);
	res = TWDR;
	TWI_stop();
	
	return res;
}

char accel_init()
{
	if (TWI_writeToReg(ACCEL_ADDR, LIS331_CTRL_REG1, 0b01000111) != 0)
		return (-1);
	//if (TWI_writeToReg(ACCEL_ADDR, LIS331_CTRL_REG2, 0b00010000) != 0)
	//	return (-1);
	
	return 0;
}

signed char GET_ACCEL_VALUE(char ADDR)
{
	signed char accel;
	accel = TWI_readReg(ACCEL_ADDR, ADDR);
	accel = ~accel + 1;
	return accel;
}


char gyro_init()
{
	if (TWI_writeToReg(GYRO_ADDR, GYRO_CTRL_REG1, 0x0F) != 0)
		return (-1);
	if (TWI_writeToReg(GYRO_ADDR, GYRO_CTRL_REG4, 0xB0) != 0)
		return (-1);
	if (TWI_writeToReg(GYRO_ADDR, GYRO_CTRL_REG5, 0x80) != 0)
		return (-1);
	return 0;
}

char gyro_dataready(void)
{
	unsigned char ready = TWI_readReg(GYRO_ADDR, GYRO_STATUS_REG) & 0x88; 
	if (ready == 0x08)
		return (1);
	else
		return (-1);
}

int GET_GYRO_VALUE(char ADDR)
{
	TWI_start();
	TWI_write_data(GYRO_ADDR);
	TWI_write_data(0x80|ADDR);
	TWI_start();
	TWI_write_data(GYRO_ADDR|TWI_READ_BIT);
	TWI_read_data(1);
	int gyro_x = (int)TWDR;
	TWI_read_data(0);
	gyro_x = gyro_x|TWDR<<8;
	gyro_x = ~gyro_x + 1;
	TWI_stop();
	return gyro_x;
}

char gyro_who_am_i(void)
{
	return TWI_readReg(GYRO_ADDR, GYRO_WHO_AM_I);
}