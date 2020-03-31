#include "PhaseAnalyzer.h"
#include "Settings.h"
#include "EepromAnalyzer.h"
#include <EEPROM.h>
#include "Display.h"

#define MAX_SETTINGS_MEMORY 	256	 // Max 64 settaggi
#define MAX_LOGS_MEMORY		   1024

#define SETTINGS_START_ADDR	      0
#define SETTINGS_CHECKSUM_ADDR	513  // +4 bytes -> 517

#define ENERGIES_ADDR			260 // +24 bytes -> 284

#define RESET_DFLT_ADDR			550 // +1 bytes -> 551

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
	{
		if(ReadResetDflt())
			SettingToDefault();
		ReadAllSettings();
	}
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
			CheckSum += (uint32_t)SettingsVals[i];
		else
			CheckSum += (uint32_t)NewVal;
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
				CheckSum += (uint32_t)SettingsVals[i];
			}
			else
			{
				EEPROM.put(i * sizeof(int32_t), SettingsDefVal[i]);
				CheckSum += (uint32_t)SettingsDefVal[i];				
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
	{
		SettingToDefault();
		ReadAllSettings();
	}
	for(int settingIndex = 0; settingIndex < MAX_SETTINGS; settingIndex++)
	{
		if(Settings[settingIndex].type == ENUM_TYPE)
		{
			switch(Settings[settingIndex].enumPtr[0].enumType)
			{
				case BOOLEAN_TYPE:
					*(bool *)Settings[settingIndex].enumPtr[0].enumValuePtr = (bool)SettingsVals[settingIndex];
					break;
				case LOG_MEASURE_TYPE:
				default:
					break;
			}
		}
	}
}


void WriteResetDeflt()
{
	EEPROM.write(RESET_DFLT_ADDR, 1);
}

bool ReadResetDflt()
{
	bool isReset = false;
	if(EEPROM.read(RESET_DFLT_ADDR) == 1)
	{
		isReset = true;
		EEPROM.write(RESET_DFLT_ADDR, 0);
	}
	else
		isReset = false;
	return isReset;
}