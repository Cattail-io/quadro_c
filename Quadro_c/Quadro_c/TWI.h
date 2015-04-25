/*
 * lis331.h
 *
 * Created: 15.05.2014 17:10:21
 *  Author: Vitaliy
 */ 
#define ACCEL_ADDR 0x38

#define GYRO_ADDR 0xD0

#define TWI_FREQ 100000L

#define TWI_READ_BIT 0x01

#define LIS331_CTRL_REG1 0x20
#define LIS331_CTRL_REG2 0x21
#define LIS331_CTRL_REG3 0x22

#define LIS331_OUT_X 0x29
#define LIS331_OUT_Y 0x2B
#define LIS331_OUT_Z 0x2D

extern void TWI_init();
extern char TWI_action(char command);
extern char TWI_start(void);
extern void TWI_stop(void);
extern char TWI_write_data(char data);
extern char TWI_read_data(char put_ack);
extern char accel_init();
/*
signed char accel_x(void);
signed char accel_y(void);
signed char accel_z(void);
*/
signed char GET_ACCEL_VALUE(char ADDR);
#define accel_getX() GET_ACCEL_VALUE(LIS331_OUT_X)
#define accel_getY() GET_ACCEL_VALUE(LIS331_OUT_Y)
#define accel_getZ() GET_ACCEL_VALUE(LIS331_OUT_Z)

#define GYRO_CTRL_REG1 0x20
#define GYRO_CTRL_REG2 0x21
#define GYRO_CTRL_REG3 0x22
#define GYRO_CTRL_REG4 0x23
#define GYRO_CTRL_REG5 0x24
#define GYRO_WHO_AM_I 0x0F
#define GYRO_STATUS_REG 0x27
#define GYRO_OUT_X 0x28
#define GYRO_OUT_Y 0x2A
#define GYRO_OUT_Z 0x2C

char gyro_init();
char gyro_dataready(void);
/*
extern signed int gyro_x(void);
extern signed int gyro_y(void);
extern signed int gyro_z(void);
*/
int GET_GYRO_VALUE(char ADDR);
#define gyro_getX() GET_GYRO_VALUE(GYRO_OUT_X)
#define gyro_getY() GET_GYRO_VALUE(GYRO_OUT_Y)
#define gyro_getZ() GET_GYRO_VALUE(GYRO_OUT_Z)
//signed int gyro_x(void);
char gyro_who_am_i(void);
