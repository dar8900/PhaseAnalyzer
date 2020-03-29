#include "PhaseAnalyzer.h"
#include "Time.h"
#include "RTClib.h"

static RTC_DS1307 rtc;
static DateTime RtcTimeDate;

const uint8_t Day4Month[12] = 
{
	31, 
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31,
};

TIME_VAR Time;
String TimeStr;
String DateStr;

void RtcInit()
{
	if (! rtc.begin()) 
	{
		Serial.println("Rtc no started");
		Time.rtcStarted = false;
	}
	else
		Time.rtcStarted = true;
	if(Time.rtcStarted)
	{
		if (! rtc.isrunning())
		{
			rtc.adjust(DateTime(2020, 3, 27, 16, 10, 0)); 
		}
	}
	// rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // DA ELIMINARE 
}

void GetTime()
{
	if(Time.rtcStarted)
	{
		RtcTimeDate = rtc.now();
		Time.hour = RtcTimeDate.hour();
		Time.minute = RtcTimeDate.minute();
		Time.second = RtcTimeDate.second();
		Time.day = RtcTimeDate.day();
		Time.month = RtcTimeDate.month();
		Time.year = RtcTimeDate.year();
		if(Time.hour < 10)
			TimeStr = "0" + String(Time.hour);
		else
			TimeStr = String(Time.hour);
		if(Time.minute < 10)
			TimeStr += ":0" + String(Time.minute);
		else
			TimeStr += ":" + String(Time.minute);
		
		if(Time.day < 10)
			DateStr = "0" + String(Time.day);
		else
			DateStr = String(Time.day);
		if(Time.month < 10)
			DateStr += "/0" + String(Time.month);
		else
			DateStr += "/" + String(Time.month);
		
		DateStr += "/" + String(Time.year);
	}
	else
	{
		TimeStr = "--:--";
		DateStr = "--/--/--";
	}
}

void SetTime(uint8_t NewHour, uint8_t NewMinute)
{
	rtc.adjust(DateTime(Time.year, Time.month, Time.day, NewHour, NewMinute, 0));
}

void SetDate(uint8_t NewDay, uint8_t NewMonth, uint8_t NewYear)
{
	rtc.adjust(DateTime(NewYear + 2000, NewMonth, NewDay, Time.hour, Time.minute, Time.second));
}