#include "PhaseAnalyzer.h"
#include "Display.h"
#include "Measures.h"

uint8_t AnalyzerPage = MEASURE;

void setup()
{
	Serial.begin(9600);
	DisplaySetup(D_LANDSCAPE_2);
	AnalogInit();
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
		case SETTINGS:
			DrawSettingPage();
			break;
		default:
			break;
	}
}
