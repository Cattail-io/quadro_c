/*
 * Quadro_c.c
 *
 * Created: 23.04.2014 17:20:13
 *  Author: Vitaliy
 */ 

/*
PD4 - led
PD7 - BAT ready
ADC7 - BAT voltage
PD6/OC0A - motor1
PD5/OC0B - motor2
PB1/OC1A - motor3
PD3/OC2B - motor4

   2
1--+--4
   3

*/

#include <util/delay.h>
#include <avr/interrupt.h>
#include "lis331.h"
#include "usart.h"
#include "rfm70.h"

#define SCALE 0.0181102

#define TIMER_1SEC 1992
#define TIMER_500MSEC 996
#define TIMER_100MSEC 199
#define TOGGLE_LED PORTD ^= (1 << 4)

volatile signed char accel_X, accel_Y, accel_Z;
//static FILE mystdout = FDEV_SETUP_STREAM(USART_putc, NULL,_FDEV_SETUP_WRITE);
unsigned int global_timer = 0;

#include "RFM70.h"

uint16_t value = 0;
uint8_t *pointer = &value;


int main(void)
{

	DDRB = 0b00000010;
	PORTB = 0b00000000;
	
	DDRD = 0b01111000;
	PORTD = 0b00010000;
		
	//stdout = &mystdout;	
	
	_delay_ms(300);	
	USART_init ( MYUBRR );
	
	//printf("Quadro_c board v.1.0.0!\r\n");
	//USART_putstring("Quadro_c board v.1.0.0!\r\n");
	
	//LIS331_init();
	
	sei();
	_delay_ms(500);
	begin();

	setModeTX();
	setChannel(8);
	value = 0xFEBA;
	
	//PD6/OC0A - motor1
	//PD5/OC0B - motor2
	//PB1/OC1A - motor3
	//PD3/OC2B - motor4
	
	// f_PWM = 8000000 / (Prescaler * 510) = 1960 Hz
	TCCR0A = 0b10100001; // Fast PWM 8 Bit, Clear OCA0/OCB0 on Compare Match, Set on TOP
	TCCR0B = 0b00000010; // 1/8 
	TCNT0 = 0;           // Reset TCNT0
	OCR0A = 0;//	OCR0A = 127; // motor1    
	OCR0B = 0;//	OCR0B = 127; // motor2
	
	TCCR1A = 0b10000001; // Fast PWM 8 Bit, Clear OCA1/OCB1 on Compare Match, Set on TOP
	TCCR1B = 0b00000010; // 1/8
	TCNT1 = 0;  
	OCR1A= 0;   //	OCR1A= 127;  // motor3     
		
	TCCR2A = 0b00100001; // Fast PWM 8 Bit, Clear OCA2/OCB2 on Compare Match, Set on TOP
	TCCR2B = 0b00000010; // 1/8 
	TCNT2 = 0;
	OCR2B= 0;//	OCR2B= 127; // motor4
	
    // timer for led "STATUS"
	OCR1B = 127;
	TIMSK1 |= (1 << OCIE1B);     
	sei();
	
	
	while (1)
	{
	//	accel_X = accel_x();
	//	accel_Y = accel_y();
		
		OCR0A = 0;//OCR0A =  127+accel_Y;
		OCR0B = 0;//OCR0B =  127+accel_X;
		OCR1A= 0; //OCR1A = 127-accel_X;
		OCR2B= 0;//OCR2B= 127-accel_Y;
		
		
		
		
		
	
	}
	
}

ISR(TIMER1_COMPB_vect)
{
	if (++global_timer >= TIMER_500MSEC){
		global_timer = 0;
		/************************************************************************/
		TOGGLE_LED;	
		sendPayload(&value,2,0);
		
		/************************************************************************/
	}
}