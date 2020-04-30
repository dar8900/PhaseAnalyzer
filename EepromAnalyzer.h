#ifndef EEPROM_ANALYZER_H
#define EEPROM_ANALYZER_H

#include <Arduino.h>
#include <EEPROM.h>

#define MAX_SETTINGS_MEMORY 				256	 // Max 64 settaggi
#define SETTINGS_START_ADDR	    			  0
#define SETTINGS_CHECKSUM_ADDR				256  // +4 bytes -> 259

#define SWITCH_STATISTICS_START_ADDR		260  // +6 bytes -> 265
			
#define RESET_DFLT_ADDR						550 // +1 bytes -> 551


bool InitMemory();
bool SettingToDefault();
void WriteSetting(uint8_t SettingsIndex, int32_t NewVal);
void ReadSetting(uint8_t SettingsIndex, int32_t *StoreVal);
void WriteAllSettings(bool toDflt);
void ReadAllSettings();
void WriteResetDeflt();
bool ReadResetDflt();
void WriteSwitchStatistics(bool IsAReset);
void ReadSwitchStatistics();
#endif