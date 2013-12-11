/* **************************************************************************																					
 *                                OpenPicus                 www.openpicus.com
 *                                                            italian concept
 * 
 *            openSource wireless Platform for sensors and Internet of Things	
 * **************************************************************************
 *  FileName:        eeprom_helper.h
 *  Dependencies:    Microchip configs files
 *  Module:          FlyPort WI-FI B/G, ETHERNET and GPRS
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Simone Marra 		 1.0     06/27/2013		   First release  (core team)
 *  
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  Software License Agreement
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by 
 *  the Free Software Foundation AND MODIFIED BY OpenPicus team.
 *  
 *  ***NOTE*** The exception to the GPL is included to allow you to distribute
 *  a combined work that includes OpenPicus code without being obliged to 
 *  provide the source code for proprietary components outside of the OpenPicus
 *  code. 
 *  OpenPicus software is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details. 
 * 
 * 
 * Warranty
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * WE ARE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 **************************************************************************/
 
#ifndef __EEPROM_HELPER_H
#define __EEPROM_HELPER_H

#include "HWlib.h"
#if ((defined FLYPORTGPRS) || (defined FLYPORT_LITE))
#include "Delay.h"
#else
#include "TCPIP Stack/Delay.h"
#endif

//#define HIGH_SPEED 0x25
//#define LOW_SPEED 0x9D
#define EEPROM_ADDR_DEF	0B1010000 /* 0xA0 >> 1 */
#define EEPROM_SIZE_DEF	8192 /* Calculated with (eeprom model size)*1024/8 = (eeprom model size)*128 => 64*128 = 8192*/
#define EEPROM_PAGE_DEF	32
#define EEPROM_RWDL_DEF 100

#define EEPROM_BYTE 	0x01
#define EEPROM_WORD 	0x02
#define EEPROM_DWORD 	0x04

#define EEPROM_ERR_NO			0x00
#define EEPROM_ERR_TIMEOUT		0x01
#define EEPROM_ERR_NO_SPACE 	0x02
#define EEPROM_ERR_UNK_DATA		0x03
#define EEPROM_ERR_POLLING		0x04

// Setup Functions
void EepromInit(BYTE deviceAddr, BYTE speed, WORD maxEepromSize);
void EepromRWDelay(int rwDelayToSet);
void EepromSetPageSize(int pageLen);
int  EepromGetPageSize();

// Data Storage Functions
BYTE EepromSaveData(WORD addr, void* dataToWr, WORD dataLen, BYTE dataType);
BYTE EepromLoadData(WORD addr, void* dataToRd, WORD dataLen, BYTE dataType);
BYTE EepromEraseData(WORD addr, WORD dataLen, BYTE dataType);

// String Storage Functions
BYTE EepromSaveString(WORD addr, char* strToWr, WORD strLen);
BYTE EepromLoadString(WORD addr, char* strToRd, WORD strLen);
BYTE EepromEraseString(WORD addr, WORD strLen);




#endif
