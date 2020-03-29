#include "PhaseAnalyzer.h"
#include "Display.h"
#include "Measures.h"
#include "Time.h"

uint8_t AnalyzerPage = SETTINGS;

void setup()
{
	Serial.begin(9600);
	DisplaySetup(D_LANDSCAPE_2);
	AnalogInit();
	RtcInit();
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
			DrawLogsPage();
			break;
		case ALARMS:
			DrawAlarmPage();
			break;
		case SETTINGS:
			DrawSettingPage();
			break;
		default:
			break;
	}
}
