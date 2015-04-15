//=============RFM70_init.c==========================

#include "RFM70.h"

//Bank1 register initialization value

//In the array RegArrFSKAnalog,all the register value is the byte reversed!!!!!!!!!!!!!!!!!!!!!
const unsigned long Bank1_Reg0_13[]={       //latest config txt
	0xE2014B40,
	0x00004BC0,
	0x028CFCD0,
	0x41390099,
	0x1B8296d9,
	0xA67F0624,//0xA67F0224,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00731200,//0x00127300
	0x0080B446,//0x46B48000
};

const uint8_t Bank1_Reg14[]=
{//0xFF EF 7D F2 08 08 20 82 04 10 41
	0x41,0x20,0x08,0x04,0x81,0x20,0xCF,0xF7,0xFE,0xFF,0xFF
};

//Bank0 register initialization value
const uint8_t Bank0_Reg[][2]={
	{0,0x0F},//reflect RX_DR\TX_DS\MAX_RT,Enable CRC ,2byte,POWER UP,PRX
	{1,0x3F},//Enable auto acknowledgement data pipe5\4\3\2\1\0
	{2,0x3F},//Enable RX Addresses pipe5\4\3\2\1\0
	{3,0x03},//RX/TX address field width 5byte
	{4,0x08},//auto retransmission dalay (4000us),auto retransmission count(15)
	{5,0x17},//23 channel
	{6,0x0F},//air data rate-1M,out power 0dbm,setup LNA gain \bit4 must set up to 0
	{7,0x07},//
	{8,0x00},//
	{9,0x00},
	{12,0xc3},//only LSB Receive address data pipe 2, MSB bytes is equal to RX_ADDR_P1[39:8]
	{13,0xc4},//only LSB Receive address data pipe 3, MSB bytes is equal to RX_ADDR_P1[39:8]
	{14,0xc5},//only LSB Receive address data pipe 4, MSB bytes is equal to RX_ADDR_P1[39:8]
	{15,0xc6},//only LSB Receive address data pipe 5, MSB bytes is equal to RX_ADDR_P1[39:8]
	{17,0x20},//Number of bytes in RX payload in data pipe0(32 byte)
	{18,0x20},//Number of bytes in RX payload in data pipe1(32 byte)
	{19,0x20},//Number of bytes in RX payload in data pipe2(32 byte)
	{20,0x20},//Number of bytes in RX payload in data pipe3(32 byte)
	{21,0x20},//Number of bytes in RX payload in data pipe4(32 byte)
	{22,0x20},//Number of bytes in RX payload in data pipe5(32 byte)
	{23,0x20},//fifo status
	{28,0x3F},//Enable dynamic payload length data pipe5\4\3\2\1\0
	{29,0x07}//Enables Dynamic Payload Length,Enables Payload with ACK,Enables the W_TX_PAYLOAD_NOACK command
};

const uint8_t RX0_Address[]={0x34,0x43,0x10,0x10,0x01};//Receive address data pipe 0
const uint8_t RX1_Address[]={0x35,0x43,0x10,0x10,0x02};////Receive address data pipe 1

///////////////////////////////////////////////////////////////////////////////
//                  SPI access                                               //
///////////////////////////////////////////////////////////////////////////////

/**************************************************         
Function: SPI_RW();                                         
                                                            
Description:                                                
	Writes one uint8_t to RFM70, and return the uint8_t read 
 **************************************************/

/*uint8_t SPI_RW(uint8_t value)                                    
{                                                           
	uint8_t bit_ctr;
	for(bit_ctr=0;bit_ctr<8;bit_ctr++)   // output 8-bit
	{
		if(value & 0x80)
		{
			MOSI=1;
		}
		else
		{
			MOSI=0;		
		}

		value = (value << 1);           // shift next bit into MSB..
		SCK = 1;						// Set SCK high..
		value |= MISO;       		  // capture current MISO bit
		SCK = 0;            		  // ..then set SCK low again
	}
	return(value);           		  // return read uint8_t
}*/

uint8_t SPI_RW(uint8_t value)                                    
{  
	SPDR = value;					// send data via SPI - put in SPI Data Register
	while((SPSR&0x80) == 0x00);	// wait for SPI interrupt flag indicating transmit complete
	return SPDR;					// return value in SPI Data Register
}

                                                              
/**************************************************         
Function: SPI_Write_Reg();                                  
                                                            
Description:                                                
	Writes value 'value' to register 'reg'              
 **************************************************/        
void SPI_Write_Reg(uint8_t reg, uint8_t value)                 
{
	cbi(PORTB,CSN);     // CSN low, init SPI transaction
	_delay_us(10);
	SPI_RW(reg);     	// select register
	SPI_RW(value);      // ..and write value to it..
	_delay_us(10);
	sbi(PORTB,CSN);     // CSN high again
	_delay_us(10);
}                                                         
/**************************************************/        
                                                            
/**************************************************         
Function: SPI_Read_Reg();                                   
                                                            
Description:                                                
	Read one uint8_t from BK2421 register, 'reg'           
 **************************************************/        
uint8_t SPI_Read_Reg(uint8_t reg)                               
{                                                           
	uint8_t value;
	cbi(PORTB,CSN);      	// CSN low, initialize SPI communication...
	_delay_us(10);
	SPI_RW(reg);            // Select register to read from..
	value = SPI_RW(0);    	// ..then read register value
	_delay_us(10);
	sbi(PORTB,CSN);         // CSN high, terminate SPI communication
_delay_us(10);
	return(value);        	// return register value
}                                                           
/**************************************************/        
                                                            
/**************************************************         
Function: SPI_Read_Buf();                                   
                                                            
Description:                                                
	Reads 'length' #of length from register 'reg'         
 **************************************************/        
void SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length)     
{                                                           
	uint8_t status,byte_ctr;                              
                                                            
	cbi(PORTB,CSN);                    	// Set CSN l
	_delay_us(10);
	status = SPI_RW(reg);       		// Select register to write, and read status uint8_t
                                                            
	for(byte_ctr=0;byte_ctr<length;byte_ctr++)           
		pBuf[byte_ctr] = SPI_RW(0);    	// Perform SPI_RW to read uint8_t from RFM70 
    _delay_us(10);                                                        
	sbi(PORTB,CSN);                  	// Set CSN high again
     _delay_us(10);          
}                                                           
/**************************************************/        
                                                            
/**************************************************         
Function: SPI_Write_Buf();                                  
                                                            
Description:                                                
	Writes contents of buffer '*pBuf' to RFM70         
 **************************************************/        
void SPI_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t length)    
{                                                           
	uint8_t byte_ctr;                              
                                                            
	cbi(PORTB,CSN);           	// Set CSN low, init SPI tranaction
	_delay_us(10);
	SPI_RW(reg);    			// Select register to write to and read status uint8_t
	for(byte_ctr=0; byte_ctr<length; byte_ctr++) // then write all uint8_t in buffer(*pBuf) 
		SPI_RW(*pBuf++);                            
	_delay_us(10);        
	sbi(PORTB,CSN);       		// Set CSN high again      
	_delay_us(10);

}
/**************************************************
Function: SwitchToRxMode();
Description:
	switch to Rx mode
 **************************************************/
void SwitchToRxMode()
{
	uint8_t value;

	SPI_Write_Reg(FLUSH_RX,0);//flush Rx

	value=SPI_Read_Reg(STATUS);	// read register STATUS's value
	SPI_Write_Reg(WRITE_REG|STATUS,value);// clear RX_DR or TX_DS or MAX_RT interrupt flag

	cbi(PORTB,CE);
_delay_us(10);
	value=SPI_Read_Reg(CONFIG);	// read register CONFIG's value
	
//PRX
	value=value|0x01;//set bit 1
  	SPI_Write_Reg(WRITE_REG | CONFIG, value); // Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled..
	  _delay_us(10);
	sbi(PORTB,CE);
	_delay_us(10);
}

/**************************************************
Function: SwitchToTxMode();
Description:
	switch to Tx mode
 **************************************************/
void SwitchToTxMode()
{
	uint8_t value;
	SPI_Write_Reg(FLUSH_TX,0);//flush Tx
_delay_us(10);
	cbi(PORTB,CE);
	_delay_us(10);
	value=SPI_Read_Reg(CONFIG);	// read register CONFIG's value
//PTX
	value=value&0xfe;//set bit 0
  	SPI_Write_Reg(WRITE_REG | CONFIG, value); // Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled.
	_delay_us(10);
	sbi(PORTB,CE);
	_delay_us(10);
}

/**************************************************
Function: SwitchCFG();
                                                            
Description:
	 access switch between Bank1 and Bank0 

Parameter:
	_cfg      1:register bank1
	          0:register bank0
Return:
     None
 **************************************************/
void SwitchCFG(char _cfg)//1:Bank1 0:Bank0
{
	uint8_t Tmp;

	Tmp=SPI_Read_Reg(7);
	Tmp=Tmp&0x80;

	if( ( (Tmp)&&(_cfg==0) )
	||( ((Tmp)==0)&&(_cfg) ) )
	{
		SPI_Write_Reg(ACTIVATE_CMD,0x53);
	}
}

/**************************************************
Function: SetChannelNum();
Description:
	set channel number

 **************************************************/
void SetChannelNum(uint8_t ch)
{
	SPI_Write_Reg((uint8_t)(WRITE_REG|5),(uint8_t)(ch));
}



///////////////////////////////////////////////////////////////////////////////
//                  RFM70 initialization                                    //
///////////////////////////////////////////////////////////////////////////////
/**************************************************         
Function: RFM70_Initialize();                                  

Description:                                                
	register initialization
 **************************************************/   
void RFM70_Initialize()
{
	uint8_t i,j;
 	uint8_t WriteArr[12];

	_delay_ms(200);
	
	SwitchCFG(0);

	for(i=0;i<20;i++)
	{
		SPI_Write_Reg((WRITE_REG|Bank0_Reg[i][0]),Bank0_Reg[i][1]);
	}
	
/*//reg 10 - Rx0 addr
	SPI_Write_Buf((WRITE_REG|10),RX0_Address,5);
	
//REG 11 - Rx1 addr
	SPI_Write_Buf((WRITE_REG|11),RX1_Address,5);

//REG 16 - TX addr
	SPI_Write_Buf((WRITE_REG|16),RX0_Address,5);*/

//reg 10 - Rx0 addr
	for(j=0;j<5;j++)
	{
		WriteArr[j]=RX0_Address[j];
	}
	SPI_Write_Buf((WRITE_REG|10),&(WriteArr[0]),5);
	
//REG 11 - Rx1 addr
	for(j=0;j<5;j++)
	{
		WriteArr[j]=RX1_Address[j];
	}
	SPI_Write_Buf((WRITE_REG|11),&(WriteArr[0]),5);
//REG 16 - TX addr
	for(j=0;j<5;j++)
	{
		WriteArr[j]=RX0_Address[j];
	}
	SPI_Write_Buf((WRITE_REG|16),&(WriteArr[0]),5);
	
//	printf("\nEnd Load Reg");

	i=SPI_Read_Reg(29);//read Feature Register 如果要支持动态长度或者 Payload With ACK，需要先给芯片发送 ACTIVATE命令（数据为0x73),然后使能动态长度或者 Payload With ACK (REG28,REG29).
	if(i==0) // i!=0 showed that chip has been actived.so do not active again.
		SPI_Write_Reg(ACTIVATE_CMD,0x73);// Active
	for(i=22;i>=21;i--)
	{
		SPI_Write_Reg((WRITE_REG|Bank0_Reg[i][0]),Bank0_Reg[i][1]);
		//SPI_Write_Reg_Bank0(Bank0_Reg[i][0],Bank0_Reg[i][1]);
	}
	
//********************Write Bank1 register******************
	SwitchCFG(1);
	
	for(i=0;i<=8;i++)//reverse
	{
		for(j=0;j<4;j++)
			WriteArr[j]=(Bank1_Reg0_13[i]>>(8*(j) ) )&0xff;

		SPI_Write_Buf((WRITE_REG|i),&(WriteArr[0]),4);
	}

	for(i=9;i<=13;i++)
	{
		for(j=0;j<4;j++)
			WriteArr[j]=(Bank1_Reg0_13[i]>>(8*(3-j) ) )&0xff;

		SPI_Write_Buf((WRITE_REG|i),&(WriteArr[0]),4);
	}

	//SPI_Write_Buf((WRITE_REG|14),&(Bank1_Reg14[0]),11);
	for(j=0;j<11;j++)
	{
		WriteArr[j]=Bank1_Reg14[j];
	}
	SPI_Write_Buf((WRITE_REG|14),&(WriteArr[0]),11);

//toggle REG4<25,26>
	for(j=0;j<4;j++)
		//WriteArr[j]=(RegArrFSKAnalog[4]>>(8*(j) ) )&0xff;
		WriteArr[j]=(Bank1_Reg0_13[4]>>(8*(j) ) )&0xff;

	WriteArr[0]=WriteArr[0]|0x06;
	SPI_Write_Buf((WRITE_REG|4),&(WriteArr[0]),4);

	WriteArr[0]=WriteArr[0]&0xf9;
	SPI_Write_Buf((WRITE_REG|4),&(WriteArr[0]),4);

	//**************************Test spi*****************************//
	//SPI_Write_Reg((WRITE_REG|Bank0_Reg[2][0]),0x0f);
	//test_data = SPI_Read_Reg(0x02);  	// must define "test_data" before using here
	
	_delay_ms(50);
	
//********************switch back to Bank0 register access******************
	SwitchCFG(0);
	SwitchToRxMode();//switch to RX mode
}
