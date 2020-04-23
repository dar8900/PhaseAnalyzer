#ifndef RELE_H
#define RELE_H

#include <Arduino.h>

#define SWITCH_PIN	5



typedef struct
{
	bool isActive;
	bool haveTimer;
	bool haveAlarm;
	bool alarmShutDown;
	uint32_t powerOnTime;
	uint16_t nSwitch;
	uint32_t timerDuration;
	uint8_t  associatedAlarm;
}RELE_DEF;

extern RELE_DEF Switch;

void SwitchInit();
void RefreshSwitchStatus();

#endif