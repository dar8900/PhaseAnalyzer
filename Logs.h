#ifndef LOGS_H
#define LOGS_H

#include <Arduino.h>

#define MAX_LOGS_MEMORY		       1024
#define LOGS_START_ADDR		       1000  // +1024 bytes = 128 log -> 2024
#define LAST_LOG_ADDR		       2030  // + 2 bytes -> 2032
#define LOG_FULL_ADDR		       2033  // + 1 bytes -> 2034

#define LOG_SIZE				      8

#define MAX_LOGS			       (MAX_LOGS_MEMORY / LOG_SIZE)

#define MAX_DAILY_ENERGIES		             30
#define DAILY_ENERGIES_ADDR		            270 // +(30 * 4) bytes -> 389
#define DAILY_EN_TIMESTMP_ADDR				390 // +(30 * 4) bytes -> 509
#define LAST_DAILY_ENERGY_INDEX_ADDR	    510 // + 1 -> 511
#define APPARENT_ENERGY_SAVE_ADDR			512 // +8 -> 519
#define ACTIVE_ENERGY_SAVE_ADDR				520 // +8 -> 527
#define REACTIVE_ENERGY_SAVE_ADDR			528 // +8 -> 535

#define APPARENT_ENERGY_F1_SAVE_ADDR		536 // +4 -> 539
#define APPARENT_ENERGY_F2_SAVE_ADDR		540 // +4 -> 543
#define APPARENT_ENERGY_F3_SAVE_ADDR		544 // +4 -> 547

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
extern DAILY_ENERGIES_T DailyEnergies;
extern uint16_t LastLogIndex;
extern uint8_t MeasureToLog;
extern bool EnableLog;
extern bool EnableDailyEnergies;
extern bool LogFull;
extern const double *Measures4Log[];

void LogMeasure();
void ReadAllLogs();
void ResetLogs();
void SaveEnergies();
void ResetSavedEnergies();
void ReadSavedEnergies();
void SaveDailyEnergies();
void ReadDailyEnergies();
void ResetDailyEnergies();
#endif