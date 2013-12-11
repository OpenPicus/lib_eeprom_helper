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
 
#include "../ExternalLib/Include/eeprom_helper.h"

static BYTE _deviceAddress	= EEPROM_ADDR_DEF; // WARNING 7 bit format!!!
static WORD _maxEepromSize	= EEPROM_SIZE_DEF;
static int  _rwDelayEeprom	= EEPROM_RWDL_DEF;
static int	_pageSize	= EEPROM_PAGE_DEF;
static WORD _currPageAddr	= 0;
static WORD _nextPageAddr	= 0;
static WORD _currAddr		= 0;

// Init Function
void EepromInit(BYTE deviceAddr, BYTE speed, WORD maxEepromSize)
{
	// Set device address
	_deviceAddress = deviceAddr;
	_currAddr = 0;
	
	TRISGbits.TRISG2 = 1;
	TRISGbits.TRISG3 = 1;                     
	I2C1TRN = 0x0000;
	I2C1RCV = 0x0000;
	
	// check bus speed: if it was just set as 
	// LOW_SPEED don't change it! 
	if(I2C1BRG != LOW_SPEED)
		I2C1BRG = speed;
	
	I2C1CON = 0x8200;	
	
	_maxEepromSize = maxEepromSize;
	
	_nextPageAddr = _currPageAddr + _pageSize;
	_currPageAddr = 0;
}


void EepromRWDelay(int rwDelayToSet)
{
	_rwDelayEeprom = rwDelayToSet;
}

void EepromSetPageSize(int pageLen)
{
	_pageSize = pageLen;
}

int EepromGetPageSize()
{
	return _pageSize;
}

static BOOL _eepromChangeAddr(WORD addr)
{
	// Set current Address
	_currAddr = addr;
	
	// Initiate Write operation:
	BYTE devAddr = _deviceAddress << 1;
 	I2CStart();						//Start sequence
	
	BOOL devAck = I2CWrite(devAddr & 0xFE);		//Initiate write sequence
 	// Check if device is available
 	if(devAck == FALSE)
 	{
 		I2CStop(); 			// Executes a STOP Transition
 		return FALSE; 		// and return
 	}
	BYTE rep = (addr >> 8) & 0xFF;	// get Most significant byte of address
	I2CWrite(0xFF & rep);
	
	I2CWrite( 0xFF & addr);				//Send register to start reading
	
	return TRUE;
}

static BOOL _eepromNextPage()
{
	// Stop last operation
	I2CStop();
	
	// Start the polling of EEPROM until timeout or ACK received
	BYTE devAddr = _deviceAddress << 1;
	int cnt = _rwDelayEeprom;
	BOOL pollRes;
	while(cnt > 0)
	{
		pollRes = I2CGetDevAck(devAddr);
		if(pollRes != TRUE)
			cnt--;
		else
			break;
	}
	
//	// If result of polling is bad break operation...
//	if(pollRes != TRUE)
//		return FALSE;
		
	_currPageAddr = _nextPageAddr;
	_nextPageAddr += _pageSize;
	if(_nextPageAddr > _maxEepromSize)
		_nextPageAddr = 0;
	return _eepromChangeAddr(_currPageAddr);
}

static BOOL _eepromCheckPageSwitch()
{
	// Increase current Address value...
	_currAddr++;
	// ..and check if we need to change page:
	if(_currAddr > _nextPageAddr-1)
		return _eepromNextPage();
	else
		return TRUE;
		
}

static BOOL _eepromSendBytes(BYTE* dat, WORD datlen)
{
	WORD cnt;
	BOOL res = FALSE;
	for (cnt = 0; cnt < datlen; cnt++)
	{
		res = I2CWrite(dat[cnt]);
		
		_eepromCheckPageSwitch();
		
		if(res != TRUE)
			return res;
	}
	return res;
}

static BOOL _eepromSendWords(WORD* dat, WORD datlen)
{
	WORD cnt;
	BOOL res = FALSE;
	BYTE b1, b2;
	for (cnt = 0; cnt < datlen; cnt++)
	{
		b1 = (BYTE)dat[cnt];
		b2 = (BYTE)(dat[cnt]>>8);
		res = I2CWrite(b2);
		_eepromCheckPageSwitch();
		if(res != TRUE)
			return res;
		
		res = I2CWrite(b1);
		_eepromCheckPageSwitch();
		if(res != TRUE)
			return res;
		
	}
	return res;
}

static BOOL _eepromSendDwords(DWORD* dat, WORD datlen)
{
	WORD cnt;
	BOOL res = FALSE;
	BYTE b1, b2, b3, b4;
	for (cnt = 0; cnt < datlen; cnt++)
	{
		// Debug only: DWORD tempVal = dat[cnt];
		b1 = (BYTE)dat[cnt];
		b2 = (BYTE)(dat[cnt]>>8);
		b3 = (BYTE)(dat[cnt]>>16);
		b4 = (BYTE)(dat[cnt]>>24);
		
		res = I2CWrite(b4);
		_eepromCheckPageSwitch();
		if(res != TRUE)
			return res;
		
		res = I2CWrite(b3);
		_eepromCheckPageSwitch();
		if(res != TRUE)
			return res;
		
		res = I2CWrite(b2);
		_eepromCheckPageSwitch();
		if(res != TRUE)
			return res;
		
		res = I2CWrite(b1);
		_eepromCheckPageSwitch();
		if(res != TRUE)
			return res;
		
	}
	return res;
}

static BOOL _eepromReceiveBytes(BYTE* dat, WORD datlen)
{
	WORD cnt;
	for(cnt = 0; cnt < (datlen-1); cnt++)
	{
		dat[cnt] = I2CRead(0);
		if(I2CTimeout())
			return FALSE;
	}
	dat[cnt] = I2CRead(1); // Last BYTE, stop to read data
	if(I2CTimeout())
		return FALSE;
	else
		return TRUE;
}

static BOOL _eepromReceiveWords(WORD* dat, WORD datlen)
{
	WORD cnt;
	BYTE d1, d2;
	WORD dd1,dd2;
	for(cnt = 0; cnt < (datlen-1); cnt++)
	{
		d2 = I2CRead(0);
		if(I2CTimeout())
			return FALSE;
		d1 = I2CRead(0);
		if(I2CTimeout())
			return FALSE;
			
		dd1 = d1 & 0xFF;
		dd2 = (WORD)d2;
		dd2 = (dd2<<8);
		
		dat[cnt] = dd1 | dd2;
	}
	d2 = I2CRead(0);
	if(I2CTimeout())
		return FALSE;
	d1 = I2CRead(1);
	if(I2CTimeout())
		return FALSE;
		
	dd1 = d1 & 0xFF;
	dd2 = (WORD)d2;
	dd2 = (dd2<<8);
		
	dat[cnt] = dd1 | dd2;
	
	if(I2CTimeout())
		return FALSE;
	else
		return TRUE;
}

static BOOL _eepromReceiveDwords(DWORD* dat, WORD datlen)
{
	WORD cnt;
	BYTE d1, d2, d3, d4;
	DWORD dd1, dd2, dd3, dd4;
	
	for(cnt = 0; cnt < (datlen-1); cnt++)
	{
		d4 = I2CRead(0);
		if(I2CTimeout())
			return FALSE;
		d3 = I2CRead(0);
		if(I2CTimeout())
			return FALSE;
		d2 = I2CRead(0);
		if(I2CTimeout())
			return FALSE;
		d1 = I2CRead(0);
		if(I2CTimeout())
			return FALSE;
			
		dd1 = d1;
		dd2 = (DWORD)d2;
		dd2 = (dd2<<8);
		
		dd3 = (DWORD)d3;
		dd3 = dd3<<16;
		
		dd4 = (DWORD)d4;
		dd4 = dd4<<24;
		
		// Debug only: DWORD tempVal = dd1 | dd2 | dd3 | dd4;
		dat[cnt] = dd1 | dd2 | dd3 | dd4;
	}
	d4 = I2CRead(0);
	if(I2CTimeout())
		return FALSE;
	d3 = I2CRead(0);
	if(I2CTimeout())
		return FALSE;
	d2 = I2CRead(0);
	if(I2CTimeout())
		return FALSE;
	d1 = I2CRead(1);
	if(I2CTimeout())
		return FALSE;
		
	dd1 = d1;
	dd2 = (DWORD)d2;
	dd2 = (dd2<<8);
	
	dd3 = (DWORD)d3;
	dd3 = dd3<<16;
	
	dd4 = (DWORD)d4;
	dd4 = dd4<<24;
	
	dat[cnt] = dd1 | dd2 | dd3 | dd4;
				  
	if(I2CTimeout())
		return FALSE;
	else
		return TRUE;
}

// Data Storage Function
BYTE EepromSaveData(WORD addr, void* dataToWr, WORD dataLen, BYTE dataType)
{
	int pagePointer;
	
	BYTE rep = EEPROM_ERR_NO;
	
	// Calculate if there is enoght space left on eeprom, starting from addr
	int bytesNeeded = _maxEepromSize - addr - (dataLen*dataType);
	// No more space available
	if(bytesNeeded  < 0)
		return EEPROM_ERR_NO_SPACE;
	
	// Save current address register size
	BYTE oldAddrSize = I2CAddrSizeGet();
	// Set 2 BYTEs address register size
	I2CAddrSizeSet(2);
	
	/* Se stiamo scrivendo dentro una pagina, possiamo usare il "WriteBurst" fino alla fine della pagina.
	** Se oltrepassiamo il limite superiore, la scrittura continua dall'inizio della pagina!!!
	**
	** Per ovviare a questo problema devo calcolare a che punto della pagina siamo e in caso siamo a fine pagina
	** inviare I2CStop + I2CGetDevAck() finchè la EEPROM non riprenda a rispondere + di nuovo tutto 
	** l'ambaradan dello I2CStart() + indirizzamento + Invio dati fino a fine pagina (se necessario) e proseguire!!!
	*/
	
	//page calculation and page moving during writes...
	if(addr < _pageSize)	// First Page of EEPROM
	{
		_nextPageAddr = _pageSize;
		_currPageAddr = 0;
	}
	else // Any other page except of first page
	{
		pagePointer = _maxEepromSize; 		// Set pagePointer to the end of eeprom
		while(pagePointer > addr) 		   	// decrease it until reaches the start of page related to our address
			pagePointer -= _pageSize;

		_currPageAddr = pagePointer;
		_nextPageAddr = _currPageAddr + _pageSize;
	}
	
	// Change eeprom Address (will change also _currAddr)
	if(_eepromChangeAddr(addr) == FALSE)
		return FALSE;
	
	switch(dataType)
	{
		case EEPROM_BYTE:
			vTaskSuspendAll();
			_eepromSendBytes(dataToWr, dataLen);		
			xTaskResumeAll();
			break;
			
		case EEPROM_WORD:
			vTaskSuspendAll();
			_eepromSendWords(dataToWr, dataLen);			
			xTaskResumeAll();
			break;
			
		case EEPROM_DWORD:
			vTaskSuspendAll();
			_eepromSendDwords(dataToWr, dataLen);			
			xTaskResumeAll();
			break;
			
		default: 
			// restore old address register size
			I2CAddrSizeSet(oldAddrSize);
			rep = EEPROM_ERR_UNK_DATA;
			break;
	}
	
	// Check report
	if(rep != EEPROM_ERR_UNK_DATA)
	{
		// Check Timeout Flag
		if(I2CTimeout())
			rep = EEPROM_ERR_TIMEOUT;
		else
			rep = EEPROM_ERR_NO;
	}	
	// restore old address register size
	I2CAddrSizeSet(oldAddrSize);
	
	I2CStop();
	
	int cc = _rwDelayEeprom;
	
	// If previous operations completed without errors,
	// we have to poll EEPROM until it replies to understand 
	// when the write sequence is finished
	if(rep == EEPROM_ERR_NO)
	{
		while (cc > 0)
		{
			Delay10us(10);
			cc--;
			BOOL ackReceived = I2CGetDevAck(_deviceAddress);
			if(ackReceived == TRUE)
			{
				// quit from while loop...
				cc = -2;
			}
		}
	}
	if(cc != -2)
		rep = EEPROM_ERR_POLLING;
	
	return rep;
}

BYTE EepromLoadData(WORD addr, void* dataToRd, WORD dataLen, BYTE dataType)
{
	int count = _maxEepromSize - addr - (dataLen*dataType);
	// No more space available
	if(count < 0)
		return EEPROM_ERR_NO_SPACE;
	
	// Save current address register size
	BYTE oldAddrSize = I2CAddrSizeGet();
	// Set 2 BYTEs address register size
	I2CAddrSizeSet(2);
	
	// Change eeprom Address
	if(_eepromChangeAddr(addr) == FALSE)
		return FALSE;
	
	BOOL report = I2CRestart();			//Restart

	//	Writing sequence terminated, check on restart error
	if (!report)
		return FALSE;
	BYTE devAddr = _deviceAddress << 1;
	I2CWrite(devAddr | 0x01); 		//Initiate read sequence to read the registers
	// Simone #warning rimosso delay: Delay10us(_rwDelayEeprom);		
	
	
	BYTE rep = EEPROM_ERR_NO;
	switch(dataType)
	{
		case EEPROM_BYTE:
			vTaskSuspendAll();
			_eepromReceiveBytes(dataToRd, dataLen);			
			xTaskResumeAll();
			break;
			
		case EEPROM_WORD:
			vTaskSuspendAll();
			_eepromReceiveWords(dataToRd, dataLen);			
			xTaskResumeAll();
			break;
			
		case EEPROM_DWORD:
			vTaskSuspendAll();
			_eepromReceiveDwords(dataToRd, dataLen);			
			xTaskResumeAll();
			break;
			
		default: 
			// restore old address register size
			I2CAddrSizeSet(oldAddrSize);
			rep = EEPROM_ERR_UNK_DATA;
			break;
	}
	
	// Check report
	if(rep != EEPROM_ERR_UNK_DATA)
	{
		// Check Timeout Flag
		if(I2CTimeout())
			rep = EEPROM_ERR_TIMEOUT;
		else
			rep = EEPROM_ERR_NO;
	}	
	// restore old address register size
	I2CAddrSizeSet(oldAddrSize);
	
	I2CStop();
	
	return rep;
}

BYTE EepromEraseData(WORD addr, WORD dataLen, BYTE dataType)
{
	int count = _maxEepromSize - addr - (dataLen*dataType);
	// No more space available
	if(count < 0)
		return EEPROM_ERR_NO_SPACE;
	
	// Save current address register size
	BYTE oldAddrSize = I2CAddrSizeGet();
	// Set 2 BYTEs address register size
	I2CAddrSizeSet(2);
	
	if(_eepromChangeAddr(addr) == FALSE)
		return FALSE;
	
	
	BYTE rep = EEPROM_ERR_NO;
	switch(dataType)
	{
		case EEPROM_BYTE:
		case EEPROM_WORD:
		case EEPROM_DWORD:
			for(count = 0; count < (dataLen*dataType); count++)
			{
				vTaskSuspendAll();
				I2CWrite(0xFF);
				xTaskResumeAll();
				// Check Timeout Flag
				if(I2CTimeout())
				{
					// restore old address register size
					I2CAddrSizeSet(oldAddrSize);
					rep = EEPROM_ERR_TIMEOUT;
				}
			}
			rep = EEPROM_ERR_NO;
			break;
			
		default: 
			// restore old address register size
			I2CAddrSizeSet(oldAddrSize);
			break;
	}
	// Check report
	if(rep != EEPROM_ERR_UNK_DATA)
	{
		// Check Timeout Flag
		if(I2CTimeout())
			rep = EEPROM_ERR_TIMEOUT;
		else
			rep = EEPROM_ERR_NO;
	}	
	
	// restore old address register size
	I2CAddrSizeSet(oldAddrSize);
	
	I2CStop();
	
	int cc = _rwDelayEeprom;
	
	// If previous operations completed without errors,
	// we have to poll EEPROM until it replies to understand 
	// when the write sequence is finished
	if(rep == EEPROM_ERR_NO)
	{
		while (cc > 0)
		{
			Delay10us(10);
			cc--;
			BOOL ackReceived = I2CGetDevAck(_deviceAddress);
			if(ackReceived == TRUE)
			{
				// quit from while loop...
				cc = -2;
			}
		}
	}
	if(cc != -2)
		rep = EEPROM_ERR_POLLING;
	
	return rep;
}

// String Storage Functions
BYTE EepromSaveString(WORD addr, char* strToWr, WORD strLen)
{
	return EepromSaveData(addr, strToWr, strLen, EEPROM_BYTE);
}

BYTE EepromLoadString(WORD addr, char* strToRd, WORD strLen)
{
	return EepromLoadData(addr, strToRd, strLen, EEPROM_BYTE);
}

BYTE EepromEraseString(WORD addr, WORD strLen)
{
	return EepromEraseData(addr, strLen, EEPROM_BYTE);
}
