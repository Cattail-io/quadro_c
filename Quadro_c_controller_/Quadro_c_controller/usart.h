/*
 * usart.h
 *
 * Created: 15.05.2014 17:21:59
 *  Author: Vitaliy
 */ 
#define BAUD 38400
#define MYUBRR F_CPU/16/BAUD-1

extern void USART_init( unsigned int ubrr);
extern void USART_putc( unsigned char data );
extern void USART_putstring(const char* str);