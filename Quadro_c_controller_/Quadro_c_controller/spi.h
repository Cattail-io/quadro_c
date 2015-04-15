/*
 * spi.h
 *
 * Created: 01.07.2013 14:00:55
 *  Author: d-wsky
 */ 


#ifndef SPI_H_
#define SPI_H_

//#include <inttypes.h>

#define DD_MOSI           PB3
#define DD_SCK            PB5

#define SPI_DORD_LSB_TO_MSB   SPCR |= (1 << DORD)
#define SPI_DORD_MSB_TO_LSB   SPCR &=~(1 << DORD)

extern void spi_init();
extern unsigned char spi_read(unsigned char value);

#endif /* SPI_H_ */