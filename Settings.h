#ifndef SETTINGS_H
#define SETTINGS_H

typedef enum
{
	UINT8_TYPE = 0,
	BOOLEAN_TYPE,
	MAX_ENUM_TYPE
}ENUM_TAB_TYPE;


typedef struct
{
	void *enumValuePtr;
	char *enumName;
	uint8_t enumType;
}ENUM_VAL;

typedef enum
{
	DISABLED = 0,
	ENABLED,
	MAX_ENUM_ENABLE
}ENUM_ENABLE;


typedef enum
{
	CHANGE_TIME = 0,                  //  0
	CHANGE_DATE,                      //  1
	SET_LOG_TIME,                     //  2
	SET_LOG_MEASURE,                  //  3
	ENABLE_LOG_MEASURE,               //  4
	ENABLE_LOG_DAILY_ENERGY,		  //  5     
	OVER_CURRENT_VALUE,               //  6
	ENABLE_OVER_CURRENT,              //  7
	UNDER_CURRENT_VALUE,              //  8
	ENABLE_UNDER_CURRENT,             //  9
	OVER_APP_POWER_VALUE,             // 10
	ENABLE_OVER_PAPP_POWER,           // 11
	UNDER_APP_POWER_VALUE,            // 12
	ENABLE_UNDER_PAPP_POWER,          // 13
	SET_SWITCH_ALARM,				  // 14
	SWITCH_ALARM_ENABLE,			  // 15
	ALARM_INSERT_DELAY,				  // 16
	AVG_TIME,                         // 17
	SIMULATION_MODE,                  // 18
	MAX_SETTINGS                      // 19
}SETTINGS_LIST;




typedef enum
{
	VALUE_TYPE = 0,
	ENUM_TYPE,
	DATE_TYPE,
	MAX_TYPE
}SETTING_TYPE;

typedef struct
{
	int32_t *settingVal;
	int32_t settingMin;
	int32_t settingMax;
	uint8_t type;
	ENUM_VAL *enumPtr;
	char *udm;
	char *settingName;
}SETTING_DEF;

extern int32_t SettingsVals[];
extern const int32_t SettingsDefVal[];
extern const SETTING_DEF Settings[];
extern const ENUM_VAL AlarmSwitchdEnum[];

#endif