#ifndef LOGS_H
#define LOGS_H

#include <Arduino.h>

#define MAX_LOGS_MEMORY		       1024
#define LOGS_START_ADDR		       1000  // +1024 bytes = 128 log -> 2024
#define LAST_LOG_ADDR		       2030  // + 2 bytes -> 2032
#define LOG_FULL_ADDR		       2033  // + 1 bytes -> 2034

#define LOG_SIZE				      8

#define MAX_LOGS			       (MAX_LOGS_MEMORY / LOG_SIZE)

#define MAX_DAILY_ENERGIES		     30
#define DAILY_ENERGIES_ADDR		    270 // +(30 * 8) bytes -> 510
#define LAST_DAILY_ENERGY_INDEX	    510 // + 1 -> 511

#pragma pack(1)
typedef struct
{
	float logMeasure;
	uint32_t timeStamp;
}LOGS_DEF;
#pragma pack()

#pragma pack(1)
typedef struct
{
	float dailyAppEn[MAX_DAILY_ENERGIES];
	uint32_t timeStamp[MAX_DAILY_ENERGIES];
	uint8_t lastDailyEnergyIndex;
}DAILY_ENERGIES_T;
#pragma pack()


typedef enum
{
	CURRENT_LOG = 0,
	VOLTAGE_LOG,
	P_ATT_LOG,
	PREA_LOG,
	P_APP_LOG,
	PF_LOG,
	MAX_MEASURE_LOG
}LOGS_MEASURES;

extern LOGS_DEF LogBuffer[];
extern uint16_t LastLogIndex;
extern uint8_t MeasureToLog;
extern bool EnableLog;
extern bool LogFull;
extern const double *Measures4Log[];

void LogMeasure();
void ReadAllLogs();
void ResetLogs();

#endif