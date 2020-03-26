#include "PhaseAnalyzer.h"
#include "Display.h"
#include "Touch.h"

#define DRAW_OK		false
#define DRAW_BACK	true


typedef struct
{
	int16_t x0;
	int16_t y0;
	int16_t w;
	int16_t h;
	uint16_t c;
	
}NAV_BUTTON_COORD;


XPT2046_Touchscreen Touch(CS_PIN);
ILI9341_t3 Display = ILI9341_t3(TFT_CS, TFT_DC);
DISPLAY_VAR DisplayParam;

Chrono DisplayRefresh;

NAV_BUTTON_COORD Up = {NAV_BUTT_X_START, NAV_BUTT_Y_START, NAV_BUTT_WIDTH, NAV_BUTT_HIGH, ILI9341_RED};
NAV_BUTTON_COORD Down = {NAV_BUTT_X_START, NAV_BUTT_Y_START + (2 * (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE)), NAV_BUTT_WIDTH, NAV_BUTT_HIGH, ILI9341_RED};
NAV_BUTTON_COORD Ok_Back = {NAV_BUTT_X_START, NAV_BUTT_Y_START + (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE), NAV_BUTT_WIDTH, NAV_BUTT_HIGH, ILI9341_GREEN};
	
const char ArrowDown_bits[] = 
{
	0x01, 0xff, 0x80, 0x1f, 0xff, 0xf8, 0x3f, 0x00, 0xfc, 0x30, 0x00, 0x0c, 
	0x20, 0x00, 0x04, 0x60, 0x00, 0x06, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 
	0xc0, 0x00, 0x03, 0xcc, 0x00, 0x33, 0xc6, 0x00, 0x63, 0xc3, 0x00, 0xc3, 
	0xc1, 0x81, 0x83, 0xc0, 0xc3, 0x03, 0xc0, 0x66, 0x03, 0xc0, 0x3c, 0x03, 
	0xc0, 0x18, 0x03, 0xc0, 0x00, 0x03, 0x60, 0x00, 0x06, 0x20, 0x00, 0x04, 
	0x30, 0x00, 0x0c, 0x3f, 0x00, 0xfc, 0x1f, 0xff, 0xf8, 0x03, 0xff, 0x80

};


const char ArrowUp_bits[] =
{
	0x01, 0xff, 0x80, 0x1f, 0xff, 0xf8, 0x3f, 0x00, 0xfc, 0x30, 0x00, 0x0c, 
	0x20, 0x00, 0x04, 0x60, 0x00, 0x06, 0xc0, 0x00, 0x03, 0xc0, 0x18, 0x03, 
	0xc0, 0x3c, 0x03, 0xc0, 0x66, 0x03, 0xc0, 0xc3, 0x03, 0xc1, 0x81, 0x83, 
	0xc3, 0x00, 0xc3, 0xc6, 0x00, 0x63, 0xcc, 0x00, 0x33, 0xc0, 0x00, 0x03, 
	0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0x60, 0x00, 0x06, 0x20, 0x00, 0x04, 
	0x30, 0x00, 0x0c, 0x3f, 0x00, 0xfc, 0x1f, 0xff, 0xf8, 0x03, 0xff, 0x80
};

const char *MenuVoices[MAX_MENU_ITEMS] = 
{
	"Misure",
	"Grafici",
	"Log",
	"Impostazioni",
};


void DisplaySetRotation(uint8_t Rotation)
{
	Display.setRotation(Rotation);
	switch(Rotation)
	{
		case D_LANDSCAPE_1:
			DisplayParam.max_x = 320;
			DisplayParam.max_y = 240;
			DisplayParam.min_analog_x = 340;
			DisplayParam.min_analog_y = 250;			
			Touch.setRotation(T_LANDSCAPE_2);
			break;
		case D_LANDSCAPE_2:
			DisplayParam.max_x = 320;
			DisplayParam.max_y = 240;	
			DisplayParam.min_analog_x = 340;
			DisplayParam.min_analog_y = 250;	
			Touch.setRotation(T_LANDSCAPE_1);
			break;
		case D_LONG_1:
			DisplayParam.max_x = 240;
			DisplayParam.max_y = 320;
			DisplayParam.min_analog_x = 250;
			DisplayParam.min_analog_y = 340;				
			Touch.setRotation(T_LONG_2);
			break;
		case D_LONG_2:
			DisplayParam.max_x = 240;
			DisplayParam.max_y = 320;
			DisplayParam.min_analog_x = 250;
			DisplayParam.min_analog_y = 340;				
			Touch.setRotation(T_LONG_1);
			break;
		default:
			DisplayParam.max_x = 320;
			DisplayParam.max_y = 240;
			DisplayParam.min_analog_x = 340;
			DisplayParam.min_analog_y = 250;			
			Touch.setRotation(T_LANDSCAPE_2);	
			break;
	}	
} 

void ClearDisplay(bool Inverse)
{
	if(Inverse)
		Display.fillScreen(ILI9341_WHITE);
	else
		Display.fillScreen(ILI9341_BLACK);
}

void DisplaySetup(uint8_t Rotation)
{
	Display.begin();
	ClearDisplay(false);
	Touch.begin();
	DisplaySetRotation(Rotation);
	DisplayParam.displayRotation = Rotation;
}

static void DrawNavButtons(bool DrawBackButt)
{
	uint16_t Color = 0;
	Display.fillRect(Up.x0, Up.y0, Up.w, Up.h, Up.c);
	if(!DrawBackButt)
		Ok_Back.c = ILI9341_GREEN;
	else
		Ok_Back.c = ILI9341_BLUE;
	Display.fillRect(Ok_Back.x0, Ok_Back.y0, Ok_Back.w, Ok_Back.h, Ok_Back.c);
	Display.fillRect(Down.x0, Down.y0, Down.w, Down.h, Down.c);
	
	Display.fillTriangle(NAV_BUTT_X_START + (NAV_BUTT_WIDTH / 2), NAV_BUTT_Y_START + 5, 
						 NAV_BUTT_X_START + (NAV_BUTT_WIDTH / 4), NAV_BUTT_Y_START + 30,
						 NAV_BUTT_X_START + (3 * NAV_BUTT_WIDTH / 4), NAV_BUTT_Y_START + 30, ILI9341_WHITE);
						 
	Display.fillTriangle(NAV_BUTT_X_START + (NAV_BUTT_WIDTH / 2), NAV_BUTT_Y_START + (2 * (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE)) + 33, 
						 NAV_BUTT_X_START + (NAV_BUTT_WIDTH / 4), NAV_BUTT_Y_START + (2 * (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE)) + 8,
						 NAV_BUTT_X_START + (3 * NAV_BUTT_WIDTH / 4), NAV_BUTT_Y_START + (2 * (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE)) + 8, ILI9341_WHITE);
	
	if(!DrawBackButt)
	{
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_10_Bold);
		Display.setCursor(NAV_BUTT_X_START + 10, NAV_BUTT_Y_START + 16 + (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE));	
		Display.print("OK");
	}
	else
	{
		Display.fillTriangle(NAV_BUTT_X_START + 5, NAV_BUTT_Y_START + (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE) + (NAV_BUTT_HIGH / 2), 
						 NAV_BUTT_X_START + 25, NAV_BUTT_Y_START + (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE) + 5,
						 NAV_BUTT_X_START + 25, NAV_BUTT_Y_START + (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE) + 35, ILI9341_WHITE);		
	}

}

static void DrawTopInfo()
{
	Display.setFont(Arial_12);
	Display.setCursor(LEFT_ALIGN, TOP_POS);
	Display.print("--:--");
	Display.setCursor(RIGHT_ALIGN("--/--/--"), TOP_POS);
	Display.print("--/--/--");
}

static uint8_t ButtonPressed()
{
	uint8_t KeyPress = MAX_KEY;
	CheckTouch();
	if(DisplayParam.isTouched)
	{
		DisplayParam.isTouched = false;
		if(DisplayParam.display_x >= NAV_BUTT_X_START && DisplayParam.display_x <= (NAV_BUTT_X_START + NAV_BUTT_WIDTH))
		{
			if(DisplayParam.display_y >= Up.y0 && DisplayParam.display_y <= (Up.y0 + Up.h))
			{
				KeyPress = UP;
			}
			else if(DisplayParam.display_y >= Down.y0 && DisplayParam.display_y <= (Down.y0 + Down.h))
			{
				KeyPress = DOWN;
			}
			else if(DisplayParam.display_y >= Ok_Back.y0 && DisplayParam.display_y <= (Ok_Back.y0 + Ok_Back.h))
			{
				KeyPress = OK;
			}
		}
	}
	else if(DisplayParam.isLongTouched)
	{
		DisplayParam.isLongTouched = false;
		if(DisplayParam.display_x >= NAV_BUTT_X_START && DisplayParam.display_x <= (NAV_BUTT_X_START + NAV_BUTT_WIDTH))
		{
			if(DisplayParam.display_y >= Ok_Back.y0 && DisplayParam.display_y <= (Ok_Back.y0 + Ok_Back.h))
			{
				KeyPress = BACK;
			}
		}
	}
	return KeyPress;
}

void DrawMainMenu()
{
	bool ExitMainMenu = false;
	uint8_t TopItem = 0, Item = 0, OldItem = 0, KeyPress = MAX_KEY;
	while(!ExitMainMenu)
	{
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
			Display.setFont(Arial_24_Bold);
			Display.setTextColor(ILI9341_WHITE);
			Display.setCursor(CENTER_ALIGN("Menu"), MENU_TITLE_POS);
			Display.print("Menu");
			Display.setFont(Arial_18);
			for(int i = 0; i < MAX_MENU_ITEMS; i++)
			{
				int NewItem = TopItem + i;
				if(NewItem >= MAX_MENU_VIEW_ITEMS)
					break;
				if(NewItem == Item)
				{
					Display.setTextColor(ILI9341_GREENYELLOW);
				}
				else
				{
					Display.setTextColor(ILI9341_WHITE);
				}
				Display.setCursor(CENTER_ALIGN(MenuVoices[i]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)));
				Display.print(MenuVoices[i]);
			}
		}
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				if(Item > 0)
					Item--;
				else
					Item = MAX_MENU_ITEMS - 1;
				break;
			case DOWN:
				if(Item < MAX_MENU_ITEMS - 1)
					Item++;
				else
					Item = 0;			
				break;
			case OK:	
				DBG("Ok premuto");
				break;
			case BACK:
				DBG("Back premuto");
				break;
			default:
				break;
		}
	
		if(OldItem != Item)
		{
			DBG("OldItem = " + String(OldItem) + " Item = " + String(Item));
			OldItem = Item;
		}
		
		if(Item > MAX_MENU_VIEW_ITEMS - 1)
		{
			TopItem = Item - (MAX_MENU_VIEW_ITEMS - 1);
		}
		else
			TopItem = 0;
		
		delay(50);
	}
}


