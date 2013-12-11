#include "taskFlyport.h"
#include "eeprom_helper.h"

void FlyportTask()
{
	_dbgwrite("Flyport LITE running...\r\n");
	_dbgwrite("eeprom_helper test\n");
	
	BYTE eepromAddr = 0b1010000; // this is 7 bit address format, you can use also EEPROM_ADDR_DEF
	
	
	EepromInit(eepromAddr, HIGH_SPEED, 8192); // => 24LC64=EEPROM_SIZE_DEF = 64*128 = 8192;
	EepromSetPageSize(32); // use "EepromSetPageSize(32);" for 24LC64
	
	while(1)
	{
		vTaskDelay(25);
		#ifdef FLYPORT_LITE
		PowerLed(toggle);
		#else
		IOPut(p21, toggle);
		#endif
		
		if (UARTBufferSize(1))
		{
			char u_buf[100];
			UARTRead(1, u_buf, 1);
			UARTFlush(1);
						
			// eeprom_helper
			if(u_buf[0] == 's')
			{
				UARTWrite(1, "testing eeprom string helper\n");
				
				EepromSaveString(0, "prova eeprom", strlen("prova eeprom"));
				
				EepromLoadString(0, u_buf, 10);
				u_buf[10] = '\0';
				UARTWrite(1, u_buf);
				UARTWrite(1, "\r\n");
			}
			
			else if(u_buf[0] == 'b')
			{
				#define B_MAX_LEN	25
				#define B_START_ADDRESS 100
				
				_dbgwrite("BYTE type data on Eeprom\n");
				BYTE data[B_MAX_LEN], data2[B_MAX_LEN];
				
				int aa;
				for (aa = 0; aa < B_MAX_LEN; aa++)
				{
					data[aa] = aa;
					sprintf(u_buf, "data[%d]=%X\n", aa, data[aa]);
					_dbgwrite(u_buf);
				}
				EepromSaveData(B_START_ADDRESS, &data, B_MAX_LEN, EEPROM_BYTE);
				
				EepromLoadData(B_START_ADDRESS, &data2, B_MAX_LEN, EEPROM_BYTE);
				
				for(aa = 0; aa < B_MAX_LEN; aa++)
				{
					sprintf(u_buf, "data2[%d]=%X\n", aa, data2[aa]);
					_dbgwrite(u_buf);
				}
			}
			
			else if(u_buf[0] == 'w')
			{
				#define W_MAX_LEN	25
				#define W_START_ADDRESS 100
				
				_dbgwrite("WORD type data on Eeprom\n");
				static WORD data[W_MAX_LEN];
				
				unsigned int aa;
				for (aa = 0; aa < W_MAX_LEN; aa++)
				{
					data[aa] = aa;
					sprintf(u_buf, "data[%d]=%X\n", aa, data[aa]);
					_dbgwrite(u_buf);
				}
				_dbgwrite("***\n");
				EepromSaveData(W_START_ADDRESS, &data, W_MAX_LEN, EEPROM_WORD);
				
				for (aa = 0; aa < W_MAX_LEN; aa++)
				{
					data[aa] = 0;
				}
				
				EepromLoadData(W_START_ADDRESS, &data, W_MAX_LEN, EEPROM_WORD);
				
				for(aa = 0; aa < W_MAX_LEN; aa++)
				{
					sprintf(u_buf, "data[%d]=%X\n", aa, data[aa]);
					_dbgwrite(u_buf);
				}
			}
			
			else if(u_buf[0] == 'd')
			{
				#define MAX_LEN	15
				#define START_ADDRESS 0
				
				_dbgwrite("DWORD type data on Eeprom\n");
				DWORD data[MAX_LEN], data2[MAX_LEN];
				
				int aa;
				for (aa = 0; aa < MAX_LEN; aa++)
				{
					data[aa] = aa;//(aa+1)+280;
					sprintf(u_buf, "data[%d]=%.0f\n", aa, (double)data[aa]);
					_dbgwrite(u_buf);
				}
				BYTE result = EepromSaveData(START_ADDRESS, &data, MAX_LEN, EEPROM_DWORD);
				if(result == EEPROM_ERR_NO)
				{
					EepromLoadData(START_ADDRESS, &data2, MAX_LEN, EEPROM_DWORD);
				
					for(aa = 0; aa < MAX_LEN; aa++)
					{
						sprintf(u_buf, "data2[%d]=%.0f\n", aa, (double)data2[aa]);
						_dbgwrite(u_buf);
					}
				}
				else
				{
					sprintf(u_buf, "Error:%d\n", result);
					_dbgwrite(u_buf);
				}		
			}
			
			// eeprom_helper
			else if(u_buf[0] == 'e')
			{
				UARTWrite(1, "erasing eeprom helper\n");
				
				EepromEraseData(12, 12, EEPROM_BYTE);
				
				UARTWrite(1, "erase done!\r\n");
			}
			
			if(I2CTimeout())
				UARTWrite(1, "Timeout occurred!\n");
		}	
	}
}

