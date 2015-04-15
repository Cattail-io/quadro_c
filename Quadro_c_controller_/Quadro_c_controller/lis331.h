/*
 * lis331.h
 *
 * Created: 15.05.2014 17:10:21
 *  Author: Vitaliy
 */ 
#define LIS331_ADDR 0x1C
#define TWI_FREQ 100000L

extern void TWI_init();
extern char TWI_action(char command);
extern char TWI_start(void);
extern void TWI_stop(void);
extern char TWI_write_data(char data);
extern char TWI_read_data(char put_ack);
extern char LIS331_init();
signed char accel_x(void);
signed char accel_y(void);
signed char accel_z(void);