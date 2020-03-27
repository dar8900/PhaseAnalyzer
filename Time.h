#ifndef TIME_H
#define TIME_H
#include <Arduino.h>

typedef struct 
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint32_t liveCnt;
	bool 	rtcStarted;
}TIME_VAR;

extern TIME_VAR Time;
extern String TimeStr;
extern String DateStr;

void RtcInit();
void GetTime();

#endif