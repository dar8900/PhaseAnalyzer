#include "PhaseAnalyzer.h"
#include "Rele.h"
#include "Time.h"
#include "Alarms.h"
#include "Display.h"

RELE_DEF Switch;
static bool IsStillActive = false;
static Chrono SwitchPowerOnTimer, SwitchTimer(Chrono::SECONDS);

void SwitchInit()
{
	pinMode(SWITCH_PIN, OUTPUT);
}


void RefreshSwitchStatus()
{
	if(Switch.isActive)
	{
		if(!IsStillActive)
		{
			digitalWrite(SWITCH_PIN, HIGH);
			IsStillActive = true;
			Switch.nSwitch++;
			SwitchPowerOnTimer.restart();
		}
		if(Switch.haveAlarm)
		{
			bool AlarmActive = false;
			switch(Switch.associatedAlarm)
			{
				case NONE:
					AlarmActive = false;
					break;
				case OVER_CURRENT:
					AlarmActive = AlarmsTab[OVER_CURRENT_ALARM].isActive;
					break;
				case OVER_P_APP:
					AlarmActive = AlarmsTab[OVER_PAPP_ALARM].isActive;
					break;
				default:
					AlarmActive = false;
					break;
			}
			if(AlarmActive)
			{
				Switch.isActive = false;
				Switch.haveTimer = false;
				Switch.timerDuration = 0;
				Switch.alarmShutDown = true;
			}
			else
			{
				Switch.alarmShutDown = false;
			}
		}
		else
		{
			Switch.haveAlarm = false;
			Switch.alarmShutDown = false;
		}
		if(SwitchPowerOnTimer.hasPassed(1000, true))
			Switch.powerOnTime++;
	}
	else
	{
		if(IsStillActive)
		{
			digitalWrite(SWITCH_PIN, LOW);
			IsStillActive = false;
			SwitchPowerOnTimer.stop();
		}		
	}
	if(Switch.haveTimer)
	{
		if(SwitchTimer.hasPassed(Switch.timerDuration))
		{
			if(Switch.isActive)
			{
				Switch.isActive = false;
				DrawInfoPopUp("Presa spenta", 1000);
			}
			else
			{
				Switch.isActive = true;
				DrawInfoPopUp("Presa accesa", 1000);
			}
			Switch.timerDuration = 0;
			Switch.haveTimer = false;
		}
	}
	else
		SwitchTimer.restart();
}

void ResetSwitchStatistics()
{
	Switch.powerOnTime = 0;
	Switch.nSwitch = 0;
}