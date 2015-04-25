/*
 * Quadro_c.c
 *
 * Created: 23.04.2014 17:20:13
 * Author: Vitaliy
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
#include <stdio.h>

#include "TWI.h"
#include "usart.h"
#include "rfm70.h"

#define DEBUG_INFO
//#undef DEBUG_INFO

#define SCALE 0.0181102

#define TIMER_1SEC 1992
#define TIMER_500MSEC 996
#define TIMER_100MSEC 199

#define TOGGLE_LED PORTD ^= (1 << 4)

unsigned int global_timer = 0;
unsigned char isTimer = 0;

FILE uart_stdout = FDEV_SETUP_STREAM(USART_putc, NULL,_FDEV_SETUP_WRITE);
/*
uint16_t value = 0;
uint8_t *pointer = &value;
*/
#ifdef DEBUG_INFO
	#warning ">>> DEBUG_INFO enabled!";
#endif

int main(void)
{
	signed int gyro_X, gyro_Y, gyro_Z;
	volatile signed char accel_X, accel_Y, accel_Z;
	//double dX, dY, dZ;
	double g_X = 0, g_Y = 0, g_Z = 0;
	
	
	DDRB  = 0b00000010;
	PORTB = 0b00000000;
	
	DDRD  = 0b01111000;
	PORTD = 0b00010000;
		
	stdout = &uart_stdout;	
	
	_delay_ms(300);	
	USART_init ( MYUBRR );
	
	#ifdef DEBUG_INFO
		printf("Quadro_c board v.1.0.0!\n");
	#endif
	
	if (accel_init() == 0){
		printf("Accel init ok!\n");
	}	
	if (gyro_init() == 0){
		printf("Gyro init ok!\n");
	}	
	sei();
	
	_delay_ms(500);
	
	//begin();
	//setModeTX();
	//setChannel(8);
	//value = 0xFEBA;
	
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
	
	//printf("Who_am_i gyro  = %#X \n",(char)gyro_who_am_i());
	//printf("Calibralion..\n");
	for(int i=0; i<1000; i++)
	{
		while (!gyro_dataready());
		g_X += gyro_getX();
		g_Y += gyro_getY();
		g_Z += gyro_getZ();
	}
	g_X = g_X/1000;
	g_Y = g_Y/1000;
	g_Z = g_Z/1000;
	//printf("Mean gyro x=%8.3f, y=%8.3f, z=%8.3f\n", g_X, g_Y, g_Z);	
	
	while (1)
	{
		OCR0A = 0;//OCR0A =  127+accel_Y;
		OCR0B = 0;//OCR0B =  127+accel_X;
		OCR1A= 0; //OCR1A = 127-accel_X;
		OCR2B= 0; //OCR2B = 127-accel_Y;

		if (isTimer == 1){
			//sendPayload(&value,2,0);
			TOGGLE_LED;
			#ifdef DEBUG_INFO
			accel_X = accel_getX();//accel_x();
			accel_Y = accel_getY();//accel_y();
			accel_Z = accel_getZ();//accel_z();
			printf("%d, %d, %d, ", accel_X, accel_Y, accel_Z);
			//printf("Accel x=%d, y=%d, z= %d   ", accel_X, accel_Y, accel_Z);
			//double fXg, fYg, fZg;
			while (!gyro_dataready());
		
			gyro_X = gyro_getX() - g_X ;
			gyro_Y = gyro_getY() - g_Y ;
			gyro_Z = gyro_getZ() - g_Z ;
			printf("%d, %d, %d\n", gyro_X, gyro_Y, gyro_Z);				
			#endif
			isTimer = 0;
		}	
	}	
}

ISR(TIMER1_COMPB_vect)
{
	if (++global_timer >= TIMER_1SEC){
		global_timer = 0;
		/************************************************************************/
		isTimer = 1;
		/************************************************************************/
	}
}