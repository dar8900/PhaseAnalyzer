#ifndef LOGS_H
#define LOGS_H

#include <Arduino.h>

#define MAX_LOGS_MEMORY		   1024
#define LOGS_START_ADDR		   1000  // +1024 bytes = 128 log -> 2024
#define LAST_LOG_ADDR		   2030  // + 2 bytes -> 2032
#define LOG_FULL_ADDR		   2033  // + 1 bytes -> 2034

#define LOG_SIZE				  8

#define MAX_LOGS			  (MAX_LOGS_MEMORY / LOG_SIZE)

#pragma pack(1)
typedef struct
{
	float logMeasure;
	uint32_t timeStamp;
}LOGS_DEF;
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