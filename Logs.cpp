#include "PhaseAnalyzer.h"
#include "Settings.h"
#include "Logs.h"
#include "Time.h"
#include "EepromAnalyzer.h"
#include "Measures.h"



LOGS_DEF LogBuffer[MAX_LOGS];
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
	// EEPROM.put((LastLogIndex * LOG_SIZE) + LOGS_START_ADDR, LogBuffer[LastLogIndex]);
	LastLogIndex++;
	if(LastLogIndex >= MAX_LOGS)
	{
		LastLogIndex = 0;
		LogFull = true;
		// EEPROM.update(LOG_FULL_ADDR, 1);
	}
	// EEPROM.put(LAST_LOG_ADDR, LastLogIndex);
}

void ReadAllLogs()
{
	EEPROM.get(LAST_LOG_ADDR, LastLogIndex);
	for(int LogIndex = 0; LogIndex < MAX_LOGS; LogIndex++)
	{
		EEPROM.get((LogIndex * LOG_SIZE) + LOGS_START_ADDR, LogBuffer[LogIndex]);
	}
	LogFull = (bool)EEPROM.read(LOG_FULL_ADDR);
}

void ResetLogs()
{
	EEPROM.put(LAST_LOG_ADDR, 0);
	LOGS_DEF DeleteValue;
	memset(&DeleteValue, 0x00, LOG_SIZE);
	for(int LogIndex = 0; LogIndex < MAX_LOGS; LogIndex++)
	{
		EEPROM.put((LogIndex * LOG_SIZE) + LOGS_START_ADDR, DeleteValue);
	}
	LogFull = false;
	EEPROM.update(LOG_FULL_ADDR, 0);
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