#include "PhaseAnalyzer.h"
#include "Alarms.h"
#include "Measures.h"
#include "Display.h"
#include "Time.h"
#include "Settings.h"

#define ALARM_DELAY		(SettingsVals[ALARM_INSERT_DELAY] * 1000)

ALARM_DEF AlarmsTab[MAX_ALARMS] = 
{
	{&Current.actual, &SettingsVals[OVER_CURRENT_VALUE]			, 0, 0, 0, 0, 0, 0, false, false, false, "Corrente alta"},
	{&Current.actual, &SettingsVals[UNDER_CURRENT_VALUE]		, 0, 0, 0, 0, 0, 0, false, false, false, "Corrente bassa"},
	{&PApp.actual   , &SettingsVals[ENABLE_OVER_PAPP_POWER]		, 0, 0, 0, 0, 0, 0, false, false, false, "P.apparente alta"},
	{&PApp.actual   , &SettingsVals[ENABLE_UNDER_PAPP_POWER]	, 0, 0, 0, 0, 0, 0, false, false, false, "P.apparente bassa"},
};

Chrono DelayAlarm[MAX_ALARMS];

void CheckAlarms()
{
	if(((*AlarmsTab[OVER_CURRENT_ALARM].alarmMeasure * 1000.0) > (double)*AlarmsTab[OVER_CURRENT_ALARM].threshold) && AlarmsTab[OVER_CURRENT_ALARM].isEnabled)
	{
		// DBG("Alarm: " + String(*AlarmsTab[OVER_CURRENT_ALARM].threshold));
		if(!AlarmsTab[OVER_CURRENT_ALARM].isActive && DelayAlarm[OVER_CURRENT_ALARM].hasPassed(ALARM_DELAY))
		{
			AlarmsTab[OVER_CURRENT_ALARM].isActive = true;
			AlarmsTab[OVER_CURRENT_ALARM].alarmHour = Time.hour;
			AlarmsTab[OVER_CURRENT_ALARM].alarmMinute = Time.minute;
			AlarmsTab[OVER_CURRENT_ALARM].alarmDay = Time.day;
			AlarmsTab[OVER_CURRENT_ALARM].alarmMonth = Time.month;
			AlarmsTab[OVER_CURRENT_ALARM].alarmYear = Time.year % 100;
			AlarmsTab[OVER_CURRENT_ALARM].nOccurence++;
		}

	}
	else
	{
		AlarmsTab[OVER_CURRENT_ALARM].isActive = false;
		DelayAlarm[OVER_CURRENT_ALARM].restart();
	}
	
	if(((*AlarmsTab[UNDER_CURRENT_ALARM].alarmMeasure * 1000.0) < (double)*AlarmsTab[UNDER_CURRENT_ALARM].threshold) && AlarmsTab[UNDER_CURRENT_ALARM].isEnabled)
	{
		if(!AlarmsTab[UNDER_CURRENT_ALARM].isActive && DelayAlarm[UNDER_CURRENT_ALARM].hasPassed(ALARM_DELAY))
		{
			AlarmsTab[UNDER_CURRENT_ALARM].isActive = true;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmHour = Time.hour;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmMinute = Time.minute;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmDay = Time.day;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmMonth = Time.month;
			AlarmsTab[UNDER_CURRENT_ALARM].alarmYear = Time.year % 100;
			AlarmsTab[UNDER_CURRENT_ALARM].nOccurence++;
		}

	}
	else
	{
		AlarmsTab[UNDER_CURRENT_ALARM].isActive = false;
		DelayAlarm[UNDER_CURRENT_ALARM].restart();
	}
	
	if(*AlarmsTab[OVER_PAPP_ALARM].alarmMeasure > (double)*AlarmsTab[OVER_PAPP_ALARM].threshold && AlarmsTab[OVER_PAPP_ALARM].isEnabled)
	{
		if(!AlarmsTab[OVER_PAPP_ALARM].isActive && DelayAlarm[OVER_PAPP_ALARM].hasPassed(ALARM_DELAY))
		{
			AlarmsTab[OVER_PAPP_ALARM].isActive = true;
			AlarmsTab[OVER_PAPP_ALARM].alarmHour = Time.hour;
			AlarmsTab[OVER_PAPP_ALARM].alarmMinute = Time.minute;
			AlarmsTab[OVER_PAPP_ALARM].alarmDay = Time.day;
			AlarmsTab[OVER_PAPP_ALARM].alarmMonth = Time.month;
			AlarmsTab[OVER_PAPP_ALARM].alarmYear = Time.year % 100;
			AlarmsTab[OVER_PAPP_ALARM].nOccurence++;
		}
	}
	else
	{
		AlarmsTab[OVER_PAPP_ALARM].isActive = false;
		DelayAlarm[OVER_PAPP_ALARM].restart();
	}
	
	if(*AlarmsTab[UNDER_PAPP_ALARM].alarmMeasure < (double)*AlarmsTab[UNDER_PAPP_ALARM].threshold && AlarmsTab[UNDER_PAPP_ALARM].isEnabled)
	{
		if(!AlarmsTab[UNDER_PAPP_ALARM].isActive && DelayAlarm[UNDER_PAPP_ALARM].hasPassed(ALARM_DELAY))
		{
			AlarmsTab[UNDER_PAPP_ALARM].isActive = true;
			AlarmsTab[UNDER_PAPP_ALARM].alarmHour = Time.hour;
			AlarmsTab[UNDER_PAPP_ALARM].alarmMinute = Time.minute;
			AlarmsTab[UNDER_PAPP_ALARM].alarmDay = Time.day;
			AlarmsTab[UNDER_PAPP_ALARM].alarmMonth = Time.month;
			AlarmsTab[UNDER_PAPP_ALARM].alarmYear = Time.year % 100;
			AlarmsTab[UNDER_PAPP_ALARM].nOccurence++;
		}
	}
	else
	{
		AlarmsTab[UNDER_PAPP_ALARM].isActive = false;
		DelayAlarm[UNDER_PAPP_ALARM].restart();
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