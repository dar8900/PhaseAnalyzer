#include "PhaseAnalyzer.h"
#include "Settings.h"
#include "Logs.h"
#include "Time.h"
#include "EepromAnalyzer.h"
#include "Measures.h"

#undef WRITE_IN_EEPROM	

LOGS_DEF LogBuffer[MAX_LOGS];
DAILY_ENERGIES_T DailyEnergies;
uint16_t LastLogIndex;
uint8_t MeasureToLog;
bool EnableLog;
bool LogFull;
Chrono LogTimer(Chrono::SECONDS);


const double *Measures4Log[MAX_MEASURE_LOG] = 
{
	&Current.actual,
	&Voltage.actual,
	&PAtt.actual,	
	&PRea.actual,	
	&PApp.actual,	
	&Pf.actual,			
};

static void WriteSingleLog()
{
	memset(&LogBuffer[LastLogIndex], 0x00, LOG_SIZE);
	LogBuffer[LastLogIndex].logMeasure = (float)*Measures4Log[MeasureToLog];
	LogBuffer[LastLogIndex].timeStamp = RtcTimeDate.unixtime();
#ifdef WRITE_IN_EEPROM	
	EEPROM.put((LastLogIndex * LOG_SIZE) + LOGS_START_ADDR, LogBuffer[LastLogIndex]);
#endif
	LastLogIndex++;
	if(LastLogIndex >= MAX_LOGS)
	{
		LastLogIndex = 0;
		LogFull = true;
	#ifdef WRITE_IN_EEPROM		
		EEPROM.update(LOG_FULL_ADDR, 1);
	#endif
	}
#ifdef WRITE_IN_EEPROM	
	EEPROM.put(LAST_LOG_ADDR, LastLogIndex);
#endif	
}

void ReadAllLogs()
{
#ifdef WRITE_IN_EEPROM	
	EEPROM.get(LAST_LOG_ADDR, LastLogIndex);
	for(int LogIndex = 0; LogIndex < MAX_LOGS; LogIndex++)
	{
		EEPROM.get((LogIndex * LOG_SIZE) + LOGS_START_ADDR, LogBuffer[LogIndex]);
	}
	LogFull = (bool)EEPROM.read(LOG_FULL_ADDR);
#endif	
}

void ResetLogs()
{
#ifdef WRITE_IN_EEPROM	
	EEPROM.put(LAST_LOG_ADDR, 0);
#endif	
	LOGS_DEF DeleteValue;
	memset(&DeleteValue, 0x00, LOG_SIZE);
	for(int LogIndex = 0; LogIndex < MAX_LOGS; LogIndex++)
	{
		LogBuffer[LogIndex] = DeleteValue;
	#ifdef WRITE_IN_EEPROM		
		EEPROM.put((LogIndex * LOG_SIZE) + LOGS_START_ADDR, DeleteValue);
	#endif
	}
	LogFull = false;
#ifdef WRITE_IN_EEPROM	
	EEPROM.update(LOG_FULL_ADDR, 0);
#endif	
	ReadAllLogs();
}

void LogMeasure()
{
	if(EnableLog)
	{
		if(LogTimer.hasPassed(SettingsVals[SET_LOG_TIME], true))
		{
			WriteSingleLog();
			// for(int i = 0; i < MAX_LOGS; i++)
			// {
				// DBG("Misura log " + String(i) + ": " + String(LogBuffer[i].logMeasure, 3) + " Timestamp: " + String(LogBuffer[i].timeStamp));
			// }
			// DBG("Log index: " + String(LastLogIndex));
			// DBG("STOP");
		}
	}
	else
	{
		LogTimer.restart();
	}
}

void SaveDailyEnergies()
{
	if(Time.rtcStarted)
	{
		
	}
}

void ReadDailyEnergies()
{
	uint16_t DaysJumped = 0;
	if(Time.rtcStarted)
	{
		EEPROM.get(DAILY_ENERGIES_ADDR, DailyEnergies);
		if(DailyEnergies.lastDailyEnergyIndex != 0)
		{
			DaysJumped = (Time.timeInUnixTime - DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex - 1]) / 86400;
			if(DaysJumped != 0)
			{
				if(DailyEnergies.lastDailyEnergyIndex + DaysJumped < MAX_DAILY_ENERGIES)
				{
					DailyEnergies.lastDailyEnergyIndex += DaysJumped;
					for(int i = 0; i < DaysJumped; i++)
						DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex + i] = 0;
				}
				else
				{
					memset(DailyEnergies, 0x00, sizeof(DAILY_ENERGIES_T));
					EEPROM.put(DAILY_ENERGIES_ADDR, DailyEnergies);
				}
			}
		}
		else
		{
			DaysJumped = (Time.timeInUnixTime - DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex]) / 86400;
			if(DaysJumped != 0)
			{
				if(DailyEnergies.lastDailyEnergyIndex + DaysJumped + 1 < MAX_DAILY_ENERGIES)
				{
					DailyEnergies.lastDailyEnergyIndex += (DaysJumped + 1);
					for(int i = 0; i < DaysJumped; i++)
						DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex + 1 + i] = 0;
				}
				else
				{
					memset(DailyEnergies, 0x00, sizeof(DAILY_ENERGIES_T));
					EEPROM.put(DAILY_ENERGIES_ADDR, DailyEnergies);
				}
			}			
		}
	}
}