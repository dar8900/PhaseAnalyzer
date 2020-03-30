#include "PhaseAnalyzer.h"
#include "Settings.h"
#include "EepromAnalyzer.h"
#include <EEPROM.h>
#include "Display.h"

#define MAX_SETTINGS_MEMORY 	512
#define MAX_LOGS_MEMORY		   1024

#define SETTINGS_START_ADDR	      0
#define SETTINGS_CHECKSUM_ADDR	513  // +4 bytes -> 517

#define LOGS_START_ADDR		   1024
#define LOGS_CHECKSUM_ADDR	   1018 // +4 bytes -> 1022


bool InitMemory()
{
	bool isEmpty = false;
	for(int i = 0; i < EEPROM.length(); i++)
	{
		if(EEPROM.read(i) != 0xff)
		{
			isEmpty = false;
			break;
		}
		else
			isEmpty = true;
	}
	if(isEmpty)
	{
		for(int i = 0; i < EEPROM.length(); i++)
		{
			EEPROM.write(i, 0x00);
			delay(1);
		}	
		SettingToDefault();
		ReadAllSettings();
	}
	else
		ReadAllSettings();
	return isEmpty;
}

bool SettingToDefault()
{
	WriteAllSettings(true);	
	DrawPopUp("Settings a dflt.", 2000);
	return true;
}


void WriteSetting(uint8_t SettingsIndex, int32_t NewVal)
{
	uint32_t CheckSum = 0;
	EEPROM.put(SettingsIndex * sizeof(int32_t), NewVal);
	for(int i = 0; i < MAX_SETTINGS; i++)
	{
		if(i != SettingsIndex)
			CheckSum += SettingsVals[i];
		else
			CheckSum += NewVal;
	}
	EEPROM.put(SETTINGS_CHECKSUM_ADDR, CheckSum);
}

void ReadSetting(uint8_t SettingsIndex, int32_t *StoreVal)
{
	int32_t MemoryVal = 0;
	EEPROM.put(SettingsIndex * sizeof(int32_t), MemoryVal);
	*StoreVal = MemoryVal;
}

void WriteAllSettings(bool toDflt)
{
	uint32_t CheckSum = 0;
	for(int i = 0; i < MAX_SETTINGS; i++)
	{ 
		if(i * sizeof(int32_t) < MAX_SETTINGS_MEMORY)
		{
			if(!toDflt)
			{
				EEPROM.put(i * sizeof(int32_t), SettingsVals[i]);
				CheckSum += SettingsVals[i];
			}
			else
			{
				EEPROM.put(i * sizeof(int32_t), SettingsDefVal[i]);
				CheckSum += SettingsDefVal[i];				
			}		
		}
		delay(1);
	}
	EEPROM.put(SETTINGS_CHECKSUM_ADDR, CheckSum);
}

void ReadAllSettings()
{
	uint32_t CheckSumReaded = 0, CheckSumSaved = 0;
	for(int i = 0; i < MAX_SETTINGS; i++)
	{ 
		EEPROM.get(i * sizeof(int32_t), SettingsVals[i]);
		CheckSumReaded += SettingsVals[i];
	}	
	EEPROM.get(SETTINGS_CHECKSUM_ADDR, CheckSumSaved);
	if(CheckSumReaded != CheckSumSaved)
		SettingToDefault();
}