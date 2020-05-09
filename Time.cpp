#include "PhaseAnalyzer.h"
#include "Time.h"


#define DFLT_HOUR  		0
#define DFLT_MINUTE 	0
#define DFLT_DAY  		1
#define DFLT_MONTH  	1
#define DFLT_YEAR  		2020

static RTC_DS1307 rtc;
DateTime RtcTimeDate;

uint8_t BandHour;

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
		Time.rtcConnected = false;
		Time.rtcStarted = false;
	}
	else
		Time.rtcConnected = true;
	if(Time.rtcConnected)
	{
		if (! rtc.isrunning())
		{
			// rtc.adjust(DateTime(DFLT_YEAR, DFLT_MONTH, DFLT_DAY, DFLT_HOUR, DFLT_MINUTE, 0));
			Time.rtcStarted = false;
		}
		else
		{
			Time.rtcStarted = true;
			GetTime();
		}
	}
}

String TimeStamp2String(uint32_t TimeStamp, uint8_t WichInfo)
{
	String TimeString = "";
	DateTime TimeRaw(TimeStamp);
	switch(WichInfo)
	{
		case ONLY_TIME:
			TimeString = (TimeRaw.hour() < 10 ? "0" + String(TimeRaw.hour()) : String(TimeRaw.hour()));
			TimeString += ":" + (TimeRaw.minute() < 10 ? "0" + String(TimeRaw.minute()) : String(TimeRaw.minute()));
			TimeString += ":" + (TimeRaw.second() < 10 ? "0" + String(TimeRaw.second()) : String(TimeRaw.second()));			
			break;
		case ONLY_DATE:
			TimeString = (TimeRaw.day() < 10 ? "0" + String(TimeRaw.day()) : String(TimeRaw.day()));
			TimeString += "/" + (TimeRaw.month() < 10 ? "0" + String(TimeRaw.month()) : String(TimeRaw.month()));
			TimeString += "/" + String(TimeRaw.year() % 100);		
			break;
		case BOTH:
			TimeString = (TimeRaw.hour() < 10 ? "0" + String(TimeRaw.hour()) : String(TimeRaw.hour()));
			TimeString += ":" + (TimeRaw.minute() < 10 ? "0" + String(TimeRaw.minute()) : String(TimeRaw.minute()));
			TimeString += ":" + (TimeRaw.second() < 10 ? "0" + String(TimeRaw.second()) : String(TimeRaw.second()));
			TimeString += "  " + (TimeRaw.day() < 10 ? "0" + String(TimeRaw.day()) : String(TimeRaw.day()));
			TimeString += "/" + (TimeRaw.month() < 10 ? "0" + String(TimeRaw.month()) : String(TimeRaw.month()));
			TimeString += "/" + String(TimeRaw.year() % 100);		
			break;
		default:
			break;
	}

	return TimeString;
}

static void CalcBend()
{
	if(Time.weekDay >= LUNEDI && Time.weekDay <= VENERDI)
	{
		if((Time.day == 1 && Time.month == 1) || (Time.day == 6 && Time.month == 1) ||
			(Time.day == 25 && Time.month == 4) || (Time.day == 1 && Time.month == 5) ||
			(Time.day == 2 && Time.month == 6) || (Time.day == 15 && Time.month == 8) ||
			(Time.day == 1 && Time.month == 11) || (Time.day == 8 && Time.month == 12) ||
			(Time.day == 25 && Time.month == 12) || (Time.day == 26 && Time.month == 12))
			{
				BandHour = F3;
			}
		else
		{
			if((Time.hour >= 7 && Time.hour < 8) || (Time.hour >= 19 && Time.hour <= 23))
			{
				BandHour = F2;
			}
			else if((Time.hour >= 8 && Time.hour < 19))
			{
				BandHour = F1;
			}
			else
			{
				BandHour = F3;
			}
		}
	}
	else if(Time.weekDay == SABATO)
	{
		if((Time.day == 1 && Time.month == 1) || (Time.day == 6 && Time.month == 1) ||
			(Time.day == 25 && Time.month == 4) || (Time.day == 1 && Time.month == 5) ||
			(Time.day == 2 && Time.month == 6) || (Time.day == 15 && Time.month == 8) ||
			(Time.day == 1 && Time.month == 11) || (Time.day == 8 && Time.month == 12) ||
			(Time.day == 25 && Time.month == 12) || (Time.day == 26 && Time.month == 12))
			{
				BandHour = F3;
			}
		else
		{
			if(Time.hour >= 7 && Time.hour < 23)
			{
				BandHour = F2;
			}
			else
			{
				BandHour = F3;
			}
		}		
	}
	else
	{
		BandHour = F3;
	}
}

void GetTime()
{
	if(Time.rtcStarted)
	{
		RtcTimeDate = rtc.now();
		Time.weekDay = RtcTimeDate.dayOfTheWeek();
		Time.timeInUnixTime = RtcTimeDate.unixtime();
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
		CalcBend();
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