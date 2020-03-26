#include "PhaseAnalyzer.h"
#include "Display.h"

uint8_t AnalyzerPage = MAIN_MENU;

void setup()
{
	Serial.begin(9600);
	DisplaySetup(D_LANDSCAPE_2);
}

void loop()
{
	switch(AnalyzerPage)
	{
		case MAIN_MENU:
			DrawMainMenu();
			break;
		case MEASURE:
			break;
		case GRAPHICS:
			break;
		case LOGS:
			break;
		case SETTINGS:
			break;
		default:
			break;
	}
}
