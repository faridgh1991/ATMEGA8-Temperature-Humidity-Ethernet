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

/*********************************************
 * CONFIGURE THE WORKING PIN
 *********************************************/

#pragma once

#include <avr/io.h> 
#include <stdio.h>

#define THERM_PORT PORTC
#define THERM_DDR DDRC
#define THERM_PIN PINC
#define THERM_DQ PC4
/* Utils */
#define THERM_INPUT_MODE() THERM_DDR&=~(1<<THERM_DQ)
#define THERM_OUTPUT_MODE() THERM_DDR|=(1<<THERM_DQ)
#define THERM_LOW() THERM_PORT&=~(1<<THERM_DQ)
#define THERM_HIGH() THERM_PORT|=(1<<THERM_DQ)
/* list of these commands translated into C defines:*/
#define THERM_CMD_CONVERTTEMP 0x44
#define THERM_CMD_RSCRATCHPAD 0xbe
#define THERM_CMD_WSCRATCHPAD 0x4e
#define THERM_CMD_CPYSCRATCHPAD 0x48
#define THERM_CMD_RECEEPROM 0xb8
#define THERM_CMD_RPWRSUPPLY 0xb4
#define THERM_CMD_SEARCHROM 0xf0
#define THERM_CMD_READROM 0x33
#define THERM_CMD_MATCHROM 0x55
#define THERM_CMD_SKIPROM 0xcc
#define THERM_CMD_ALARMSEARCH 0xec
/* constants */
#define THERM_DECIMAL_STEPS_12BIT 625 //.0625

void therm_delay(uint16_t delay);
uint8_t therm_reset();
void therm_write_bit(uint8_t bit);
uint8_t therm_read_bit(void);
uint8_t therm_read_byte(void);
void therm_write_byte(uint8_t byte);
//void therm_read_temperature(int *digi, int *deci);
float therm_read_temperature();