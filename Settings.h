#ifndef SETTINGS_H
#define SETTINGS_H

typedef enum
{
	LOG_MEASURE_TYPE = 0,
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
	OVER_CURRENT_VALUE,               //  5
	ENABLE_OVER_CURRENT,              //  6
	UNDER_CURRENT_VALUE,              //  7
	ENABLE_UNDER_CURRENT,             //  8
	OVER_ATT_POWER_VALUE,             //  9
	ENABLE_OVER_PATT_POWER,           // 10
	UNDER_ATT_POWER_VALUE,            // 11
	ENABLE_UNDER_PATT_POWER,          // 12
	ALARM_INSERT_DELAY,				  // 13
	AVG_TIME,                         // 14
	SIMULATION_MODE,                  // 15
	MAX_SETTINGS                      // 16
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