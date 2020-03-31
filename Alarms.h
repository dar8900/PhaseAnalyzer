#ifndef ALARMS_H
#define ALARMS_H
#include <Arduino.h>


typedef struct
{
	double *alarmMeasure;
	int32_t threshold;
	uint8_t alarmHour;
	uint8_t alarmMinute;
	uint8_t alarmDay;
	uint8_t alarmMonth;
	uint8_t alarmYear;
	bool    isEnabled;
	bool 	isActive;
	bool 	isNotified;
	String  alarmName;
}ALARM_DEF;

extern ALARM_DEF AlarmsTab[];

void CheckAlarms();
bool AlarmPresence();

#endif