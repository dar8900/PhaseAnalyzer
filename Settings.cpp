#include "PhaseAnalyzer.h"
#include "Settings.h"
#include "Measures.h"

int32_t SettingsVals[MAX_SETTINGS];




ENUM_VAL LogEnum[MAX_MEASURE_LOG] = 
{
	{&Current.actual, "Corrente"         },
	{&Voltage.actual, "Tensione"		 },
	{&PAtt.actual	, "Potenza attiva"   },
	{&PRea.actual	, "Potenza reattiva" },
	{&PApp.actual	, "Potenza apparente"},
	{&Pf.actual		, "PF"	        	 },
};


const SETTING_DEF Settings[MAX_SETTINGS] = 
{
	{&SettingsVals[CHANGE_TIME]   	        , 0			 , 0 					, DATE_TYPE , ""   , "Imposta ora"},
	{&SettingsVals[CHANGE_DATE]   	        , 0			 , 0 					, DATE_TYPE , ""   , "Imposta data"},
	{&SettingsVals[SET_LOG_TIME]			, 1			 , 1800 				, VALUE_TYPE, "s"  , "Tempo di log"},
	{&SettingsVals[SET_LOG_MEASURE]		    , CURRENT_LOG, MAX_MEASURE_LOG - 1  , ENUM_TYPE , ""   , "Misura di log"},
	{&SettingsVals[ENABLE_LOG_MEASURE]    	, 0		     , 1 					, ENUM_TYPE , ""   , "Abilita log"},
	{&SettingsVals[OVER_CURRENT_VALUE]      , 1			 , 15 			 	    , VALUE_TYPE, "A"  , "Sovra corrente"},
	{&SettingsVals[ENABLE_OVER_CURRENT]     , 0			 , 1  			 	    , ENUM_TYPE , ""   , "Abilita sovra corrente"},
	{&SettingsVals[UNDER_CURRENT_VALUE]     , 0			 , 1  			 	    , VALUE_TYPE, "A"  , "Sotto corrente"},
	{&SettingsVals[ENABLE_UNDER_CURRENT]    , 0			 , 1  			 	    , ENUM_TYPE , ""	  , "Abilita sotto corrente"},
	{&SettingsVals[OVER_ATT_POWER_VALUE]    , 1000	     , 3500 		        , VALUE_TYPE, "W"  , "Sovra potenza att."},
	{&SettingsVals[ENABLE_OVER_PATT_POWER]  , 0			 , 1    				, ENUM_TYPE , ""   , "Abitlita sovra pot.att."},
	{&SettingsVals[UNDER_ATT_POWER_VALUE]   , 0			 , 1000 				, VALUE_TYPE, "W"  , "Sotto potenza att."},
	{&SettingsVals[ENABLE_UNDER_PATT_POWER] , 0			 , 1    				, ENUM_TYPE , ""   , "Abitlita sotto pot.att."},
	{&SettingsVals[AVG_TIME]				, 1			 , 30   				, VALUE_TYPE, "min", "Tempo di media"},
};