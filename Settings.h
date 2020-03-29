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
	CURRENT_LOG = 0,
	VOLTAGE_LOG,
	P_ATT_LOG,
	PREA_LOG,
	P_APP_LOG,
	PF_LOG,
	MAX_MEASURE_LOG
}LOGS_MEASURES;

typedef enum
{
	CHANGE_TIME = 0,
	CHANGE_DATE,
	SET_LOG_TIME,
	SET_LOG_MEASURE,
	ENABLE_LOG_MEASURE,
	OVER_CURRENT_VALUE,
	ENABLE_OVER_CURRENT,
	UNDER_CURRENT_VALUE,
	ENABLE_UNDER_CURRENT,
	OVER_ATT_POWER_VALUE,
	ENABLE_OVER_PATT_POWER,
	UNDER_ATT_POWER_VALUE,
	ENABLE_UNDER_PATT_POWER,
	AVG_TIME,
	SIMULATION_MODE,
	MAX_SETTINGS
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
extern const SETTING_DEF Settings[];

#endif