#include "PhaseAnalyzer.h"
#include "Display.h"
#include "Measures.h"
#include "Time.h"
#include "EepromAnalyzer.h"



uint8_t AnalyzerPage = MAIN_MENU;

void setup()
{
	Serial.begin(9600);
	DisplaySetup(D_LANDSCAPE_2);
	AnalogInit();
	RtcInit();
	InitMemory();
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
			DrawLogsList();
			break;		
		default:
			break;
	}
}
