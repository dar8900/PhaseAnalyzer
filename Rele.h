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

typedef enum
{
	NONE = 0,
	OVER_CURRENT,
	OVER_P_APP,
	MAX_ALARMS_SWITCH
}ALARMS_SWITCH;

extern RELE_DEF Switch;

void SwitchInit();
void RefreshSwitchStatus();
void ResetSwitchStatistics();

#endif