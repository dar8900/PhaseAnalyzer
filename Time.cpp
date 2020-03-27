#include "PhaseAnalyzer.h"
#include "Time.h"
#include "RTClib.h"

static RTC_DS1307 rtc;
static DateTime RtcTimeDate;


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