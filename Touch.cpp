#include "Touch.h"


static bool WasLongTouched;


static void ReScaleDisplayCoord(TS_Point p)
{
	if(p.x < DisplayParam.min_analog_x || p.x > 3900)
	{
		if(p.x < DisplayParam.min_analog_x)
			DisplayParam.display_x = 0;
		else
			DisplayParam.display_x = DisplayParam.max_x;
	}
	else
	{
		DisplayParam.display_x = (p.x * DisplayParam.max_x) / 3900;
	}
	if(p.y < DisplayParam.min_analog_y || p.y > 3900)
	{
		if(p.y < DisplayParam.min_analog_y)
			DisplayParam.display_y = 0;
		else
			DisplayParam.display_y = DisplayParam.max_y;
	}
	else
	{
		DisplayParam.display_y = (p.y * DisplayParam.max_y) / 3900;
	}	
}

void CheckTouch()
{
	uint32_t TouchTime = 0;
	bool LongTouch = false;
	if(Touch.touched())
	{
		TouchTime = millis();
		while(Touch.touched() == true)
		{
			ReScaleDisplayCoord(Touch.getPoint());
			if(millis() - TouchTime >= 500)
			{
				LongTouch = true;
				break;
			}
		}
		if(LongTouch)
		{
			DisplayParam.isLongTouched = true;
			WasLongTouched = true;
		}
		else
		{
			if(!WasLongTouched)
				DisplayParam.isTouched = true;
			else
				WasLongTouched = false;
		}
		delay(25);
	}
}



