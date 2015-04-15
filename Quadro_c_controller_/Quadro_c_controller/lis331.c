/*
 * lis331.c
 *
 * Created: 15.05.2014 17:08:43
 *  Author: Vitaliy
 */ 
#include <avr/io.h>
#include "lis331.h"


void TWI_init()
//sets bitrate and prescaler
{
	TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
	//mask off the high prescaler bits (we only need the lower three bits)
	TWSR = 0;
}

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

char TWI_start(void)
//uses TWI_action to generate a start condition, returns status code
{	//TWI_action writes the following command to TWCR: (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)
	return TWI_action(1<<TWSTA);
	//return values should be 0x08 (start) or 0x10 (repeated start)
}


void TWI_stop(void)
//generates stop condition
{	//as TWINT is not set after a stop condition, we can't use TWI_action here!
	TWCR = ((1<<TWINT)|(1<<TWSTO)|(1<<TWEN));
	//status code returned is 0xF8 (no specific operation)
}


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

/*
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
*/



char LIS331_init()
{
	char dummy;

	TWI_init();
	
	dummy = TWI_start();
	//if the start was successful, continue, otherwise return 1
	if((dummy != 0x08) && (dummy != 0x10))
	return TWSR;
	//now send the EEPROM slave address together with the address bits 8..10 for page select
	if(TWI_write_data(0x38) != 0x18)
	return TWSR;
	
	if(TWI_write_data(0x20) != 0x28)
	return TWSR;
	if(TWI_write_data(0x47) != 0x28)
	return TWSR;
	TWI_stop();

	//if everything was OK, return zero.
	return 0;
}


signed char accel_x(void)
{
	signed char accel_x;
	TWI_start();
	TWI_write_data(0x38);
	TWI_write_data(0x80|0x29);
	TWI_start();
	TWI_write_data(0x39);
	TWI_read_data(0);
	accel_x = TWDR;
	accel_x = ~accel_x + 1;
	TWI_stop();
	return accel_x;
	
	//printf("x=%d, y=%d, z= %d\r\n", accel_x, accel_y, accel_z);
}

signed char accel_z(void)
{
	signed char accel_z;
	TWI_start();
	TWI_write_data(0x38);
	TWI_write_data(0x80|0x2D);
	TWI_start();
	TWI_write_data(0x39);
	TWI_read_data(0);
	accel_z = TWDR;
	accel_z = ~accel_z + 1;
	TWI_stop();
	return accel_z;
}

signed char accel_y(void)
{
	signed char accel_y;
	TWI_start();
	TWI_write_data(0x38);
	TWI_write_data(0x80|0x2B);
	TWI_start();
	TWI_write_data(0x39);
	TWI_read_data(0);
	accel_y = TWDR;
	accel_y = ~accel_y + 1;
	TWI_stop();
	
	return accel_y;
}