#ifndef EEPROM_ANALYZER_H
#define EEPROM_ANALYZER_H

#include <Arduino.h>
#include <EEPROM.h>

bool InitMemory();
bool SettingToDefault();
void WriteSetting(uint8_t SettingsIndex, int32_t NewVal);
void ReadSetting(uint8_t SettingsIndex, int32_t *StoreVal);
void WriteAllSettings(bool toDflt);
void ReadAllSettings();
void WriteResetDeflt();
bool ReadResetDflt();
#endif