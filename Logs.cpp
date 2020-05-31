#include "PhaseAnalyzer.h"
#include "Settings.h"
#include "Logs.h"
#include "Time.h"
#include "EepromAnalyzer.h"
#include "Measures.h"

#define WRITE_IN_EEPROM	

LOGS_DEF LogBuffer[MAX_LOGS];
DAILY_ENERGIES_T DailyEnergies;
uint16_t LastLogIndex;
uint8_t MeasureToLog;
uint8_t DailyEnergiesSaveHour = 0;
bool EnableLog;
bool EnableDailyEnergies;
bool NotRewriteDailyEnergies = false;
bool LogFull;
Chrono LogTimer(Chrono::SECONDS), SaveApparentEnergyTimer(Chrono::SECONDS);


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

void SaveEnergies()
{
	if(SaveApparentEnergyTimer.hasPassed(SettingsVals[ENEGIES_SAVE_TIME] * 60, true))
	{
		// DBG("Energie salvate");
		EEPROM.put(APPARENT_ENERGY_SAVE_ADDR, EnApp.actual);
		EEPROM.put(ACTIVE_ENERGY_SAVE_ADDR, EnAtt.actual);
		EEPROM.put(REACTIVE_ENERGY_SAVE_ADDR, EnRea.actual);
		EEPROM.put(APPARENT_ENERGY_F1_SAVE_ADDR, (float)EnAppF1.actual);
		EEPROM.put(APPARENT_ENERGY_F2_SAVE_ADDR, (float)EnAppF2.actual);
		EEPROM.put(APPARENT_ENERGY_F3_SAVE_ADDR, (float)EnAppF3.actual);		
	}
}

void ResetSavedEnergies()
{
	EEPROM.put(APPARENT_ENERGY_SAVE_ADDR, EnApp.actual);
	EEPROM.put(ACTIVE_ENERGY_SAVE_ADDR, EnAtt.actual);
	EEPROM.put(REACTIVE_ENERGY_SAVE_ADDR, EnRea.actual);
	EEPROM.put(APPARENT_ENERGY_F1_SAVE_ADDR, (float)EnAppF1.actual);
	EEPROM.put(APPARENT_ENERGY_F2_SAVE_ADDR, (float)EnAppF2.actual);
	EEPROM.put(APPARENT_ENERGY_F3_SAVE_ADDR, (float)EnAppF3.actual);
}

void ReadSavedEnergies()
{
	float TmpVal = 0.0;
	EEPROM.get(APPARENT_ENERGY_SAVE_ADDR, EnApp.actual);
	EEPROM.get(ACTIVE_ENERGY_SAVE_ADDR, EnAtt.actual);
	EEPROM.get(REACTIVE_ENERGY_SAVE_ADDR, EnRea.actual);
	
	EEPROM.get(APPARENT_ENERGY_F1_SAVE_ADDR, TmpVal);
	EnAppF1.actual = (double)TmpVal;
	EEPROM.get(APPARENT_ENERGY_F2_SAVE_ADDR, TmpVal);
	EnAppF2.actual = (double)TmpVal;
	EEPROM.get(APPARENT_ENERGY_F3_SAVE_ADDR, TmpVal);
	EnAppF3.actual = (double)TmpVal;
}


void SaveDailyEnergies()
{
	double DailyEnAppCopy = DailyEnApp;
	if(Time.rtcStarted && EnableDailyEnergies)
	{
		if(Time.hour == SettingsVals[SET_SAVE_HOUR_DAILY_EN] && Time.minute == 0 && Time.second == 0 && !NotRewriteDailyEnergies)
		{
			NotRewriteDailyEnergies = true;
			if(DailyEnergies.lastDailyEnergyIndex != 0)
			{
				for(int i = DailyEnergies.lastDailyEnergyIndex; i >= 0; i--)
				{
					if(DailyEnergies.timeStamp[i] != 0)
					{
						DailyEnAppCopy = DailyEnApp - DailyEnergies.dailyAppEn[i];
						if(DailyEnAppCopy < 0)
							DailyEnAppCopy = DailyEnApp;
						break;
					}

				}
			}
			DailyEnergies.dailyAppEn[DailyEnergies.lastDailyEnergyIndex] = (float)DailyEnAppCopy;
			DailyEnApp = 0.0;
			DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex] = Time.timeInUnixTime;
			EEPROM.put(DAILY_ENERGIES_ADDR + (DailyEnergies.lastDailyEnergyIndex * 4), DailyEnergies.dailyAppEn[DailyEnergies.lastDailyEnergyIndex]);
			EEPROM.put(DAILY_EN_TIMESTMP_ADDR + (DailyEnergies.lastDailyEnergyIndex * 4), DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex]);
			DailyEnergies.lastDailyEnergyIndex++;
			EEPROM.put(LAST_DAILY_ENERGY_INDEX_ADDR, DailyEnergies.lastDailyEnergyIndex);
			// DBG("Salvataggio energie giornaliere");
			// #ifdef DEBUG
			// for(int i = 0; i < MAX_DAILY_ENERGIES; i++)
			// {
				// DBG("Energia giorno " + String(i) + ": " + String(DailyEnergies.dailyAppEn[i]));
				// DBG("TimeStamp " + String(i) + ": " + String(DailyEnergies.timeStamp[i]));
			// }
			// DBG("LastDayIndex: " + String(DailyEnergies.lastDailyEnergyIndex));
			// #endif			
		}
		else if(Time.hour == SettingsVals[SET_SAVE_HOUR_DAILY_EN] && Time.minute == 0 && Time.second != 0)
		{
			NotRewriteDailyEnergies = false;
		}
	}
}

void ReadDailyEnergies()
{
	uint32_t DaysJumped = 0;
	if(Time.rtcStarted && EnableDailyEnergies)
	{
		EEPROM.get(DAILY_ENERGIES_ADDR, DailyEnergies);
		// #ifdef DEBUG
		// DBG("Prima della GET per le energie");
		// for(int i = 0; i < MAX_DAILY_ENERGIES; i++)
		// {
			// DBG("Energia giorno " + String(i) + ": " + String(DailyEnergies.dailyAppEn[i]));
			// DBG("TimeStamp giorno " + String(i) + ": " + String(DailyEnergies.timeStamp[i]));
		// }
		// DBG("LastDayIndex: " + String(DailyEnergies.lastDailyEnergyIndex));
		// #endif
		if(DailyEnergies.lastDailyEnergyIndex != 0)
		{
			DaysJumped = (Time.timeInUnixTime - DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex - 1]) / 86400;
			// DBG("Tempo unix: " + String(Time.timeInUnixTime));
			// DBG("TimeStamp salvato: " + String(DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex - 1]));
			// DBG("Giorni saltati: " + String(DaysJumped));
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
					DBG("Reset energie nella read con last day != 0");
					memset(&DailyEnergies, 0x00, sizeof(DAILY_ENERGIES_T));
					EEPROM.put(DAILY_ENERGIES_ADDR, DailyEnergies);
				}
			}
		}
		// else
		// {
			// DaysJumped = (Time.timeInUnixTime - DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex]) / 86400;
			// if(DaysJumped != 0)
			// {
				// DBG("Tempo unix: " + String(Time.timeInUnixTime));
				// DBG("TimeStamp salvato: " + String(DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex]));
				// DBG("Giorni saltati: " + String(DaysJumped));
				// DBG("Indice ultimo giorno:"  + String(DailyEnergies.lastDailyEnergyIndex));
				// if(DailyEnergies.lastDailyEnergyIndex + DaysJumped + 1 < MAX_DAILY_ENERGIES)
				// {
					// DailyEnergies.lastDailyEnergyIndex += (DaysJumped + 1);
					// for(int i = 0; i < DaysJumped; i++)
						// DailyEnergies.timeStamp[DailyEnergies.lastDailyEnergyIndex + 1 + i] = 0;
				// }
				// else
				// {
					// DBG("Reset energie nella read con last day == 0");
					// memset(&DailyEnergies, 0x00, sizeof(DAILY_ENERGIES_T));
					// // EEPROM.put(DAILY_ENERGIES_ADDR, DailyEnergies);
				// }
			// }			
		// }
		// #ifdef DEBUG
		// DBG("Dopo la GET e l'elaborazione per le energie");
		// for(int i = 0; i < MAX_DAILY_ENERGIES; i++)
		// {
			// DBG("Energia giorno " + String(i) + ": " + String(DailyEnergies.dailyAppEn[i]));
			// DBG("TimeStamp giorno " + String(i) + ": " + String(DailyEnergies.timeStamp[i]));
		// }
		// DBG("LastDayIndex: " + String(DailyEnergies.lastDailyEnergyIndex));
		// #endif
	}
}

void ResetDailyEnergies()
{
	memset(&DailyEnergies, 0x00, sizeof(DAILY_ENERGIES_T));
	EEPROM.put(DAILY_ENERGIES_ADDR, DailyEnergies);	
}