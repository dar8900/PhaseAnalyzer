#include "PhaseAnalyzer.h"
#include "Alarms.h"
#include "Measures.h"
#include "Display.h"
#include "Time.h"
#include "Settings.h"

#define ALARM_DELAY		(SettingsVals[ALARM_INSERT_DELAY] * 1000)

ALARM_DEF AlarmsTab[MAX_ALARMS] = 
{
	{&Current.actual, SettingsVals[OVER_CURRENT_VALUE]		, 0, 0, 0, 0, 0, false, false, false, "Corrente alta"},
	{&Current.actual, SettingsVals[UNDER_CURRENT_VALUE]		, 0, 0, 0, 0, 0, false, false, false, "Corrente bassa"},
	{&PAtt.actual   , SettingsVals[ENABLE_OVER_PATT_POWER]	, 0, 0, 0, 0, 0, false, false, false, "P.attiva alta"},
	{&PAtt.actual   , SettingsVals[ENABLE_UNDER_PATT_POWER]	, 0, 0, 0, 0, 0, false, false, false, "P.attiva bassa"},
};

Chrono DelayAlarm[MAX_ALARMS];

void CheckAlarms()
{
	if(*(double*)AlarmsTab[OVER_CURRENT_ALARM].alarmMeasure > (double)AlarmsTab[OVER_CURRENT_ALARM].threshold && AlarmsTab[OVER_CURRENT_ALARM].isEnabled)
	{
		if(!AlarmsTab[OVER_CURRENT_ALARM].isActive && DelayAlarm[OVER_CURRENT_ALARM].hasPassed(ALARM_DELAY))
		{
			AlarmsTab[OVER_CURRENT_ALARM].isActive = true;
			AlarmsTab[OVER_CURRENT_ALARM].alarmHour = Time.hour;
			AlarmsTab[OVER_CURRENT_ALARM].alarmMinute = Time.minute;
			AlarmsTab[OVER_CURRENT_ALARM].alarmDay = Time.day;
			AlarmsTab[OVER_CURRENT_ALARM].alarmMonth = Time.month;
			AlarmsTab[OVER_CURRENT_ALARM].alarmYear = Time.year % 100;
		}

	}
	else
	{
		AlarmsTab[OVER_CURRENT_ALARM].isActive = false;
		DelayAlarm[OVER_CURRENT_ALARM].restart();
	}
	
	if(*(double*)AlarmsTab[UNDER_CURRENT_ALARM].alarmMeasure < (double)AlarmsTab[UNDER_CURRENT_ALARM].threshold && AlarmsTab[UNDER_CURRENT_ALARM].isEnabled)
	{
		if(!AlarmsTab[UNDER_CURRENT_ALARM].isActive && DelayAlarm[UNDER_CURRENT_ALARM].hasPassed(ALARM_DELAY))
		{
			AlarmsTab[UNDER_CURRENT_ALARM].isActive = true;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmHour = Time.hour;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmMinute = Time.minute;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmDay = Time.day;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmMonth = Time.month;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmYear = Time.year % 100;
		}

	}
	else
	{
		AlarmsTab[UNDER_CURRENT_ALARM].isActive = false;
		DelayAlarm[UNDER_CURRENT_ALARM].restart();
	}
	
	if(*(double*)AlarmsTab[OVER_PATT_ALARM].alarmMeasure > (double)AlarmsTab[OVER_PATT_ALARM].threshold && AlarmsTab[OVER_PATT_ALARM].isEnabled)
	{
		if(!AlarmsTab[OVER_PATT_ALARM].isActive && DelayAlarm[OVER_PATT_ALARM].hasPassed(ALARM_DELAY))
		{
			AlarmsTab[OVER_PATT_ALARM].isActive = true;
			AlarmsTab[OVER_PATT_ALARM].alarmHour = Time.hour;
			AlarmsTab[OVER_PATT_ALARM].alarmMinute = Time.minute;
			AlarmsTab[OVER_PATT_ALARM].alarmDay = Time.day;
			AlarmsTab[OVER_PATT_ALARM].alarmMonth = Time.month;
			AlarmsTab[OVER_PATT_ALARM].alarmYear = Time.year % 100;
		}
	}
	else
	{
		AlarmsTab[OVER_PATT_ALARM].isActive = false;
		DelayAlarm[OVER_PATT_ALARM].restart();
	}
	
	if(*(double*)AlarmsTab[UNDER_PATT_ALARM].alarmMeasure < (double)AlarmsTab[UNDER_PATT_ALARM].threshold && AlarmsTab[UNDER_PATT_ALARM].isEnabled)
	{
		if(!AlarmsTab[UNDER_PATT_ALARM].isActive && DelayAlarm[UNDER_PATT_ALARM].hasPassed(ALARM_DELAY))
		{
			AlarmsTab[UNDER_PATT_ALARM].isActive = true;
			AlarmsTab[UNDER_PATT_ALARM].alarmHour = Time.hour;
			AlarmsTab[UNDER_PATT_ALARM].alarmMinute = Time.minute;
			AlarmsTab[UNDER_PATT_ALARM].alarmDay = Time.day;
			AlarmsTab[UNDER_PATT_ALARM].alarmMonth = Time.month;
			AlarmsTab[UNDER_PATT_ALARM].alarmYear = Time.year % 100;
		}
	}
	else
	{
		AlarmsTab[UNDER_PATT_ALARM].isActive = false;
		DelayAlarm[UNDER_PATT_ALARM].restart();
	}
	
	for(int Alarm = 0; Alarm < MAX_ALARMS; Alarm++)
	{
		if(AlarmsTab[Alarm].isActive && !AlarmsTab[Alarm].isNotified)
		{
			DrawAlarmPopUp(AlarmsTab[Alarm].alarmName.c_str(), 3000);
			AlarmsTab[Alarm].isNotified = true;
		}
		else if(!AlarmsTab[Alarm].isActive)	
			AlarmsTab[Alarm].isNotified = false;
	}

}

bool AlarmPresence()
{
	bool almostOneActive = false;
	for(int Alarm = 0; Alarm < MAX_ALARMS; Alarm++)
	{
		if(AlarmsTab[Alarm].isActive)
		{
			almostOneActive = true;
			break;
		}
	}
	return almostOneActive;
}