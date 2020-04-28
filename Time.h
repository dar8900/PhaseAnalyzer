#ifndef TIME_H
#define TIME_H
#include <Arduino.h>
#include <RTClib.h>

typedef struct 
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint32_t liveCnt;
	uint32_t timeInUnixTime;
	bool 	rtcStarted;
	bool 	rtcConnected;
}TIME_VAR;

extern TIME_VAR Time;
extern String TimeStr;
extern String DateStr;
extern const uint8_t Day4Month[];
extern DateTime RtcTimeDate;

void RtcInit();
String TimeStamp2String(uint32_t TimeStamp);
void GetTime();
void SetTime(uint8_t NewHour, uint8_t NewMinute);
void SetDate(uint8_t NewDay, uint8_t NewMonth, uint8_t NewYear);

#endif