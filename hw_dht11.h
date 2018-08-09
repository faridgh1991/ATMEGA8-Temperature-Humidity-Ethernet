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

/*********************************************
 * CONFIGURE THE WORKING PIN
 *********************************************/

#pragma once

#include <avr/io.h> 
#include <stdio.h>

#define DHT11_PORT PORTC
#define DHT11_DDR DDRC
#define DHT11_PIN PINC
#define DHT11_DQ PC3
/* Utils */
#define DHT11_INPUT_MODE() DHT11_DDR&=~(1<<DHT11_DQ)
#define DHT11_OUTPUT_MODE() DHT11_DDR|=(1<<DHT11_DQ)
#define DHT11_LOW() DHT11_PORT&=~(1<<DHT11_DQ)
#define DHT11_HIGH() DHT11_PORT|=(1<<DHT11_DQ)

#define DHTLIB_OK                               0
#define DHTLIB_ERROR_CHECKSUM					-1
#define DHTLIB_ERROR_TIMEOUT					-2


int DHT11_read(int *temp, int *humidity);