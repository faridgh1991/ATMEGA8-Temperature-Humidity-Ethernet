/* @project 
 * 
 * License to access, copy or distribute this file:
 * This file or any portions of it, is Copyright (C) 2012, Radu Motisan ,  http://www.pocketmagic.net . All rights reserved.
 * This file can be used for free only in educational or non commercial applications, assuming this original 
 * copyright message is preserved.
 *
 * @author Radu Motisan, radu.motisan@gmail.com
 * 
 * This file is protected by copyright law and international treaties. Unauthorized access, reproduction 
 * or distribution of this file or any portions of it may result in severe civil and criminal penalties.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * 
 * @purpose DS18B20 Temperature Sensor Minimal interface for Atmega microcontrollers
 * http://www.pocketmagic.net/
 */


#include "hw_ds18b20.h"
#include "aux_globals.h"


uint8_t therm_reset()
{
	uint8_t i;
	//Pull line low and wait for 480uS
	THERM_LOW();
	THERM_OUTPUT_MODE();
	fcpu_delay_us(430);	//480 //this must be smaller when moving delay func to other .c file
	//Release line and wait for 60uS
	THERM_INPUT_MODE();
	fcpu_delay_us(60); //60
	//Store line value and wait until the completion of 480uS period
	i=(THERM_PIN & (1<<THERM_DQ));
	fcpu_delay_us(420); //420
	//Return the value read from the presence pulse (0=OK, 1=WRONG)
	return i;
}

void therm_write_bit(uint8_t bit)
{
	//Pull line low for 1uS
	THERM_LOW();
	THERM_OUTPUT_MODE();
	fcpu_delay_us(1);//1
	//If we want to write 1, release the line (if not will keep low)
	if(bit) THERM_INPUT_MODE();
	//Wait for 60uS and release the line
	fcpu_delay_us(50);//60
	THERM_INPUT_MODE();
}

uint8_t therm_read_bit(void)
{
	uint8_t bit=0;
	//Pull line low for 1uS
	THERM_LOW();
	THERM_OUTPUT_MODE();
	fcpu_delay_us(3);//1
	//Release line and wait for 14uS
	THERM_INPUT_MODE();
	fcpu_delay_us(10);//14
	//Read line value
	if(THERM_PIN&(1<<THERM_DQ)) bit=1;
	//Wait for 45uS to end and return read value
	fcpu_delay_us(53);//45
	return bit;
}

uint8_t therm_read_byte(void)
{
	uint8_t i=8, n=0;
	while(i--) {
		//Shift one position right and store read value
		n>>=1;
		n|=(therm_read_bit()<<7);
	}
	return n;
}

uint16_t therm_read_word(void)
{
	uint16_t i=16, n=0;
	while(i--) {
		//Shift one position right and store read value
		n>>=1;
		n|=(therm_read_bit()<<15);
	}
	return n;
}

void therm_write_byte(uint8_t byte)
{
	uint8_t i=8;
	while(i--) {
		//Write actual bit and shift one position right to make the next bit ready
		therm_write_bit(byte&1);
		byte>>=1;
	}
}
/*
void therm_read_temperature(int *digi, int *deci)
{
	//Reset, skip ROM and start temperature conversion
	therm_reset();
	therm_write_byte(THERM_CMD_SKIPROM);
	therm_write_byte(THERM_CMD_CONVERTTEMP);
	//Wait until conversion is complete
	while(!therm_read_bit());
	//Reset, skip ROM and send command to read Scratchpad
	therm_reset();
	therm_write_byte(THERM_CMD_SKIPROM);
	therm_write_byte(THERM_CMD_RSCRATCHPAD);
	//Read Scratchpad (only 2 first bytes)
	uint16_t temp = therm_read_word();
	therm_reset();
	// first 12bits represent the integer temperature
	// the last 4bits represent the fractional temperature
	
	// I I I I  I I I I  I I I I  F F F F
	// 1 0 0 0  0 0 0 0  0 0 0 0 negative temp
	// 0 1 1 1  1 1 1 1  1 1 1 1
	//Store temperature integer digits and decimal digits
		
	uint16_t integer_bits = temp >> 4,
			fractional_bits = temp & 0xF;
	int negative = 0;
	if (integer_bits & 0x800) negative = 1; //the 12th digit is one, representing a negative number
	if (negative) {
		*digi =  (-1)*((!integer_bits) & 0x7FF); // complement.  leave only the 11 bits "positive" part
		*deci = (!fractional_bits)& 0xF;
	}
	else {
		*digi = integer_bits & 0x7FF; //leave only the 11 bits "positive" part
		*deci= fractional_bits;
	}
}*/

float therm_read_temperature()
{
	//Reset, skip ROM and start temperature conversion
	therm_reset();
	therm_write_byte(THERM_CMD_SKIPROM);
	therm_write_byte(THERM_CMD_CONVERTTEMP);
	//Wait until conversion is complete
	while(!therm_read_bit());
	//Reset, skip ROM and send command to read Scratchpad
	therm_reset();
	therm_write_byte(THERM_CMD_SKIPROM);
	therm_write_byte(THERM_CMD_RSCRATCHPAD);
	uint8_t l = therm_read_byte();
	uint8_t h = therm_read_byte();
	therm_reset();
	float temp = ( (h << 8) + l )*0.0625;
	return temp;
}																																																																																