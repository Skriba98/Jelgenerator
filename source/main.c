/*
 * 
 *
 * Created: 2020. 09. 09. 16:12:35
 *  Author: Skriba
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#define LCD_BACKLIGHT 0x08
#define LCD_EN 0x04
#define LCD_RW 0x02
#define LCD_RS 0x01

volatile uint8_t state_machine = 0;
#define UPD_LCD 0x01

volatile uint8_t twi_status = 0;
#define TWI_START 0x01
#define SEND_ADDRESS 0x02
#define ACK_OK 0x04
#define SEND_DATA 0x08
#define STOP_COND 0x10

volatile uint8_t twi_data[2];
const uint8_t address = 0x4E;

volatile uint16_t potData = 0;
volatile int poti = 0;

//Values

volatile int setFreq[4] = {0,0,0,0};
int setOut = 1;
int setOutDiff = 0;
volatile uint16_t setOff[4] = {0,0,0,0};
int isOffPos = 1;
volatile uint16_t offValue = 0;	
	
volatile uint16_t setAmp[4] = {0,0,0,0};

//Helyiertek
volatile int valueDec = 0;
int menuNumber = 1;

/***Sine frequency setup***/
volatile uint32_t divided = 8000000;
volatile uint32_t period = 256;
volatile uint16_t newOCRA = 1;
volatile int sinemode = 1;
/***8-bit Sine lookup table***/
int type = 0;

uint8_t sine[4][256] = {{
	0x80,0x83,0x86,0x89,0x8c,0x8f,0x92,0x95,
	0x98,0x9b,0x9e,0xa2,0xa5,0xa7,0xaa,0xad,
	0xb0,0xb3,0xb6,0xb9,0xbc,0xbe,0xc1,0xc4,
	0xc6,0xc9,0xcb,0xce,0xd0,0xd3,0xd5,0xd7,
	0xda,0xdc,0xde,0xe0,0xe2,0xe4,0xe6,0xe8,
	0xea,0xeb,0xed,0xee,0xf0,0xf1,0xf3,0xf4,
	0xf5,0xf6,0xf8,0xf9,0xfa,0xfa,0xfb,0xfc,
	0xfd,0xfd,0xfe,0xfe,0xfe,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xfe,0xfe,0xfe,0xfd,
	0xfd,0xfc,0xfb,0xfa,0xfa,0xf9,0xf8,0xf6,
	0xf5,0xf4,0xf3,0xf1,0xf0,0xee,0xed,0xeb,
	0xea,0xe8,0xe6,0xe4,0xe2,0xe0,0xde,0xdc,
	0xda,0xd7,0xd5,0xd3,0xd0,0xce,0xcb,0xc9,
	0xc6,0xc4,0xc1,0xbe,0xbc,0xb9,0xb6,0xb3,
	0xb0,0xad,0xaa,0xa7,0xa5,0xa2,0x9e,0x9b,
	0x98,0x95,0x92,0x8f,0x8c,0x89,0x86,0x83,
	0x80,0x7c,0x79,0x76,0x73,0x70,0x6d,0x6a,
	0x67,0x64,0x61,0x5d,0x5a,0x58,0x55,0x52,
	0x4f,0x4c,0x49,0x46,0x43,0x41,0x3e,0x3b,
	0x39,0x36,0x34,0x31,0x2f,0x2c,0x2a,0x28,
	0x25,0x23,0x21,0x1f,0x1d,0x1b,0x19,0x17,
	0x15,0x14,0x12,0x11,0x0f,0x0e,0x0c,0x0b,
	0x0a,0x09,0x07,0x06,0x05,0x05,0x04,0x03,
	0x02,0x02,0x01,0x01,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x02,
	0x02,0x03,0x04,0x05,0x05,0x06,0x07,0x09,
	0x0a,0x0b,0x0c,0x0e,0x0f,0x11,0x12,0x14,
	0x15,0x17,0x19,0x1b,0x1d,0x1f,0x21,0x23,
	0x25,0x28,0x2a,0x2c,0x2f,0x31,0x34,0x36,
	0x39,0x3b,0x3e,0x41,0x43,0x46,0x49,0x4c,
	0x4f,0x52,0x55,0x58,0x5a,0x5d,0x61,0x64,
	0x67,0x6a,0x6d,0x70,0x73,0x76,0x79,0x7c
},{
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
},{
	0x2,0x4,0x6,0x8,0xa,0xc,0xe,0x10,
	0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,0x20,
	0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,
	0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,0x40,
	0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,
	0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,0x60,
	0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,
	0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,0x80,
	0x81,0x83,0x85,0x87,0x89,0x8b,0x8d,0x8f,
	0x91,0x93,0x95,0x97,0x99,0x9b,0x9d,0x9f,
	0xa1,0xa3,0xa5,0xa7,0xa9,0xab,0xad,0xaf,
	0xb1,0xb3,0xb5,0xb7,0xb9,0xbb,0xbd,0xbf,
	0xc1,0xc3,0xc5,0xc7,0xc9,0xcb,0xcd,0xcf,
	0xd1,0xd3,0xd5,0xd7,0xd9,0xdb,0xdd,0xdf,
	0xe1,0xe3,0xe5,0xe7,0xe9,0xeb,0xed,0xef,
	0xf1,0xf3,0xf5,0xf7,0xf9,0xfb,0xfd,0xff,
	0xfd,0xfb,0xf9,0xf7,0xf5,0xf3,0xf1,0xef,
	0xed,0xeb,0xe9,0xe7,0xe5,0xe3,0xe1,0xdf,
	0xdd,0xdb,0xd9,0xd7,0xd5,0xd3,0xd1,0xcf,
	0xcd,0xcb,0xc9,0xc7,0xc5,0xc3,0xc1,0xbf,
	0xbd,0xbb,0xb9,0xb7,0xb5,0xb3,0xb1,0xaf,
	0xad,0xab,0xa9,0xa7,0xa5,0xa3,0xa1,0x9f,
	0x9d,0x9b,0x99,0x97,0x95,0x93,0x91,0x8f,
	0x8d,0x8b,0x89,0x87,0x85,0x83,0x81,0x80,
	0x7e,0x7c,0x7a,0x78,0x76,0x74,0x72,0x70,
	0x6e,0x6c,0x6a,0x68,0x66,0x64,0x62,0x60,
	0x5e,0x5c,0x5a,0x58,0x56,0x54,0x52,0x50,
	0x4e,0x4c,0x4a,0x48,0x46,0x44,0x42,0x40,
	0x3e,0x3c,0x3a,0x38,0x36,0x34,0x32,0x30,
	0x2e,0x2c,0x2a,0x28,0x26,0x24,0x22,0x20,
	0x1e,0x1c,0x1a,0x18,0x16,0x14,0x12,0x10,
	0xe,0xc,0xa,0x8,0x6,0x4,0x2,0x0
}, {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
	0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
}
	};



/*
uint8_t sine[256] = {
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

*/
int factor = 1;
/*uint8_t sine[50] = {0x80,0x8f,0x9f,0xae,0xbd,0xca,0xd7,0xe2,
0xeb,0xf3,0xf9,0xfd,0xff,0xff,0xfd,0xf9,
0xf3,0xeb,0xe2,0xd7,0xca,0xbd,0xae,0x9f,
0x8f,0x80,0x70,0x60,0x51,0x42,0x35,0x28,
0x1d,0x14,0xc,0x6,0x2,0x0,0x0,0x2,
0x6,0xc,0x14,0x1d,0x28,0x35,0x42,0x51,
0x60,0x70};*/


//Gombok
volatile int valueDecPressedUp = 0;
volatile int valueDecPressedDown = 0;
volatile int menuNumberPressed = 0;
volatile int rotary = 0;

void i2cSendData (uint8_t data1, uint8_t data2) {
	TIMSK2 = (1<<OCIE2A);
	twi_data[0] = data1;
	twi_data[1] = data2;
	TCCR2A |= 0x03;//enable timer, 1/32 ... 
	twi_status |= TWI_START;
	//UDR0 = TCCR2A;
	//UDR0 = TIMSK2;
}

void i2cSendCommand (uint8_t c1) {
	
	while(twi_status!=0)
	{
		asm("nop");
	}
	i2cSendData (c1 | LCD_BACKLIGHT | LCD_EN, (c1 | LCD_BACKLIGHT) & ~LCD_EN);
	
};

void lcdBegin (void) {
	
	
	//Function Set 1
	i2cSendCommand ((0b00110000 &~LCD_RS) & ~LCD_RW);
	_delay_ms(5);
	i2cSendCommand ((0b00110000 &~LCD_RS) & ~LCD_RW);
	_delay_us(100);
	i2cSendCommand ((0b00110000 &~LCD_RS) & ~LCD_RW);
	_delay_ms(5);
	//Function Set 2
	i2cSendCommand ((0b00100000 &~LCD_RS) & ~LCD_RW);
	_delay_us(100);
	//Function Set 2 full
	i2cSendCommand ((0b00100000 &~LCD_RS) & ~LCD_RW);
	i2cSendCommand ((0b10000000 &~LCD_RS) & ~LCD_RW);
	_delay_us(53);
	//Display ON/OFF D-C-B OFF!!
	i2cSendCommand ((0b0000000 &~LCD_RS) & ~LCD_RW);
	i2cSendCommand ((0b1000000 &~LCD_RS) & ~LCD_RW);
	_delay_us(53);
	//Display Clear
	i2cSendCommand ((0b0000000 &~LCD_RS) & ~LCD_RW);
	i2cSendCommand ((0b00010000 &~LCD_RS) & ~LCD_RW);
	_delay_ms(3);
	//Entry Mode Set
	i2cSendCommand ((0b0000000 &~LCD_RS) & ~LCD_RW);
	i2cSendCommand ((0b0111000 &~LCD_RS) & ~LCD_RW);
	_delay_ms(1);
	
	//
	i2cSendCommand ((0b00000000 &~LCD_RS) & ~LCD_RW);
	i2cSendCommand ((0b11100000 &~LCD_RS) & ~LCD_RW);
	_delay_ms(1);
	
}

void lcdSetCursor (uint8_t col, uint8_t row)  {
	int row_offsets[] = {0b00000000, 0b01000000};
	//SET DDRAM Address
	i2cSendCommand	(((0b10000000 | row_offsets[row])& ~LCD_RW)& ~LCD_RS);
	i2cSendCommand(((0b00000000 | (row_offsets[row]+col)<<4)& ~LCD_RW )& ~LCD_RS);
}

void lcdWriteChar (uint8_t c) {
	
	i2cSendCommand(((c & 0xF0 ) | LCD_RS) & ~LCD_RW);
	i2cSendCommand((c << 4 | LCD_RS) & ~LCD_RW);
	
};

void lcdWriteString (char string[]) {
	int i = 0;
	while (string[i] != '\0') {
		lcdWriteChar(string[i]);
		i++;
	}
}

void lcdClear (void) {
	i2cSendCommand ((0b0000000 &~LCD_RS) & ~LCD_RW);
	i2cSendCommand ((0b00010000 &~LCD_RS) & ~LCD_RW);
	_delay_ms(2);
}

	
	
int first = 1;







void updateMenu() {
	switch (menuNumber) {
		case 0:
		menuNumber = 1;
		case 1:
		lcdClear();
		lcdSetCursor(0, 0);
		lcdWriteString(" < Frequency >");
		break;
		case 2:
		lcdClear();
		lcdSetCursor(0, 0);
		lcdWriteString(" < Amplitude >");
		break;
		case 3:
		lcdClear();
		lcdSetCursor(0, 0);
		if (setOutDiff)
		lcdWriteString("   < Comm. >");
		else
		lcdWriteString("   < Offset >");
		break;
		case 4:
		lcdClear();
		lcdSetCursor(0, 0);
		lcdWriteString("  < Diff out >");
		break;
		case 5:
		lcdClear();
		lcdSetCursor(0, 0);
		lcdWriteString("     < Out >");
		break;
		case 6:
		lcdClear();
		lcdSetCursor(0, 0);
		lcdWriteString("  < Waveform >");
		break;
	}
}


void updateValue(int up, int change, int valueDec) {
	switch (menuNumber) {
		case 1:
		if (up && change) {
			if (setFreq[valueDec] < 9) {
				setFreq[valueDec]++;
			}
			else {
				if (setFreq[valueDec+1] == 9)
				{
					if (setFreq[valueDec+2] != 9)
					{
						setFreq[valueDec]=0;
						setFreq[valueDec+1]=0;
						setFreq[valueDec+2]++;
					}
				}
				else {
					setFreq[valueDec]=0;
					setFreq[valueDec+1]++;
				}
			}
		}
		else if (change) {
			if (setFreq[valueDec] > 0) {
				setFreq[valueDec]--;
			}
			else
			{
				
				if (setFreq[valueDec+1] == 0)
				{
					if (setFreq[valueDec+2] != 0)
				{
					setFreq[valueDec]=9;
					setFreq[valueDec+1]=9;
					setFreq[valueDec+2]--;
				}
				}
				else {
				setFreq[valueDec]=9;
			setFreq[valueDec+1]--;
				}
			}
			
		}
		lcdSetCursor(0,1);
		lcdWriteString("                ");
		lcdSetCursor(7,1);
		lcdWriteChar(setFreq[0]+'0');
		lcdSetCursor(6,1);
		lcdWriteChar(setFreq[1]+'0');
		lcdSetCursor(5,1);
		lcdWriteChar(setFreq[2]+'0');
		lcdSetCursor(4,1);
		lcdWriteChar(setFreq[3]+'0');
		lcdSetCursor(9,1);
		lcdWriteString("Hz");

		//lcd.cursor();
		lcdSetCursor(7-valueDec,1);
		break;
		
		case 2:
		if (up && change) {
			if (valueDec < 4) {
			if (setAmp[valueDec] < 9) {
				setAmp[valueDec]++;
				if (setOutDiff == 1){
					if ((setAmp[0]+(setAmp[1]*10)+(setAmp[2]*100)+(setAmp[3]*1000))>=2047)
					{
						setAmp[0] = 7;
						setAmp[1] = 4;
						setAmp[2] = 0;
						setAmp[3] = 2;
					}
				}
				else
				{
					if ((setAmp[0]+(setAmp[1]*10)+(setAmp[2]*100)+(setAmp[3]*1000))>=6000)
				{
					setAmp[0] = 0;
					setAmp[1] = 0;
					setAmp[2] = 0;
					setAmp[3] = 6;
				}
				}
			}
			else {
				if (setAmp[valueDec+1] == 9)
				{
					if (setAmp[valueDec+2] != 9)
					{
						setAmp[valueDec]=0;
						setAmp[valueDec+1]=0;
						setAmp[valueDec+2]++;
					}
				}
				else {
					setAmp[valueDec]=0;
					setAmp[valueDec+1]++;
				}
			}
		}
		if ((valueDec == 4) && (sinemode == 0))
		sinemode = 1;
		}
		else if (change) {
			if (valueDec < 4)
			{
			if (setAmp[valueDec] > 0) {
				setAmp[valueDec]--;
			}
			else
			{
				
				if (setAmp[valueDec+1] == 0)
				{
					if (setAmp[valueDec+2] != 0)
					{
						setAmp[valueDec]=9;
						setAmp[valueDec+1]=9;
						setAmp[valueDec+2]--;
					}
				}
				else {
					setAmp[valueDec]=9;
					setAmp[valueDec+1]--;
				}
			}
			}
			if ((valueDec == 4) && (sinemode == 1))
			sinemode = 0;
		}
		lcdSetCursor(0,1);
		lcdWriteString("                ");
		lcdSetCursor(8,1);
		lcdWriteChar(setAmp[0]+'0');
		lcdSetCursor(7,1);
		lcdWriteChar(setAmp[1]+'0');
		lcdSetCursor(6,1);
		lcdWriteChar(setAmp[2]+'0');
		lcdSetCursor(5,1);
		lcdWriteString(",");
		lcdSetCursor(4,1);
		lcdWriteChar(setAmp[3]+'0');
		lcdSetCursor(3,1);
		if (sinemode)
		lcdWriteChar('B');
		else
		lcdWriteChar('U');
		lcdSetCursor(10,1);
		lcdWriteString("V");
		//lcd.cursor();
		if (valueDec == 3)
		lcdSetCursor(4,1);
		else if (valueDec == 4)
		lcdSetCursor(3,1);
		else
		lcdSetCursor(8-valueDec,1);
		
		if (sinemode)
		PORTA |= (1 << PA6);
		else
		{
		PORTA &= ~(1 << PA6);
		}
		
		if ((setOutDiff == 1) || ((setFreq[0]+(setFreq[1]*10)+(setFreq[2]*100)+(setFreq[3]*1000))== 0))
		potData = 0;
		else
		potData = 255 - ((setAmp[0]+(setAmp[1]*10)+(setAmp[2]*100)+(setAmp[3]*1000))/33);
		//potData = 120;
		TIMSK0 = (1<<OCIE0A);
		
		break;
		
		case 3:
		
		if (up && change) {
			if (valueDec < 4) {
			if (setOff[valueDec] < 9) {
				setOff[valueDec]++;
				if (setOutDiff == 0)
				{
					
				if ((setOff[0]+(setOff[1]*10)+(setOff[2]*100)+(setOff[3]*1000))>=2047)
				{
					setOff[0] = 7;
					setOff[1] = 4;
					setOff[2] = 0;
					setOff[3] = 2;
				}
				}
				else {
					if ((setOff[0]+(setOff[1]*10)+(setOff[2]*100)+(setOff[3]*1000))>=4095)
					{
						setOff[0] = 5;
						setOff[1] = 9;
						setOff[2] = 0;
						setOff[3] = 4;
					}
				}
			}
			
			else {
				if (setOff[valueDec+1] == 9)
				{
					if (setOff[valueDec+2] != 9)
					{
						setOff[valueDec]=0;
						setOff[valueDec+1]=0;
						setOff[valueDec+2]++;
					}
				}
				else {
					setOff[valueDec]=0;
					setOff[valueDec+1]++;
				}
			}
			}
			if ((valueDec == 4) && (isOffPos == 0))
			isOffPos = 1;
			
		}
		else if (change) {
			if (valueDec < 4)
			{
			if (setOff[valueDec] > 0) {
				setOff[valueDec]--;
			}
			else
			{
				
				if (setOff[valueDec+1] == 0)
				{
					if (setOff[valueDec+2] != 0)
					{
						setOff[valueDec]=9;
						setOff[valueDec+1]=9;
						setOff[valueDec+2]--;
					}
				}
				else {
					setOff[valueDec]=9;
					setOff[valueDec+1]--;
				}
			}
			}
			if ((valueDec == 4) && (isOffPos == 1))
			isOffPos = 0;
			
		}
		lcdSetCursor(0,1);
		lcdWriteString("                ");
		lcdSetCursor(8,1);
		lcdWriteChar(setOff[0]+'0');
		lcdSetCursor(7,1);
		lcdWriteChar(setOff[1]+'0');
		lcdSetCursor(6,1);
		lcdWriteChar(setOff[2]+'0');
		lcdSetCursor(5,1);
		lcdWriteString(",");
		lcdSetCursor(4,1);
		lcdWriteChar(setOff[3]+'0');
				lcdSetCursor(3,1);
			if (isOffPos)
				lcdWriteChar('+');
				else
				lcdWriteChar('-');
		lcdSetCursor(10,1);
		lcdWriteString("V");
		if (valueDec == 3)
		lcdSetCursor(4,1);
		else if (valueDec == 4)
		lcdSetCursor(3,1);
		else
		lcdSetCursor(8-valueDec,1);
		
		////////////////////////////
		if (setOutDiff)
		{
			if (isOffPos && (((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0])!= 0) && (((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0]) > 15) )
			{
			PORTA |= (1 << PA6);
			offValue = (128 - ((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0])/32) & 0x7F;
			PORTC = offValue;
			PORTC &= ~(1 << PC7);
			}
			else {
				offValue = ((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0])/32;
				PORTC = offValue & 0x7F;
				PORTC |= (1 << PC7);
		
			}
				PORTG = PORTG & ~(0b00000010);
				PORTG = PORTG & ~(0b00000001);
				PORTG = PORTG | (0b00000010);
				PORTG = PORTG | (0b00000001);
		}
		else
		{
			if (isOffPos && (((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0])!= 0) )
			{
				offValue = (2048 - ((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0])) & 0x07FF;
				PORTC = offValue & 0xFF;
				PORTD = (PORTD & 0b10000011) | (0b00011100 & (offValue >> 6));
			}
			else
			{
								offValue = ((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0]);
								PORTC = offValue & 0xFF;
								PORTD = (PORTD & 0b10000011) | (0b00011100 & (offValue) >> 6) | 0b00100000;
								
			}
			PORTD &= ~(0b00000001);
			PORTD &= ~(0b00000010);
			PORTD |= (0b00000001);
			PORTD |= (0b00000010);
			
		}
		
		break;
			
		case 4:
		if (up && change) {
			setOutDiff = 1;
			PORTC = 0b10000000;
			PORTG = PORTG & ~(0b00000010);
			PORTG = PORTG & ~(0b00000001);
			PORTG = PORTG | (0b00000010);
			PORTG = PORTG | (0b00000001);
			
		}
		else if (change) {
			setOutDiff = 0;
			//Set back the previous offset value
			//PORTC = ((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0]) & 0xFF;
			//PORTD = (0b00011100 & (((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0]) >> 6)) | 0b00100000;
			//PORTD &= ~(0b00000001);
			//PORTD &= ~(0b00000010);
			//PORTD |= (0b00000001);
			//PORTD |= (0b00000010);
		}
		//lcd.noCursor();

		if (setOutDiff)
		{
			lcdSetCursor(0,1);
			lcdWriteString("      ON ");
		}
		else
		{
			lcdSetCursor(0,1);
			lcdWriteString("      OFF");
		}
		
		break;
		
		case 5:
		if (up && change) {
			setOut = 1;
		}
		else if (change) {
			setOut = 0;
		}

		if (setOut)
		{
			lcdSetCursor(0,1);
			lcdWriteString("      ON ");
		}
		else
		{
			lcdSetCursor(0,1);
			lcdWriteString("      OFF");
		}
		//lcd.noCursor();
		break;
		case 6:
		if (up && change) {
			if (type < 3)
			type++;
		}
		else if (change) {
			if (type > 0)
			type--;
		}

		if (type == 0)
		{
			lcdSetCursor(0,1);
			lcdWriteString("      Sine   ");
		}
		else if (type == 1)
		{
			lcdSetCursor(0,1);
			lcdWriteString("     Square  ");
		}
		else if (type == 2)
		{
			lcdSetCursor(0,1);
			lcdWriteString("    Triangle");
		}
		else if (type == 3)
		{
			lcdSetCursor(0,1);
			lcdWriteString("    Sawtooth");
		}
		//lcd.noCursor();
		break;
		
	}
	
	
	
	if (((setFreq[0]+(setFreq[1]*10)+(setFreq[2]*100)+(setFreq[3]*1000))== 0))
	{
		if (setOutDiff == 1) {
			int amp = 2047 - (((setAmp[3]*1000)+(setAmp[2]*100)+(setAmp[1]*10)+setAmp[0]));
			PORTC = amp & 0xFF;
			PORTD = (0b00011100 & (amp>>6)) | (PORTD & (0b11000011));
			PORTD &= ~(0b00000001);
			PORTD &= ~(0b00000010);
			PORTD |= (0b00000001);
			PORTD |= (0b00000010);
		}
		else {
		TIMSK1 = (0<<OCIE1A);//disable interrupt
		PORTC = ((((setAmp[3]*1000)+(setAmp[2]*100)+(setAmp[1]*10)+setAmp[0])/63) & 0x7F) | 0x80;
		PORTG = PORTG & ~(0b00000010);
		PORTG = PORTG & ~(0b00000001);
		PORTG = PORTG | (0b00000010);
		PORTG = PORTG | (0b00000001);
		}
		TIMSK1 = (0<<OCIE1A);//disable interrupt
	}
	else {
	
	if ((setFreq[0]+(setFreq[1]*10)+(setFreq[2]*100)+(setFreq[3]*1000))<250)
	{
		period = 256;
	}

	else if ((setFreq[0]+(setFreq[1]*10)+(setFreq[2]*100)+(setFreq[3]*1000)) < 500)
	{
		period = 128;
	}
	else if ((setFreq[0]+(setFreq[1]*10)+(setFreq[2]*100)+(setFreq[3]*1000)) < 1000)
	{
		period = 64;
	}
	else if ((setFreq[0]+(setFreq[1]*10)+(setFreq[2]*100)+(setFreq[3]*1000)) < 2000)
	{
		period = 32;
	}
	else
	{
		period = 16;
	}
	
	factor = 256 / period;
	

	
	newOCRA = divided/((setFreq[0]+(setFreq[1]*10)+(setFreq[2]*100)+(setFreq[3]*1000))*period);
	OCR1A = newOCRA;
	//OCR1A = 1;
	TIMSK1 = (1<<OCIE1A);//enable interrupt
	}
}


/***Rotary encoder***/
int val;
int encoder0PinA = 10;
int encoder0PinB = 13;
int encoder0SW = 9;
int encoder0Pos = 0;
int encoder0PinALast = 0;
int na = 0;
int i=0;





int main(void)
{
	/***Port directions***/
	DDRA |= (1 << PA6) | (1 << PA5) | (1 << PA4) | (1 << PA3);
	DDRB &= ~(1<<PB0);
	DDRC |= 0xFF;
	DDRD |= (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD7);
	DDRE &= ~((1<<PE3)|(1<<PE4)|(1<<PE5)|(1<<PE2)|(1<<PE6)|(1<<PE7));//input
	DDRG |= (1<<PG4) | (1<<PG1) | (1<<PG0);
	DDRF |= (1<<PF7);
	DDRB |= (1<<PB5);
	
	/***Port set***/
	
	//Bipolar default
	PORTA |= (1 << PA6);
	
	//POT_CS and POT_SCK High
	PORTA |= (1 << PA3) | (1 << PA4);
	
	PORTB |= (1<<PB0);
	
	//Switch off diff. output
	PORTD &= ~(1 << PD7);
	
	//Set high CS_DA12 & WR_DA12
	PORTD = PORTD  | (0b00000010);
	PORTD = PORTD | (0b00000001);
	
	//Set high CS_DA8 & WR_DA8
	PORTG = PORTG | (0b00000010);
	PORTG = PORTG | (0b00000001);
	

	PORTE &= ~((1<<PE4)|(1<<PE5));//pull-up off
	PORTE |= (1<<PE3) | (1<<PE2) | (1<<PE6) | (1<<PE7) ;//pull-up on
	
	//Set offset 0
	PORTC = ((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0]) & 0xFF;
	PORTD = (0b00011100 & (((setOff[3]*1000)+(setOff[2]*100)+(setOff[1]*10)+setOff[0]) >> 6)) | (PORTD & (0b11000011));
	PORTD &= ~(0b00000001);
	PORTD &= ~(0b00000010);
	PORTD |= (0b00000001);
	PORTD |= (0b00000010);
	
	
	UBRR0L = 12;//38400bps
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);//8N1
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);//enable
	
	UDR0 = 'A';
	
	/***Counters***/
	
	//Counter 0 (Digital potmeter)
	
	//64 prescale, ctc mode:
	TCCR0A = (1<<WGM01)|(1<<CS02)|(1<<CS00);
	TIMSK0 = (1<<OCIE0A);
	OCR0A = 3;
	
	
	//Counter 1 (Sine waveform generation)
	TCCR1B = (1<<WGM12)|(1<<CS10);//Enable counter, 1/1024 prescaler; Timer1: CTC mode
	
	//TCCR1A = (1<<WGM10)|(1<<WGM11)|(1<<COM1A0)|(1<<COM1B0);
	//TCCR1B = (1<<WGM12)|(1<<CS10)|(1<<WGM13);//Enable counter, no prescaler; Timer1: CTC mode
	TIMSK1 = (1<<OCIE1A);//enable interrupt
	OCR1A = 2;
	
	//Counter 2 (i2c communication - LCD)
	TCCR2A = (1<<WGM21); //Timer2: CTC mode
	TIMSK2 = (1<<OCIE2A);
	//OCR2A = 3; !!
	OCR2A = 1;
	
	
	/*
	PORTC = 0b10000000;
	PORTG = PORTG & ~(0b00000010);
	PORTG = PORTG & ~(0b00000001);
	PORTG = PORTG | (0b00000010);
	PORTG = PORTG | (0b00000001);
	*/
	
	
	
	sei();


	while(1)
    {
		
	if (first <= 1)
		{
				lcdBegin();
				updateMenu();
				updateValue(0, 0, 0);
				first++;
				
		}

if (first >= 2)
{
	//char key = keypad.getKey();
   if (!(PINE & 0x04)){
	   if (menuNumberPressed == 0)
	   {
	   
	   valueDec = 0;
	   if (menuNumber == 6)
	   menuNumber = 0;
	   menuNumber++;
	   ////////////////////////////////////////!!!!!!!!!!
	   _delay_ms(50);
	   updateMenu();
	   updateValue(0, 0, 0);
	   menuNumberPressed = 1;
	   }
   }
   else {
	   menuNumberPressed = 0;
   }
   
   
   na = ~(PINE & 0x40);
   if ((~(encoder0PinALast & 0x40)) && (na & 0x40)) {
	   if (rotary == 0)
	   {
	   if ((PINE & 0x80)) {
		   updateValue(1, 1, valueDec);
		   rotary = 1;
		   } else {
		   updateValue(0, 1, valueDec);
		   rotary = 1;
	   }
	   }
   }
   else {
	   rotary =0;
   }
   encoder0PinALast = na;
   
   if (setOut) {
	   PORTG &= ~(1<<PG4);
   } else {
	   PORTG |= (1<<PG4);
   }
   
   
   if (setOutDiff) {
	   //PORTD &= ~(1 << PD7);
	   PORTD |= (1 << PD7); 
	   } else {
	   PORTD &= ~(1 << PD7);
   }


  if (!(PINE & 0x08)){
    if((valueDec < 4) && ((valueDecPressedUp) == 0))
	{
   valueDec++;
   updateValue(0, 0, valueDec);
   valueDecPressedUp = 1;
	}
  }
  else {
	  valueDecPressedUp = 0;
  }

  if (!(PINB & 0x01)){
    if ((valueDec != 0) && ((valueDecPressedDown) == 0))
	{
   valueDec--;
   updateValue(0, 0, valueDec);
   valueDecPressedDown = 1;
	}
  }
  else
  {
	  valueDecPressedDown = 0;
  }
}	
		//TODO:: Please write your application code 
    

//potData = 0;





   }
   
   }


ISR (TIMER0_COMP_vect) {
	if (poti == 0)
	PORTA &= ~(1 << PA3);
	
	if (PORTA & 0b00010000)
	{
		PORTA &= ~(0b00010000);
		PORTA = (PORTA & 0b11011111) | ((((potData << poti) & 0x8000) >> 10) & (0b00100000));
		if (poti == 16)
		{
			PORTA |= (1 << PA3);
			poti = 0;
			TIMSK0 &= ~(1<<OCIE0A);
		}
		else
		poti++;
	}
	else
	{
		PORTA |= (0b00010000);
	}
	
	
}



ISR(TIMER1_COMPA_vect)


{
	//PORTF = (~(PORTF & 0b10000000)) & 0b10000000;
	
	if (setOutDiff) {
		
		PORTC = (sine[type][i] << 4) & (0x0f0);
		PORTD = ((sine[type][i] >> 2) & (0b00111100)) | (PORTD & 0b11000011) ;
		PORTD &= ~(0b00000001);
		PORTD &= ~(0b00000010);
		PORTD |= (0b00000001);
		PORTD |= (0b00000010);
		}
	else {
		if (sinemode) {
			PORTC = sine[type][i];
			}
		else
			PORTC = (sine[type][i] << 1);
	
	PORTG = PORTG & ~(0b00000010);
	PORTG = PORTG & ~(0b00000001);
	PORTG = PORTG | (0b00000010);
	PORTG = PORTG | (0b00000001);
	}
	
	/*
	if (i == 100)
	{
	PORTF = (~(PORTF & 0b10000000)) & 0b10000000;
	}*/
	
	i = i + factor;
	
	if ((sinemode == 0) && (setOutDiff == 0))
	{
		if (i == 128)
		i=0;
	}
	else
	{
		if (i >= 255)
		i=0;
	}
	
	OCR1A = newOCRA;
	
	
}


ISR(TIMER2_COMP_vect)
{
	static uint8_t cnt = 0;
	static uint8_t data_bit = 0;
	static uint8_t data_num = 0;
	uint8_t temp = 0;
	if (twi_status & TWI_START)
	{
		DDRE |= (1<<PE5);//pull SDA
		twi_status &= ~TWI_START;
		twi_status |= SEND_ADDRESS;
		cnt = 1;
	}
	else if ((twi_status & SEND_ADDRESS)||(twi_status & SEND_DATA))
	{
		if (cnt == 1)
		{
			if (data_bit == 9)//read ACK
			{
				temp = (PINE & 0x20);//mask PIN E 5
				if (temp == 0)//ACK OK
				{
					twi_status |= ACK_OK;
					if (twi_status & SEND_ADDRESS)
					{
						twi_status &= ~SEND_ADDRESS;
						twi_status|= SEND_DATA;
					}
					else if (data_num == 0)//send_data
					{
						data_num++;
					}
					else //ready: data num = 1
					{
						twi_status &= ~SEND_DATA;//stop transmission
						twi_status |= STOP_COND;
						data_num = 0;
						data_bit = 0;
					}
					
					UDR0 = 'O';
					data_bit = 0;
				}
				else//ACK not OK
				{
					twi_status = 0;
					TCCR2A &= ~(0x05);//disable timer
					cnt = 0;
					data_bit = 0;
					DDRE &= ~((1<<PE5)|(1<<PE4));//release SDA, SCL
					UDR0 = 'N';
				}
				
			}
			else
			{
				DDRE |= (1<<PE4);//pull SCL
				cnt = 2;
			}
			
		}
		else if (cnt == 2)//data out to SDA
		{
			if (data_bit == 8)//read ACK
			{
				DDRE &= ~(1<<PE5);//release SDA
				data_bit = 9;
				cnt = 3;
			}
			else //normal address/data bit
			{
				if (twi_status & SEND_ADDRESS)//address bit to send
				{
					temp = (address<<data_bit);
				}
				else//data to send
				{
					temp = (twi_data[data_num]<<data_bit);
				}
				if (temp & 0x80)
				{
					DDRE &= ~(1<<PE5);//release SDA
				}
				else
				{
					DDRE |= (1<<PE5);//pull SDA
				}
				
				cnt = 3;
				data_bit ++;
			}
		}
		else //cnt=3
		{
			DDRE &= ~(1<<PE4);//release SCL
			cnt = 1;
		}
	}
	else if (twi_status & STOP_COND)
	{
		if (cnt == 1)//SCL to pull
		{
			DDRE |= (1<<PE4);//pull SCL
			cnt = 2;
		}
		else if (cnt == 2)//SDA to pull
		{
			DDRE |= (1<<PE5);//pull SDA
			cnt = 3;
		}
		else if(cnt == 3) //release SCL
		{
			DDRE &= ~(1<<PE4);//pull SCL
			cnt = 4;
		}
		else//cnt = 4, release SDA
		{
			DDRE &= ~(1<<PE5);//release SDA
			twi_status = 0;
			cnt = 0;
			UDR0 = 'R';
			//TCCR2A &= ~(0x05);//disable timer
			TIMSK2 &= ~(1<<OCIE2A);
		}
	}
	
}
