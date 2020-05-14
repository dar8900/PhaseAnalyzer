#ifndef PHASE_ANALYZER_H
#define PHASE_ANALYZER_H

#include <Arduino.h>
#include <stdint.h>
#include <Chrono.h>

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

#define DEBUG

#ifdef DEBUG

#define DBG(msg)	(Serial.println(msg))

#else

#define DBG(msg)

#endif

#define MIN_TO_SEC(min)			(min * 60)

#define FW_VERSION	1.2

typedef enum
{
	MAIN_MENU = 0,
	MEASURE,
	GRAPHICS,
	LOGS,
	ALARMS,
	RELE,
	SETTINGS,
	RESETS,
	LIST_LOG,
	GRAPHIC_LOG,
	DAILY_ENERGIES_LOG,
	SET_TIME_DATE,
	MAX_ANALYZER_PAGES
}ANALYZER_PAGES;

typedef enum
{
	MEASURE_ITEM = 0,
	GRAPHICS_ITEM,
	LOGS_ITEM,
	ALARM_ITEM,
	RELE_STATUS,
	SETTINGS_ITEM,
	RESET_ITEM,
	MAX_MENU_ITEMS
}MENU_ITEMS;

typedef enum
{
	V_I_PF = 0,
	PATT_PREA_PAPP,
	V_I_PF_AVG,
	PATT_PREA_PAPP_AVG,
	ENATT_ENREA_ENAPP,
	ENAPPF1_ENAPPF2_ENAPPF3,
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
	LOG_DAILY_ENERGIES,
	MAX_LOGS_PAGES
}LOGS_PAGES;

typedef enum
{
	OVER_CURRENT_ALARM = 0,
	UNDER_CURRENT_ALARM,
	OVER_PAPP_ALARM,
	UNDER_PAPP_ALARM,
	MAX_ALARMS
}ALARMS_PAGES;


typedef enum
{
	STATUS = 0,
	SET_TIMER,
	STATISTICS,
	MAX_RELE_ITEM
}RELE_PAGES;

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

typedef enum
{
	RESET_DFLT = 0,
	RESTART,
	RESET_MAX_MIN,
	RESET_AVG,
	RESET_ENERGIES,
	RESET_LOG,
	RESET_DAILY_ENERGIES,
	RESET_SWITCH_STAT,
	MAX_RESETS
}RESET_PAGES;


extern uint8_t AnalyzerPage;

#endif
