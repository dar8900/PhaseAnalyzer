#include "PhaseAnalyzer.h"
#include "Settings.h"
#include "Measures.h"
#include "Alarms.h"
#include "Logs.h"

int32_t SettingsVals[MAX_SETTINGS];

const int32_t SettingsDefVal[MAX_SETTINGS] = 
{
	0,						//CHANGE_TIME,            
	0,						//CHANGE_DATE,                
	900,					//SET_LOG_TIME,               
	CURRENT_LOG,			//SET_LOG_MEASURE,            
	DISABLED,				//ENABLE_LOG_MEASURE,         
	10,						//OVER_CURRENT_VALUE,         
	DISABLED,				//ENABLE_OVER_CURRENT,        
	1,						//UNDER_CURRENT_VALUE,        
	DISABLED,				//ENABLE_UNDER_CURRENT,       
	2000,					//OVER_ATT_POWER_VALUE,       
	DISABLED,				//ENABLE_OVER_PATT_POWER,     
	500,					//UNDER_ATT_POWER_VALUE,      
	DISABLED,				//ENABLE_UNDER_PATT_POWER,   
	5,						//ALARM_INSERT_DELAY
	1,						//AVG_TIME,                   
	DISABLED,				//SIMULATION_MODE,               
};


const ENUM_VAL LogMeasureEnum[MAX_MEASURE_LOG] = 
{
	{&MeasureToLog, "Corrente"         , LOG_MEASURE_TYPE},
	{&MeasureToLog, "Tensione"		   , LOG_MEASURE_TYPE},
	{&MeasureToLog, "Potenza att."     , LOG_MEASURE_TYPE},
	{&MeasureToLog, "Potenza rea."     , LOG_MEASURE_TYPE},
	{&MeasureToLog, "Potenza app."     , LOG_MEASURE_TYPE},
	{&MeasureToLog, "PF"	           , LOG_MEASURE_TYPE},
};

const ENUM_VAL LogEnable[2] = 
{
	{&EnableLog, "Disabilitato"   , BOOLEAN_TYPE},
	{&EnableLog, "Abilitato"      , BOOLEAN_TYPE},
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

const ENUM_VAL OverPAttEnableEnum[2] = 
{
	{&AlarmsTab[OVER_PATT_ALARM].isEnabled, "Disabilitato"   , BOOLEAN_TYPE},
	{&AlarmsTab[OVER_PATT_ALARM].isEnabled, "Abilitato"      , BOOLEAN_TYPE},
};


const ENUM_VAL UnderPAttEnabledEnum[2] = 
{
	{&AlarmsTab[UNDER_PATT_ALARM].isEnabled, "Disabilitato"   , BOOLEAN_TYPE},
	{&AlarmsTab[UNDER_PATT_ALARM].isEnabled, "Abilitato"      , BOOLEAN_TYPE},
};

const ENUM_VAL SimulationEnabledEnum[2] = 
{
	{&simulationMode, "Disabilitato"   , BOOLEAN_TYPE},
	{&simulationMode, "Abilitato"      , BOOLEAN_TYPE},
};


const SETTING_DEF Settings[MAX_SETTINGS] = 
{
	{&SettingsVals[CHANGE_TIME]   	        , 0			 		 , 0 					, DATE_TYPE , NULL				     , ""   , "Imposta ora"		       },
	{&SettingsVals[CHANGE_DATE]   	        , 0			 		 , 0 					, DATE_TYPE , NULL				     , ""   , "Imposta data"		   },
	{&SettingsVals[SET_LOG_TIME]		  	, 1			 		 , 1800 				, VALUE_TYPE, NULL				     , "s"  , "Tempo di log"		   },
	{&SettingsVals[SET_LOG_MEASURE]		    , CURRENT_LOG		 , MAX_MEASURE_LOG - 1  , ENUM_TYPE , LogMeasureEnum         , ""   , "Misura di log"		   },
	{&SettingsVals[ENABLE_LOG_MEASURE]    	, DISABLED		     , ENABLED 				, ENUM_TYPE , LogEnable			     , ""   , "Abilita log"			   },
	{&SettingsVals[OVER_CURRENT_VALUE]      , 1			 		 , 15 			 	    , VALUE_TYPE, NULL				     , "A"  , "Sovra corrente"		   },
	{&SettingsVals[ENABLE_OVER_CURRENT]     , DISABLED			 , ENABLED  			, ENUM_TYPE , OverCurrentEnableEnum  , ""   , "Abilita sovra corrente" },
	{&SettingsVals[UNDER_CURRENT_VALUE]     , 0					 , 1  			 	    , VALUE_TYPE, NULL				     , "A"  , "Sotto corrente"		   },
	{&SettingsVals[ENABLE_UNDER_CURRENT]    , DISABLED			 , ENABLED  	 	    , ENUM_TYPE , UnderCurrentEnableEnum , ""   , "Abilita sotto corrente" },
	{&SettingsVals[OVER_ATT_POWER_VALUE]    , 1000	    		 , 3500 		        , VALUE_TYPE, NULL					 , "W"  , "Sovra potenza att."     },
	{&SettingsVals[ENABLE_OVER_PATT_POWER]  , DISABLED			 , ENABLED    	    	, ENUM_TYPE , OverPAttEnableEnum	 , ""   , "Abitlita sovra pot.att."},
	{&SettingsVals[UNDER_ATT_POWER_VALUE]   , 0					 , 1000 				, VALUE_TYPE, NULL					 , "W"  , "Sotto potenza att."	   },
	{&SettingsVals[ENABLE_UNDER_PATT_POWER] , DISABLED			 , ENABLED    			, ENUM_TYPE , UnderPAttEnabledEnum   , ""   , "Abitlita sotto pot.att."},
	{&SettingsVals[ALARM_INSERT_DELAY]		, 1					 , 15   				, VALUE_TYPE, NULL					 , "s"  , "Tempo inser. allarme"   },
	{&SettingsVals[AVG_TIME]				, 1					 , 30   				, VALUE_TYPE, NULL					 , "min", "Tempo di media"		   },
	{&SettingsVals[SIMULATION_MODE]  		, DISABLED			 , ENABLED    			, ENUM_TYPE , SimulationEnabledEnum  , ""   , "Abilita simulazione"	   },
};