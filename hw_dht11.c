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
 * 
 * @purpose DHT11 Humidity & Temperature Sensor
 *	DHT11
 *	20-90%RH ±5?RH
 *	0-50 ? ±2?
 *	4 Pin Single Row
 */


#include <avr/interrupt.h>
#include "hw_dht11.h"
#include "aux_globals.h"

#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
volatile unsigned long timer0_overflow_count = 0;

/************************************************************************
 * Returns the number of microseconds since the uC board began 
 * running the current program. This number will overflow (go back to zero), 
 * after approximately 70 minutes. 
 ************************************************************************/
unsigned long micros() {
	unsigned long m, t;
	uint8_t oldSREG = SREG;
	cli();
	t = TCNT0;
	if ((TIFR & _BV(TOV0)) && (t == 0)) t = 256;
	m = timer0_overflow_count;
	SREG = oldSREG;	
	return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
}

// Based on Arduino code: http://arduino.cc/playground/Main/DHT11Lib
int DHT11_read(int *temp, int *humidity)
{
	// BUFFER TO RECEIVE
	uint8_t bits[5];
	uint8_t cnt = 7;
	uint8_t idx = 0;

	// EMPTY BUFFER
	for (int i=0; i< 5; i++) bits[i] = 0;

	// REQUEST SAMPLE
	DHT11_OUTPUT_MODE();
	DHT11_LOW();
	fcpu_delay_ms(18);
	DHT11_HIGH();
	fcpu_delay_us(40);
	DHT11_INPUT_MODE();

	// ACKNOWLEDGE or TIMEOUT
	unsigned int loopCnt = 10000;
	
	while(!(DHT11_PIN&(1<<DHT11_DQ))) //pin is low
		if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

	loopCnt = 10000;
	while((DHT11_PIN&(1<<DHT11_DQ))) // pin is high
	if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

	// READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
	for (int i=0; i<40; i++)
	{
		loopCnt = 10000;
		while(!(DHT11_PIN&(1<<DHT11_DQ)))
			if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

		unsigned long t = micros();

		loopCnt = 10000;
		while((DHT11_PIN&(1<<DHT11_DQ))) //high
			if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

		if ((micros() - t) > 40) bits[idx] |= (1 << cnt);
		if (cnt == 0)   // next byte?
		{
			cnt = 7;    // restart at MSB
			idx++;      // next byte!
		}
		else cnt--;
	}

	// WRITE TO RIGHT VARS
	// as bits[1] and bits[3] are always zero they are omitted 
	*humidity    = bits[0];
	*temp = bits[2];

	uint8_t sum = bits[0] + bits[2];

	if (bits[4] != sum) return DHTLIB_ERROR_CHECKSUM;
	return DHTLIB_OK;
}
