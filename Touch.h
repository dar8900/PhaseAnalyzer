#ifndef TOUCH_H
#define TOUCH_H

#include "Display.h"

typedef enum
{
	UP = 0,
	DOWN,
	OK,
	BACK,
	MAX_KEY
}KEYS;



void CheckTouch(void);


#endif