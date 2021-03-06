#include "PhaseAnalyzer.h"
#include "Settings.h"
#include "Measures.h"
#include "Alarms.h"
#include "Logs.h"
#include "Rele.h"

int32_t SettingsVals[MAX_SETTINGS];

const int32_t SettingsDefVal[MAX_SETTINGS] = 
{
	0,						// CHANGE_TIME = 0,                   //  0
	0,						// CHANGE_DATE,                       //  1
	900,					// SET_LOG_TIME,                      //  2
	CURRENT_LOG,			// SET_LOG_MEASURE,                   //  3
	DISABLED,				// ENABLE_LOG_MEASURE,                //  4
	ENABLED,				// ENABLE_LOG_DAILY_ENERGY,		      //  5
	0,					    // SET_SAVE_HOUR_DAILY_EN,			  //  6
	DISABLED,				// ENABLE_SAVE_SWITCH_STATE,		  //  7
	10000,					// OVER_CURRENT_VALUE,                //  8
	DISABLED,				// ENABLE_OVER_CURRENT,               //  9
	1000,					// UNDER_CURRENT_VALUE,               // 10
	DISABLED,				// ENABLE_UNDER_CURRENT,              // 11
	2000,					// OVER_APP_POWER_VALUE,              // 12
	DISABLED,				// ENABLE_OVER_PAPP_POWER,            // 13
	500,					// UNDER_APP_POWER_VALUE,             // 14
	DISABLED,				// ENABLE_UNDER_PAPP_POWER,           // 15
	NONE,					// SET_SWITCH_ALARM,				  // 16
	DISABLED,               // SWITCH_ALARM_ENABLE,			      // 17
	5,						// ALARM_INSERT_DELAY,				  // 18
	1,						// AVG_TIME,                          // 19
	15,						// ENEGIES_SAVE_TIME,		  		  // 20
	20,                     // CENT_PER_KVARH,					  // 21
	ENABLED,				// ENABLE_ENERGY_AVG_CALC,			  // 22
	130,                    // TARP_CURRENT,					  // 23
	DISABLED,				// SIMULATION_MODE,                   // 24
};                                                                


const ENUM_VAL LogMeasureEnum[MAX_MEASURE_LOG] = 
{
	{&MeasureToLog, "Corrente"         , UINT8_TYPE},
	{&MeasureToLog, "Tensione"		   , UINT8_TYPE},
	{&MeasureToLog, "Potenza att."     , UINT8_TYPE},
	{&MeasureToLog, "Potenza rea."     , UINT8_TYPE},
	{&MeasureToLog, "Potenza app."     , UINT8_TYPE},
	{&MeasureToLog, "PF"	           , UINT8_TYPE},
};

const ENUM_VAL LogEnable[2] = 
{
	{&EnableLog, "Disabilitato"   , BOOLEAN_TYPE},
	{&EnableLog, "Abilitato"      , BOOLEAN_TYPE},
};

const ENUM_VAL DailyEnergiesLogEnable[2] = 
{
	{&EnableDailyEnergies, "Disabilitato"   , BOOLEAN_TYPE},
	{&EnableDailyEnergies, "Abilitato"      , BOOLEAN_TYPE},
};


const ENUM_VAL OverCurrentEnableEnum[2] = 
{
	{&AlarmsTab[OVER_CURRENT_ALARM].isEnabled, "Disabilitato"   , BOOLEAN_TYPE},
	{&AlarmsTab[OVER_CURRENT_ALARM].isEnabled, "Abilitato"      , BOOLEAN_TYPE},
};

const ENUM_VAL UnderCurrentEnableEnum[2] = 
{
	{&AlarmsTab[UNDER_CURRENT_ALARM].isEnabled, "Disabilitato"   , BOOLEAN_TYPE},
	{&AlarmsTab[UNDER_CURRENT_ALARM].isEnabled, "Abilitato"      , BOOLEAN_TYPE},
};

const ENUM_VAL OverPAppEnableEnum[2] = 
{
	{&AlarmsTab[OVER_PAPP_ALARM].isEnabled, "Disabilitato"   , BOOLEAN_TYPE},
	{&AlarmsTab[OVER_PAPP_ALARM].isEnabled, "Abilitato"      , BOOLEAN_TYPE},
};


const ENUM_VAL UnderPAppEnabledEnum[2] = 
{
	{&AlarmsTab[UNDER_PAPP_ALARM].isEnabled, "Disabilitato"   , BOOLEAN_TYPE},
	{&AlarmsTab[UNDER_PAPP_ALARM].isEnabled, "Abilitato"      , BOOLEAN_TYPE},
};

const ENUM_VAL SimulationEnabledEnum[2] = 
{
	{&simulationMode, "Disabilitato"   , BOOLEAN_TYPE},
	{&simulationMode, "Abilitato"      , BOOLEAN_TYPE},
};


const ENUM_VAL AlarmSwitchdEnum[MAX_ALARMS_SWITCH] = 
{
	{&Switch.associatedAlarm, "Nessuno"   		 		, UINT8_TYPE},
	{&Switch.associatedAlarm, "Sovra corrente"   		, UINT8_TYPE},
	{&Switch.associatedAlarm, "Sovra potenza att."      , UINT8_TYPE},
};

const ENUM_VAL AlarmSwitchEnableEnum[2] = 
{
	{&Switch.haveAlarm, "Disabilitato"   , BOOLEAN_TYPE},
	{&Switch.haveAlarm, "Abilitato"      , BOOLEAN_TYPE},
};

const ENUM_VAL EnableSaveSwitchState[2] = 
{
	{&Switch.saveSwitchState, "Disabilitato"   , BOOLEAN_TYPE},
	{&Switch.saveSwitchState, "Abilitato"      , BOOLEAN_TYPE},
};

const ENUM_VAL EnableCalcEnergyAvgSetting[2] = 
{
	{&EnableCalcEnergyAvg, "Disabilitato"   , BOOLEAN_TYPE},
	{&EnableCalcEnergyAvg, "Abilitato"      , BOOLEAN_TYPE},
};

const SETTING_DEF Settings[MAX_SETTINGS] = 
{
	{&SettingsVals[CHANGE_TIME]   	        	, 0			 		 , 0 						, DATE_TYPE , NULL				         , ""    , "Imposta ora"		         },
	{&SettingsVals[CHANGE_DATE]   	        	, 0			 		 , 0 						, DATE_TYPE , NULL				         , ""    , "Imposta data"		     },
	{&SettingsVals[SET_LOG_TIME]		  		, 1			 		 , 1800 					, VALUE_TYPE, NULL				         , "s"   , "Tempo di log"		     },
	{&SettingsVals[SET_LOG_MEASURE]		    	, CURRENT_LOG		 , MAX_MEASURE_LOG - 1  	, ENUM_TYPE , LogMeasureEnum             , ""    , "Misura di log"		     },
	{&SettingsVals[ENABLE_LOG_MEASURE]    		, DISABLED		     , ENABLED 					, ENUM_TYPE , LogEnable			         , ""    , "Abilita log"			     },
	{&SettingsVals[ENABLE_LOG_DAILY_ENERGY] 	, DISABLED		     , ENABLED 					, ENUM_TYPE , DailyEnergiesLogEnable     , ""    , "Abilita en.app giorn."    },
	{&SettingsVals[SET_SAVE_HOUR_DAILY_EN]		, 0			 		 , 23 				    	, VALUE_TYPE, NULL				         , "h"   , "Ora salvataggio E.G."		     },
	{&SettingsVals[ENABLE_SAVE_SWITCH_STATE] 	, DISABLED		     , ENABLED 					, ENUM_TYPE , EnableSaveSwitchState      , ""    , "Salva stato presa"        },
	{&SettingsVals[OVER_CURRENT_VALUE]      	, 1000		 		 , 15000		 	    	, VALUE_TYPE, NULL				         , "mA"  , "Sovra corrente"		     },
	{&SettingsVals[ENABLE_OVER_CURRENT]     	, DISABLED			 , ENABLED  				, ENUM_TYPE , OverCurrentEnableEnum      , ""    , "Abilita sovra corrente"   },
	{&SettingsVals[UNDER_CURRENT_VALUE]     	, 0					 , 1000  			 	   	, VALUE_TYPE, NULL				         , "mA"  , "Sotto corrente"		     },
	{&SettingsVals[ENABLE_UNDER_CURRENT]    	, DISABLED			 , ENABLED  	 	    	, ENUM_TYPE , UnderCurrentEnableEnum     , ""    , "Abilita sotto corrente"   },
	{&SettingsVals[OVER_APP_POWER_VALUE]    	, 1000	    		 , 3500 		        	, VALUE_TYPE, NULL					     , "W"   , "Sovra potenza app."       },
	{&SettingsVals[ENABLE_OVER_PAPP_POWER]  	, DISABLED			 , ENABLED    	    		, ENUM_TYPE , OverPAppEnableEnum	     , ""    , "Abitlita sovra pot.app."  },
	{&SettingsVals[UNDER_APP_POWER_VALUE]   	, 0					 , 1000 					, VALUE_TYPE, NULL					     , "W"   , "Sotto potenza app."	     },
	{&SettingsVals[ENABLE_UNDER_PAPP_POWER] 	, DISABLED			 , ENABLED    				, ENUM_TYPE , UnderPAppEnabledEnum       , ""    , "Abitlita sotto pot.app."  },
	{&SettingsVals[SET_SWITCH_ALARM] 			, 0			 		 , MAX_ALARMS_SWITCH - 1	, ENUM_TYPE , AlarmSwitchdEnum           , ""    , "Scelta allarme presa"     },
	{&SettingsVals[SWITCH_ALARM_ENABLE] 		, DISABLED			 , ENABLED    				, ENUM_TYPE , AlarmSwitchEnableEnum      , ""    , "Abilita allarme presa"    },
	{&SettingsVals[ALARM_INSERT_DELAY]			, 1					 , 15   					, VALUE_TYPE, NULL					     , "s"   , "Tempo inser. allarme"     },
	{&SettingsVals[AVG_TIME]					, 1					 , 30   					, VALUE_TYPE, NULL					     , "min" , "Tempo di media"		     },
	{&SettingsVals[ENEGIES_SAVE_TIME]	        , 1					 , 30   					, VALUE_TYPE, NULL					     , "min" , "Tempo salvataggio en."    },
	{&SettingsVals[CENT_PER_KVARH]	            , 0					 , 100   					, VALUE_TYPE, NULL					     , "cent", "Euro per kVAH"           },
	{&SettingsVals[ENABLE_ENERGY_AVG_CALC]  	, DISABLED			 , ENABLED    				, ENUM_TYPE , EnableCalcEnergyAvgSetting , ""    , "Calcolo energie/avg"	     },
	{&SettingsVals[TARP_CURRENT]	            , 50			     , 1000   					, VALUE_TYPE, NULL					     , "mA"  , "Corrente di tarpamento"   },
	{&SettingsVals[SIMULATION_MODE]  			, DISABLED			 , ENABLED    				, ENUM_TYPE , SimulationEnabledEnum      , ""    , "Abilita simulazione"	     },
};
