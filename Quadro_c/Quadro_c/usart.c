/*
 * usart.c
 *
 * Created: 15.05.2014 17:21:09
 *  Author: Vitaliy
 */ 
#include <avr/io.h>
#include "usart.h"


void USART_init( unsigned int ubrr)
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
}

void USART_putc( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

void USART_putstring(const char* str) {
	unsigned char c;
	while ((c=*str++))
	{
		USART_putc(c);
	}
}