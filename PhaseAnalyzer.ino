#include "PhaseAnalyzer.h"
#include "Display.h"
#include "Measures.h"
#include "Time.h"
#include "EepromAnalyzer.h"
#include "Rele.h"
#include "BT.h"
#include "Logs.h"

#pragma message ( "Seriale sotto linux: occhio!!!" )

uint8_t AnalyzerPage = MAIN_MENU;

void setup()
{
	Serial.begin(115200);

	// while(!Serial){}
	
	DisplaySetup(D_LANDSCAPE_1);
	AnalogInit();
	RtcInit();
	if(!Time.rtcStarted && Time.rtcConnected)
		AnalyzerPage = SET_TIME_DATE;
	InitMemory();
	SwitchInit();
	BtInit();
	ReadAllLogs();
	ReadSavedEnergies();
	ReadDailyEnergies();
	ReadSwitchStatistics();
	DBG("Software started");
}

void loop()
{
	switch(AnalyzerPage)
	{
		case MAIN_MENU:
			DrawMainMenu();
			break;
		case MEASURE:
			DrawMeasurePage();
			break;
		case GRAPHICS:
			DrawGraphicsPage();
			break;
		case LOGS:
			DrawLogMenu();
			break;
		case ALARMS:
			DrawAlarmPage();
			break;
		case SETTINGS:
			DrawSettingPage();
			break;
		case RESETS:
			DrawResetPage();
			break;
		case LIST_LOG:
			DrawLogsList();
			break;
		case GRAPHIC_LOG:
			DrawLogGraphic();
			break;	
		case DAILY_ENERGIES_LOG:
			DrawDailyEnergiesHysto();
			break;				
		case RELE:
			DrawRelePage();
			break;
		case SET_TIME_DATE:
			ChangeTimeDate(true, false);
			Time.rtcStarted = true;
			ChangeTimeDate(false, false);
			Time.rtcStarted = true;
			ReadDailyEnergies();
			AnalyzerPage = MAIN_MENU;
			break;
		default:
			break;
	}
}
