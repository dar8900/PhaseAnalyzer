#include "Display.h"


void setup()
{
	Serial.begin(9600);
	DisplaySetup(D_LANDSCAPE_2);
}

void loop()
{
	TestDisplayTouch();
	delay(50);
}
