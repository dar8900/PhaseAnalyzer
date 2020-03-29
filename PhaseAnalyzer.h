#ifndef PHASE_ANALYZER_H
#define PHASE_ANALYZER_H

#include <Arduino.h>
#include <stdint.h>
#include <Chrono.h>

#define DEBUG

#ifdef DEBUG

#define DBG(msg)	(Serial.println(msg))

#else

#define DBG(msg)

#endif

#define MIN_TO_SEC(min)			(min * 60)

typedef enum
{
	MAIN_MENU = 0,
	MEASURE,
	GRAPHICS,
	LOGS,
	ALARMS,
	SETTINGS,
	MAX_ANALYZER_PAGES
}ANALYZER_PAGES;

typedef enum
{
	MEASURE_ITEM = 0,
	GRAPHICS_ITEM,
	LOGS_ITEM,
	ALARM_ITEM,
	SETTINGS_ITEM,
	MAX_MENU_ITEMS
}MENU_ITEMS;

typedef enum
{
	V_I_PF = 0,
	PATT_PREA_PAPP,
	V_I_PF_AVG,
	PATT_PREA_PAPP_AVG,
	ENATT_ENREA_ENAPP,
	LIVE_CNT,
	MAX_MEASURE_PAGES
}MEASURE_PAGES;

typedef enum
{
	WAVEFORM_I = 0,
	WAVEFORM_V,
	MAX_GRAPHICS_PAGES
}GRAPHIC_PAGES;

typedef enum
{
	LOG_LIST = 0,
	LOG_GRAPHIC,
	MAX_LOGS_PAGES
}LOGS_PAGES;

typedef enum
{
	SET_TIME = 0,
	SET_DATE,
	MEASURE_LOG,
	PERIOD_LOG,
	ENABLE_LOG,
	AVG_MEASURE_SAMPLE_TIME,
	MAX_SETTINGS_PAGES
}SETTINGS_PAGES;

typedef struct
{
	bool overCurrentEnabled;
	bool underCurrentEnabled;
	bool overPAttEnabled;
	bool underPAttEnabled;
}ALARMS_FLAG;


extern uint8_t AnalyzerPage;

#endif