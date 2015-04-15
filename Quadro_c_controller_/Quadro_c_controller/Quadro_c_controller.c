/*
 * Quadro_c.c
 *
 * Created: 23.04.2014 17:20:13
 *  Author: Vitaliy
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "RFM70.h"

#define TIMER_1SEC 1992
#define TIMER_500MSEC 996
#define TIMER_100MSEC 199
#define TOGGLE_LED PORTD ^= (1 << 4)

volatile signed char accel_X, accel_Y, accel_Z;
//static FILE mystdout = FDEV_SETUP_STREAM(USART_putc, NULL,_FDEV_SETUP_WRITE);
unsigned int global_timer = 0;



volatile int counter;


int main(void)
{
	
	int buffer [] = {};
	char c [] = "";
	
	DDRB = 0b00000010;
	PORTB = 0b00000000;
	
	DDRD = 0b01111000;
	PORTD = 0b00010000;
	
	
	//stdout = &mystdout;	
	
	_delay_ms(300);	
	USART_init ( MYUBRR );
	
	//printf("Quadro_c board v.1.0.0!\r\n");
	USART_putstring("Quadro_c board v.1.0.0!\r\n");

	
	//LIS331_init();
	
	setBegin(-1, RFM77_DEFAULT_SPI_CLOCK_DIV);

	setModeRX();
	setChannel(8);
	
	USART_putstring("**********Welcome to EWS_RFM70!!!!!**********\n\r\n\r");
	
	TCCR1A = 0b10000001; // Fast PWM 8 Bit, Clear OCA1/OCB1 on Compare Match, Set on TOP
	TCCR1B = 0b00000010; // 1/8
	TCNT1 = 0;  	
    // timer for led "STATUS"
	OCR1B = 127;
	TIMSK1 |= (1 << OCIE1B);     
	
	//cbi(PORTD,4);
	while (1)
	{
		counter = receivePayload(buffer);
		if (counter)
		{
			USART_putc(counter);
			USART_putstring(buffer);
			USART_putc(9);
		}
	}
	
}

ISR(TIMER1_COMPB_vect)
{
	if (++global_timer >= TIMER_500MSEC){
		global_timer = 0;
		/************************************************************************/
		
		/************************************************************************/
	}
}