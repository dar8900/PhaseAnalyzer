#include "PhaseAnalyzer.h"
#include "Settings.h"
#include "EepromAnalyzer.h"
#include "Display.h"
#include "Rele.h"
#include "Settings.h"

Chrono WriteSwitchStateTimer;
Chrono WriteSwitchStatisticsTimer(Chrono::SECONDS);

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
	// for(int settingIndex = 0; settingIndex < MAX_SETTINGS; settingIndex++)
	// {
	// 	DBG(SettingsVals[settingIndex]);
	// }
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
	CheckSum += MAX_SETTINGS;
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
	CheckSum += MAX_SETTINGS;
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
	CheckSumReaded += MAX_SETTINGS;
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
				case UINT8_TYPE:
					*(uint8_t *)Settings[settingIndex].enumPtr[0].enumValuePtr = (uint8_t)SettingsVals[settingIndex];
					break;
				default:
					break;
			}
		}
		else if(Settings[settingIndex].type == VALUE_TYPE)
		{
			*Settings[settingIndex].settingVal = SettingsVals[settingIndex];
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

void WriteSwitchState()
{
	bool OldState = false;
	if(WriteSwitchStateTimer.hasPassed(500, true))
	{
		EEPROM.get(SWITCH_STATE_ADDR, OldState);
		if(Switch.isActive != OldState && Switch.saveSwitchState)
		{
			EEPROM.put(SWITCH_STATE_ADDR, Switch.isActive);
		}
	}
}

void ReadSwitchState()
{
	if(Switch.saveSwitchState)
		EEPROM.get(SWITCH_STATE_ADDR, Switch.isActive);
}

void WriteSwitchStatistics(bool IsAReset)
{
	if(WriteSwitchStatisticsTimer.hasPassed(900, true))
	{
		EEPROM.put(SWITCH_STATISTICS_START_ADDR, Switch.powerOnTime);
		EEPROM.put(SWITCH_STATISTICS_START_ADDR + 4, Switch.nSwitch);
	}
	if(IsAReset)
	{
		EEPROM.put(SWITCH_STATISTICS_START_ADDR, Switch.powerOnTime);
		EEPROM.put(SWITCH_STATISTICS_START_ADDR + 4, Switch.nSwitch);		
	}
}

void ReadSwitchStatistics()
{
	EEPROM.get(SWITCH_STATISTICS_START_ADDR, Switch.powerOnTime);
	EEPROM.get(SWITCH_STATISTICS_START_ADDR + 4, Switch.nSwitch);	
}
