#include "PhaseAnalyzer.h"
#include "Display.h"
#include "Touch.h"
#include "Time.h"
#include "Settings.h"
#include "EepromAnalyzer.h"
#include "Alarms.h"
#include "Logs.h"
#include "Rele.h"
#include "BT.h"
// #include "doom_bmp.h"

#define DRAW_OK		false
#define DRAW_BACK	true

#define RANGE_TAB_LENGHT 21

typedef struct
{
	int16_t x0;
	int16_t y0;
	int16_t w;
	int16_t h;
	uint16_t c;
	
}NAV_BUTTON_COORD;

typedef enum
{
	LINE_1 = 0,
	LINE_2,
	LINE_3,
	MAX_LINES
}LINES;

typedef enum
{
	NO_TYPE = 0,
	INT,
	UINT,
	DOUBLE,
	DOUBLE_NO_FORMAT,
	TIME_TYPE,
	MAX_TYPES
}MEASURE_TYPES;

typedef struct
{
	void *actualMeasure;
	void *max;
	void *min;
	String measureUnit;
	uint8_t type;
}MEASURE_LINE;

typedef struct
{
	MEASURE_LINE	measureL1;
	MEASURE_LINE 	measureL2;
	MEASURE_LINE    measureL3;
}MEASURE_PAGE;


const MEASURE_PAGE MeasureTab[MAX_MEASURE_PAGES] = 
{
	{{&Current.actual, &Current.max	    , &Current.min, "A"   , DOUBLE 			 }, 
	{&Voltage.actual , &Voltage.max     , &Voltage.min, "V"   , DOUBLE 			 } , 
	{&Pf.actual      , &Pf.max          , &Pf.min     , ""    , DOUBLE_NO_FORMAT }},
	
	{{&PAtt.actual   , &PAtt.max   	    , &PAtt.min   , "W"   , DOUBLE           }, 
	{&PRea.actual    , &PRea.max        , &PRea.min   , "VAr" , DOUBLE           } , 
	{&PApp.actual    , &PApp.max        , &PApp.min   , "VA"  , DOUBLE           }},
	
	{{&Current.avg   , &Current.maxAvg  , NULL        , "A"   , DOUBLE           }, 
	{&Voltage.avg    , &Voltage.maxAvg  , NULL        , "V"   , DOUBLE           } , 
	{&Pf.avg         , &Pf.maxAvg       , NULL        , ""    , DOUBLE_NO_FORMAT }},
	
	{{&PAtt.avg      , &PAtt.maxAvg     , NULL        , "W"   , DOUBLE           }, 
	{&PRea.avg       , &PRea.maxAvg     , NULL        , "VAr" , DOUBLE           } ,
	{&PApp.avg       , &PApp.maxAvg     , NULL        , "VA"  , DOUBLE           }},
	
	{{&EnAtt.actual  , NULL        		, NULL        , "Wh"  , DOUBLE 		     }, 
	{&EnRea.actual   , NULL             , NULL        , "VArh", DOUBLE 		     } , 
	{&EnApp.actual   , NULL             , NULL  	  , "VAh" , DOUBLE 		     }},
	
	{{		   NULL  , NULL             , NULL        , ""    , NO_TYPE		     } , 
	{&Time.liveCnt  , NULL        		, NULL        , ""    , TIME_TYPE	     }, 
	{	       NULL  , NULL             , NULL  	  , ""    , NO_TYPE		     }},
};

const RANGES RangeTab[RANGE_TAB_LENGHT] = 
{
	{0.000000001	, 'n',  1000000000.0},
	{0.00000001		, 'n',  1000000000.0},
	{0.0000001		, 'n',  1000000000.0},
	{0.000001		, 'u',     1000000.0},
	{0.00001		, 'u',     1000000.0},
	{0.0001			, 'u',     1000000.0},
	{0.001			, 'm',  	  1000.0},
	{0.01			, 'm',  	  1000.0},
	{0.1			, 'm',  	  1000.0},
	{1.0			, ' ',           1.0},
	{10.0			, ' ',           1.0},
	{100.0			, ' ',           1.0},
	{1000.0			, 'k',         0.001},
	{10000.0		, 'k',         0.001},
	{100000.0		, 'k',         0.001},
	{1000000.0		, 'M',      0.000001},
	{10000000.0		, 'M',      0.000001},
	{100000000.0	, 'M',      0.000001},
	{1000000000.0	, 'G',   0.000000001},
	{10000000000.0	, 'G',   0.000000001},
	{100000000000.0	, 'G',   0.000000001},
	
};


const uint8_t AlarmIcon[] =
{
	0x0f, 0x00, 0x19, 0x80, 0x19, 0x80, 0x36, 0xc0, 
	0x36, 0xc0, 0x36, 0xc0, 0x60, 0x60, 0x66, 0x60, 
	0x66, 0x60, 0xc0, 0x30, 0xff, 0xf0, 0x3f, 0xc0, 
};

const uint8_t LogIcon[] =
{
	0x0f, 0xf0, 0x12, 0x50, 0x12, 0x50, 0x62, 0x50, 
	0x42, 0x50, 0x80, 0x10, 0xa6, 0xd0, 0xa6, 0x90, 
	0xa6, 0xb0, 0xb6, 0xd0, 0x80, 0x10, 0xff, 0xf0, 
};

const uint8_t ReleTimerIcon[] =
{
	0x3f, 0x80, 0x60, 0xc0, 0x80, 0x20, 0x84, 0x20, 
	0x84, 0x20, 0x87, 0xa0, 0x80, 0x20, 0x80, 0x20, 
	0xc0, 0x60, 0x60, 0xc0, 0x1f, 0x00, 0x00, 0x00, 
};

const uint8_t BtIcon[] =
{
	0x60, 0x00, 0x51, 0x20, 0x4b, 0xa0, 0x49, 0x20, 
	0x59, 0x20, 0x61, 0x20, 0x61, 0x20, 0x51, 0x20, 
	0x49, 0x20, 0x49, 0x20, 0x51, 0x70, 0x61, 0x20, 
};

XPT2046_Touchscreen Touch(TOUCH_CS_PIN);
ILI9341_t3 Display = ILI9341_t3(TFT_CS, TFT_DC);
DISPLAY_VAR DisplayParam;

Chrono TimeRefresh, DisplayRefresh, MeasureRefresh, LogInconTimer, BtRefresh;
static bool LogInconToggle;

NAV_BUTTON_COORD Up = {NAV_BUTT_X_START, NAV_BUTT_Y_START, NAV_BUTT_WIDTH, NAV_BUTT_HIGH, ILI9341_RED};
NAV_BUTTON_COORD Down = {NAV_BUTT_X_START, NAV_BUTT_Y_START + (2 * (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE)), NAV_BUTT_WIDTH, NAV_BUTT_HIGH, ILI9341_RED};
NAV_BUTTON_COORD Ok_Back = {NAV_BUTT_X_START, NAV_BUTT_Y_START + (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE), NAV_BUTT_WIDTH, NAV_BUTT_HIGH, ILI9341_GREEN};
	
String LogList[MAX_LOGS];
LOGS_DEF LogBufferReordered[MAX_LOGS];

int32_t CurrentGraphicCopy[N_SAMPLE], VoltageGraphicCopy[N_SAMPLE];

const char *MenuVoices[MAX_MENU_ITEMS] = 
{
	"Misure",
	"Grafici",
	"Log",
	"Allarmi",
	"Presa",
	"Impostazioni",
	"Reset",
};

const char *MeasureTitle[MAX_MEASURE_PAGES] = 
{
	"I V PF",
	"Potenze",
	"Medie 1",
	"Medie 2",
	"Energie",
	"Wake time",
};

const char *LogTitle[MAX_LOGS_PAGES] = 
{
	"Lista dei log",
	"Grafico dei log",
};


const char *AlarmsTitle[MAX_ALARMS] = 
{
	"Sovra corrente",
	"Sotto corrente",
	"Sovra potenza app.",
	"Sotto potenza app.",
};

const char *ReleTitle[MAX_RELE_ITEM] = 
{
	"Stato presa",
	"Imposta timer",
	"Statistiche",
};


const char *ResetsTitle[MAX_RESETS] = 
{
	"Reset default",
	"Riavvio",
	"Massimi/minimi",
	"Medie",
	"Energie",
	"Log",
	"Stat. presa",
};

uint8_t SearchRange(double Value2Search)
{
	uint8_t Range = 0;
	for(Range = 0; Range < RANGE_TAB_LENGHT; Range++)
	{
		if(Value2Search > RangeTab[Range].value)
			continue;
		else
		{
			if(Range - 1 >= 0)
			{
				Range -= 1;
			}
			else
				Range = 9;
			break;
		}
	}

	return Range;
}

void DoTasks()
{
	GetMeasure();	
	if(TimeRefresh.hasPassed(1000, true))
	{
		if(Time.liveCnt == UINT32_MAX)
			Time.liveCnt = 0;
		Time.liveCnt++;
	}
	CheckAlarms();
	LogMeasure();
	RefreshSwitchStatus();
	
	CheckBtDevConn();
	if(BtDeviceConnected)
	{
		if(BtRefresh.hasPassed(500, true))
			BtTransaction();			
	}
	else
		BtRefresh.restart();
	WriteSwitchStatistics(false);
}

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

static void ClearDisplay(bool Inverse)
{
	if(Inverse)
		Display.fillScreen(ILI9341_WHITE);
	else
		Display.fillScreen(ILI9341_BLACK);
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
		Display.setTextColor(ILI9341_BLACK);
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
				KeyPress = BACK;
			}
		}
	}
	return KeyPress;
}


void DrawPopUp(char *Msg, uint16_t Delay)
{
	uint32_t PopUpTimer = 0, PopUpTime = 0;
	ClearDisplay(false);
	Display.setTextColor(ILI9341_GREENYELLOW);
	Display.setFont(Arial_28_Bold);	
	if(TEXT_LENGHT(Msg) > DISPLAY_WIDTH)
		Display.setFont(Arial_20_Bold);	
	Display.setCursor(CENTER_ALIGN(Msg), CENTER_POS);
	Display.print(Msg);
	Display.drawRoundRect(0, 0, DISPLAY_WIDTH, DISPLAY_HIGH, 2, ILI9341_WHITE);
	Display.drawRoundRect(1, 1, DISPLAY_WIDTH - 2, DISPLAY_HIGH - 2, 2, ILI9341_WHITE);
	Display.drawRoundRect(2, 2, DISPLAY_WIDTH - 3, DISPLAY_HIGH - 3, 2, ILI9341_WHITE);
	PopUpTime = millis();
	while(Delay > PopUpTimer)
	{
		DoTasks();
		PopUpTimer = millis() - PopUpTime;
	}
	ClearDisplay(false);
}

void DrawInfoPopUp(char *Msg, uint16_t Delay)
{
	uint32_t PopUpTimer = 0, PopUpTime = 0;
	ClearDisplay(false);
	Display.setTextColor(ILI9341_GREENYELLOW);
	Display.setFont(Arial_28_Bold);
	if(TEXT_LENGHT(Msg) > DISPLAY_WIDTH)
		Display.setFont(Arial_20_Bold);		
	Display.setCursor(CENTER_ALIGN("INFO"), CENTER_POS - 30);	
	Display.print("INFO");
	Display.setCursor(CENTER_ALIGN(Msg), CENTER_POS + 30);
	Display.print(Msg);
	Display.drawRoundRect(0, 0, DISPLAY_WIDTH, DISPLAY_HIGH, 2, ILI9341_WHITE);
	Display.drawRoundRect(1, 1, DISPLAY_WIDTH - 2, DISPLAY_HIGH - 2, 2, ILI9341_WHITE);
	Display.drawRoundRect(2, 2, DISPLAY_WIDTH - 3, DISPLAY_HIGH - 3, 2, ILI9341_WHITE);
	PopUpTime = millis();
	while(Delay > PopUpTimer)
	{
		GetMeasure();	
		if(TimeRefresh.hasPassed(1000, true))
			Time.liveCnt++;
		CheckAlarms();
		LogMeasure();
		if(TimeRefresh.hasPassed(1000, true))
			Time.liveCnt++;
		PopUpTimer = millis() - PopUpTime;
	}
	ClearDisplay(false);
}


void DrawAlarmPopUp(char *Msg, uint16_t Delay)
{
	uint32_t PopUpTimer = 0, PopUpTime = 0;
	ClearDisplay(false);
	Display.setTextColor(ILI9341_GREENYELLOW);
	Display.setFont(Arial_28_Bold);
	if(TEXT_LENGHT(Msg) > DISPLAY_WIDTH)
		Display.setFont(Arial_20_Bold);		
	Display.setCursor(CENTER_ALIGN("ALLARME"), CENTER_POS - 30);	
	Display.print("ALLARME");
	Display.setCursor(CENTER_ALIGN(Msg), CENTER_POS + 30);
	Display.print(Msg);
	Display.drawRoundRect(0, 0, DISPLAY_WIDTH, DISPLAY_HIGH, 2, ILI9341_WHITE);
	Display.drawRoundRect(1, 1, DISPLAY_WIDTH - 2, DISPLAY_HIGH - 2, 2, ILI9341_WHITE);
	Display.drawRoundRect(2, 2, DISPLAY_WIDTH - 3, DISPLAY_HIGH - 3, 2, ILI9341_WHITE);
	PopUpTime = millis();
	while(Delay > PopUpTimer)
	{
		GetMeasure();
		RefreshSwitchStatus();
		if(TimeRefresh.hasPassed(1000, true))
			Time.liveCnt++;
		PopUpTimer = millis() - PopUpTime;
	}
	ClearDisplay(false);
}

bool DrawAskPopUp(char *Msg)
{
	bool Confirm = false;
	uint8_t KeyPress = MAX_KEY;
	ClearDisplay(false);
	DrawNavButtons(DRAW_OK);
	Display.setTextColor(ILI9341_GREENYELLOW);
	Display.setFont(Arial_28_Bold);	
	Display.setCursor(CENTER_ALIGN(Msg), CENTER_POS);
	Display.print(Msg);
	Display.drawRoundRect(0, 0, DISPLAY_WIDTH, DISPLAY_HIGH, 2, ILI9341_WHITE);
	Display.drawRoundRect(1, 1, DISPLAY_WIDTH - 2, DISPLAY_HIGH - 2, 2, ILI9341_WHITE);
	Display.drawRoundRect(2, 2, DISPLAY_WIDTH - 3, DISPLAY_HIGH - 3, 2, ILI9341_WHITE);
	while(1)
	{
		DoTasks();
		KeyPress = ButtonPressed();
		if(KeyPress == OK)
		{
			Confirm = true;
			break;
		}
		else if(KeyPress == BACK)
		{
			Confirm = false;
			break;			
		}
	}
	ClearDisplay(false);
	return Confirm;
}

void DisplaySetup(uint8_t Rotation)
{
	Display.begin();
	ClearDisplay(false);
	Touch.begin();
	DisplaySetRotation(Rotation);
	DisplayParam.displayRotation = Rotation;
	// Display.writeRect(0, 0, start_screen.width, start_screen.height, (uint16_t*)(start_screen.pixel_data));
}


static void ClearMenu()
{
	Display.fillRect(0, MENU_TITLE_POS + 32, DISPLAY_WIDTH - (DISPLAY_WIDTH - NAV_BUTT_X_START) - 2, DISPLAY_HIGH - (MENU_TITLE_POS + 30), ILI9341_BLACK);
}



static void DrawTopInfo()
{
	String Info = "";
	GetTime();
	Display.fillRect(0, 0, DISPLAY_WIDTH, 20, ILI9341_BLACK);
	Display.setTextColor(ILI9341_WHITE);
	Display.setFont(Arial_12);
	Display.setCursor(LEFT_ALIGN, TOP_POS);
	Display.print(TimeStr.c_str());
	Info += ("v " + String(FW_VERSION, 1));
	if(simulationMode)
		Info += " DEMO";
	Display.setCursor(CENTER_ALIGN(Info.c_str()), TOP_POS);
	Display.print(Info.c_str());
	Display.setCursor(RIGHT_ALIGN(DateStr.c_str()), TOP_POS);
	Display.print(DateStr.c_str());
	if(AlarmPresence())
		Display.drawBitmap(LEFT_ALIGN + TEXT_LENGHT(TimeStr.c_str()) + 3, TOP_POS, AlarmIcon, 12, 12, ILI9341_YELLOW);
	if(EnableLog)
	{
		if(LogInconTimer.hasPassed(500, true))
		{
			LogInconToggle = !LogInconToggle;
		}
		if(LogInconToggle)
			Display.drawBitmap(LEFT_ALIGN + TEXT_LENGHT(TimeStr.c_str()) + 18, TOP_POS, LogIcon, 12, 12, ILI9341_GREEN);
	}
	else
	{
		LogInconTimer.restart();
		LogInconToggle = false;
	}
	if(Switch.haveTimer)
	{
		Display.drawBitmap(LEFT_ALIGN + TEXT_LENGHT(TimeStr.c_str()) + 33, TOP_POS, ReleTimerIcon, 12, 12, ILI9341_CYAN);
	}
	if(BtDeviceConnected)
	{
		Display.drawBitmap(LEFT_ALIGN + TEXT_LENGHT(TimeStr.c_str()) + 48, TOP_POS, BtIcon, 12, 12, ILI9341_BLUE);
	}
}


void DrawMainMenu()
{
	bool ExitMainMenu = false;
	uint8_t TopItem = 0, Item = 0, OldItem = 0, KeyPress = MAX_KEY;
	String NumPage = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);
	while(!ExitMainMenu)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
		}
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_12);
		NumPage = String(Item + 1) + "/" + String(MAX_MENU_ITEMS);
		Display.setCursor(RIGHT_ALIGN(NumPage.c_str()) - 20, MENU_TITLE_POS + 10);
		Display.print(NumPage.c_str());
		Display.setFont(Arial_24_Bold);
		Display.setCursor(CENTER_ALIGN("Menu"), MENU_TITLE_POS);
		Display.print("Menu");
		Display.setFont(Arial_18);
		for(int i = 0; i < MAX_MENU_VIEW_ITEMS; i++)
		{
			int NewItem = TopItem + i;
			if(NewItem >= MAX_MENU_ITEMS)
				break;
			if(NewItem == Item)
			{
				Display.setTextColor(ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(MenuVoices[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + Display.fontCapHeight(),
										TEXT_LENGHT(MenuVoices[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(MenuVoices[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 1 + Display.fontCapHeight(),
										TEXT_LENGHT(MenuVoices[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(MenuVoices[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 2 + Display.fontCapHeight(),
										TEXT_LENGHT(MenuVoices[NewItem]), ILI9341_GREENYELLOW);
			}
			else
			{
				Display.setTextColor(ILI9341_WHITE);
			}
			Display.setCursor(CENTER_ALIGN(MenuVoices[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)));
			Display.print(MenuVoices[NewItem]);
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
				AnalyzerPage = Item + 1;
				ExitMainMenu = true;
				break;
			case BACK:
				break;
			default:
				break;
		}
	
		if(OldItem != Item)
		{
			ClearMenu();
			Display.fillRect(RIGHT_ALIGN(NumPage.c_str()) - 24, MENU_TITLE_POS + 8, 40, 16, ILI9341_BLACK);
			OldItem = Item;
		}
		
		if(Item > MAX_MENU_VIEW_ITEMS - 1)
		{
			TopItem = Item - (MAX_MENU_VIEW_ITEMS - 1);
		}
		else
			TopItem = 0;
	}
}


static void FormatMeasure(void *Measure2Format, String *Measure, String *Udm, uint8_t Type)
{
	double MeasureCpy = 0.0;
	switch(Type)
	{
		case INT:
			MeasureCpy = (double)*(int32_t*)Measure2Format;
			break;
		case UINT:
		case TIME_TYPE:
			MeasureCpy = (double)*(uint32_t*)Measure2Format;
			break;
		case DOUBLE:
		case DOUBLE_NO_FORMAT:
			MeasureCpy = *(double*)Measure2Format;
			break;
		default:
			break;
			
	}
	
	uint8_t Range = 0;
	if(Type == DOUBLE)
	{
		Range = SearchRange(MeasureCpy);
		MeasureCpy *= RangeTab[Range].rescale;
		if(RangeTab[Range].odg != ' ')
		{
			*Udm = String(RangeTab[Range].odg);
		}
		*Measure = String(MeasureCpy, 3);
	}
	else if(Type == UINT)
	{
		*Measure = String((uint32_t)MeasureCpy);
	}
	else if(Type == TIME_TYPE)
	{
		uint32_t timeVar = (uint32_t)MeasureCpy;
		uint8_t sec = 0, min = 0, hour = 0;
		*Measure = "";
		hour = timeVar / 3600;
		min = (timeVar / 60) % 60;
		sec = timeVar % 60;
		*Measure += (hour < 10 ? ("0" + String(hour) + ":"): String(hour) + ":");
		*Measure += (min < 10 ? ("0" + String(min) + ":"): String(min) + ":");
		*Measure += (sec < 10 ? ("0" + String(sec)): String(sec));
	}
	else if(Type == DOUBLE_NO_FORMAT)
	{
		if((uint32_t)MeasureCpy == (uint32_t)PF_INVALID)
			*Measure = "----";
		else
		{	
			if(MeasureCpy < 0)
			{
				MeasureCpy = -MeasureCpy;
				*Measure = String(MeasureCpy, 3);
				*Measure += " CAP";
			}
			else if(MeasureCpy > 0)
			{
				*Measure = String(MeasureCpy, 3);
				*Measure += " IND";
			}
			else
				*Measure = String(MeasureCpy, 3);
		}			
	}
	else
	{
		*Measure = String(MeasureCpy, 3);
	}
	
}

static void FormatMaxMin(void *Measure2Format, String *Measure, String *Udm, uint8_t Type)
{
	double MeasureCpy = *(double*)Measure2Format;
	uint8_t Range = 0;
	char MaxMinStr[6];
	String MeasureString = "";
	if(Type != DOUBLE_NO_FORMAT)
	{
		Range = SearchRange(MeasureCpy);
		MeasureCpy *= RangeTab[Range].rescale;
		if(RangeTab[Range].odg != ' ')
		{
			*Udm = String(RangeTab[Range].odg);
		}
		MeasureString = String(MeasureCpy, 3);		
	}
	else if(Type == DOUBLE_NO_FORMAT)
	{
		if((uint32_t)MeasureCpy == (uint32_t)PF_INVALID)
			MeasureString = "----";
		else
			MeasureString = String(MeasureCpy, 3);		
	}
	
	MeasureString.toCharArray(MaxMinStr, 6);
	*Measure = String(MaxMinStr);
}


static void DrawMeasureLine(uint8_t MeasurePage, uint8_t Line)
{
	uint16_t MeasureLenght = 0;
	String MainMeasure = "", Max = "", Min = "", Udm = "";
	const MEASURE_LINE *MeasureTabLine;

	switch(Line)
	{
		case LINE_1:
			MeasureTabLine = &MeasureTab[MeasurePage].measureL1;
			break;
		case LINE_2:
			MeasureTabLine = &MeasureTab[MeasurePage].measureL2;
			break;
		case LINE_3:
			MeasureTabLine = &MeasureTab[MeasurePage].measureL3;
			break;
		default:
			break;
	}
	
	if(MeasureTabLine->actualMeasure != NULL)
	{
		// MISURA PRINCIPALE
		FormatMeasure(MeasureTabLine->actualMeasure, &MainMeasure, &Udm, MeasureTabLine->type);
		Udm += MeasureTabLine->measureUnit;
		
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_28_Bold);	
		Display.setCursor(55, MEASURE_POS + 40 + (Line * 65));
		Display.print(MainMeasure.c_str());
		
		MeasureLenght = TEXT_LENGHT(MainMeasure.c_str());
		Display.setFont(Arial_14_Bold);	
		Display.setCursor(55 + MeasureLenght + 2, MEASURE_POS + 55 + (Line * 65));
		Display.print(Udm.c_str());
	}
		
	if(MeasureTabLine->max != NULL || MeasureTabLine->min != NULL)
	{
		// MISURA MAX
		Display.setFont(Arial_9_Bold);	
		Display.setCursor(0, MEASURE_POS + 30 + (Line * 65));
		Display.print("Max");
		Udm = "";
		if(MeasureTabLine->max != NULL)
			FormatMaxMin(MeasureTabLine->max, &Max, &Udm, MeasureTabLine->type);
		else
			Max = "----";
		Max += Udm;
		Display.setFont(Arial_10_Bold);
		Display.setCursor(0, MEASURE_POS + 42 + (Line * 65));
		Display.print(Max.c_str());	

		// MISURA MIN
		Udm = "";	
		Display.setFont(Arial_9_Bold);
		Display.setCursor(0, MEASURE_POS + 54 + (Line * 65));
		Display.print("Min");

		if(MeasureTabLine->min != NULL)
			FormatMaxMin(MeasureTabLine->min, &Min, &Udm, MeasureTabLine->type);
		else
			Min = "----";
		Min += Udm;
		Display.setFont(Arial_10_Bold);
		Display.setCursor(0, MEASURE_POS + 66 + (Line * 65));
		Display.print(Min.c_str());	
	}
	
}

void DrawMeasurePage()
{
	bool ExitMeasures = false;
	uint8_t TopItem = 0, Item = 0, OldItem = 0, KeyPress = MAX_KEY;
	String NumPage = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_BACK);
	while(!ExitMeasures)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_BACK);		
		}
		Display.setFont(Arial_24_Bold);
		Display.setTextColor(ILI9341_WHITE);
		Display.setCursor(CENTER_ALIGN(MeasureTitle[Item]), MENU_TITLE_POS);
		Display.print(MeasureTitle[Item]);
		Display.setFont(Arial_12);
		Display.setTextColor(ILI9341_WHITE);
		NumPage = String(Item + 1) + "/" + String(MAX_MEASURE_PAGES);
		Display.setCursor(RIGHT_ALIGN(NumPage.c_str()) - 20, MENU_TITLE_POS + 10);
		Display.print(NumPage.c_str());
		if(MeasureRefresh.hasPassed(1000, true))
		{
			ClearMenu();
			for(int Line = 0; Line < MAX_LINES; Line++)
				DrawMeasureLine(Item, Line);				
		}
		
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				if(Item > 0)
					Item--;
				else
					Item = MAX_MEASURE_PAGES - 1;
				break;
			case DOWN:
				if(Item < MAX_MEASURE_PAGES - 1)
					Item++;
				else
					Item = 0;			
				break;
			case OK:	
				break;
			case BACK:
				AnalyzerPage = MAIN_MENU;
				ExitMeasures = true;
				break;
			default:
				break;
		}
	
		if(OldItem != Item)
		{
			Display.fillRect(RIGHT_ALIGN(NumPage.c_str()) - 24, MENU_TITLE_POS + 8, 40, 16, ILI9341_BLACK);
			Display.setFont(Arial_24_Bold);
			Display.fillRect(CENTER_ALIGN(MeasureTitle[OldItem]) - 2, MENU_TITLE_POS - 2, TEXT_LENGHT(MeasureTitle[OldItem]) + 4, 33, ILI9341_BLACK);
			OldItem = Item;
		}
	}		
}

static void DrawGraph()
{
	int32_t y = 0, MaxValI = 0, MaxValV = 0;
	Display.drawRoundRect(GRAPHIC_X, GRAPHIC_Y, GRAPHIC_W, GRAPHIC_H, 1, ILI9341_WHITE);
	Display.drawFastVLine(GRAPHIC_X + (GRAPHIC_W / 2), GRAPHIC_Y, GRAPHIC_H, ILI9341_DARKGREY);
	Display.drawFastHLine(GRAPHIC_X , GRAPHIC_HALF, GRAPHIC_W, ILI9341_DARKGREY);
	if(simulationMode)
	{
		for(int i = 0; i < GRAPHIC_W; i++)
		{
			if(MaxValI < (int32_t)SimCurrentRawVal[i])
				MaxValI = (int32_t)SimCurrentRawVal[i];
			if(MaxValV < (int32_t)SimVoltageRawVal[i])
				MaxValV = (int32_t)SimVoltageRawVal[i];
		}
	
		for(int i = 0; i < GRAPHIC_W; i++)
		{
			// CURRENT
			if((int32_t)SimCurrentRawVal[i] > 0)
				y = GRAPHIC_HALF - ((int32_t)SimCurrentRawVal[i] * (GRAPHIC_H / 3) / MaxValI);
			else
				y = GRAPHIC_HALF - ((int32_t)SimCurrentRawVal[i] * (GRAPHIC_H / 3) / MaxValI);
			Display.drawPixel(GRAPHIC_X + i, y, ILI9341_RED);
			
			// VOLTAGE
			if((int32_t)SimCurrentRawVal[i] > 0)
				y = GRAPHIC_HALF - ((int32_t)SimVoltageRawVal[i] * (GRAPHIC_H / 3) / MaxValV);
			else
				y = GRAPHIC_HALF - ((int32_t)SimVoltageRawVal[i] * (GRAPHIC_H / 3) / MaxValV);
			Display.drawPixel(GRAPHIC_X + i, y, ILI9341_CYAN);
		}
	}
	else
	{
		for(int i = 0; i < GRAPHIC_W; i++)
		{
			CurrentGraphicCopy[i] = (int32_t)CurrentRawVal[i];
			VoltageGraphicCopy[i] = (int32_t)VoltageRawVal[i];
		}		
		for(int i = 0; i < GRAPHIC_W; i++)
		{
			if(MaxValI < CurrentGraphicCopy[i])
				MaxValI = CurrentGraphicCopy[i];
			if(MaxValV < VoltageGraphicCopy[i])
				MaxValV = VoltageGraphicCopy[i];
		}
	
		for(int i = 0; i < GRAPHIC_W; i++)
		{
			// CURRENT
			if(CurrentGraphicCopy[i] > 0)
				y = GRAPHIC_HALF - (((CurrentGraphicCopy[i] - TO_ADC_VAL(CURRENT_BIAS)) * (GRAPHIC_H / 3) / MaxValI) * 2);
			else
				y = GRAPHIC_HALF - (((CurrentGraphicCopy[i] - TO_ADC_VAL(CURRENT_BIAS)) * (GRAPHIC_H / 3) / MaxValI) * 2);
			Display.drawPixel(GRAPHIC_X + i, y, ILI9341_RED);
			
			// VOLTAGE
			if(VoltageGraphicCopy[i] > 0)
				y = GRAPHIC_HALF - (((VoltageGraphicCopy[i] - TO_ADC_VAL(VOLTAGE_BIAS)) * (GRAPHIC_H / 3) / MaxValV) * 2);
			else
				y = GRAPHIC_HALF - (((VoltageGraphicCopy[i] - TO_ADC_VAL(VOLTAGE_BIAS)) * (GRAPHIC_H / 3) / MaxValV) * 2);
			y -= 20;
			Display.drawPixel(GRAPHIC_X + i, y, ILI9341_CYAN);
		}		
	}
	Display.setFont(Arial_12);
	Display.drawFastHLine(GRAPHIC_X + GRAPHIC_W + 5, GRAPHIC_Y + 15, 10, ILI9341_RED);
	Display.drawFastHLine(GRAPHIC_X + GRAPHIC_W + 5, GRAPHIC_Y + 35, 10, ILI9341_CYAN);
	Display.setCursor(GRAPHIC_X + GRAPHIC_W + 18, GRAPHIC_Y + 10);
	Display.print("I");
	Display.setCursor(GRAPHIC_X + GRAPHIC_W + 18, GRAPHIC_Y + 30);
	Display.print("V");
}


void DrawGraphicsPage()
{
	bool ExitGraphics = false;
	uint8_t KeyPress = MAX_KEY;
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_BACK);	
	while(!ExitGraphics)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(250, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_BACK);
			Display.fillRect(GRAPHIC_X + 1, GRAPHIC_Y + 1, GRAPHIC_W - 1, GRAPHIC_H - 1, ILI9341_BLACK);
			DrawGraph();
		}
		Display.setFont(Arial_24_Bold);
		Display.setTextColor(ILI9341_WHITE);
		Display.setCursor(CENTER_ALIGN("Grafici"), MENU_TITLE_POS);
		Display.print("Grafici");	
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				break;
			case DOWN:		
				break;
			case OK:	
				break;
			case BACK:
				AnalyzerPage = MAIN_MENU;
				ExitGraphics = true;
				break;
			default:
				break;
		}
	}		
}


static void FillLogList()
{
	memset(LogBufferReordered, 0x00, sizeof(LOGS_DEF) * MAX_LOGS);
	for(int i = LastLogIndex; i < MAX_LOGS; i++)
	{
		LogBufferReordered[i - LastLogIndex] = LogBuffer[i];
	}
	for(int i = 0; i < LastLogIndex; i++)
	{
		LogBufferReordered[MAX_LOGS - 1 - LastLogIndex + i] = LogBuffer[i];
	}
	for(int i = MAX_LOGS - 1; i >= 0; i--)
	{
		if(LogBufferReordered[i].timeStamp != 0)
		{
			LogList[MAX_LOGS - 1 - i] = TimeStamp2String(LogBufferReordered[i].timeStamp);
			// DateTime LogTime(LogBufferReordered[i].timeStamp);				
			// LogList[MAX_LOGS - 1 - i] = (LogTime.hour() < 10 ? "0" + String(LogTime.hour()) : String(LogTime.hour()));
			// LogList[MAX_LOGS - 1 - i] += ":" + (LogTime.minute() < 10 ? "0" + String(LogTime.minute()) : String(LogTime.minute()));
			// LogList[MAX_LOGS - 1 - i] += ":" + (LogTime.second() < 10 ? "0" + String(LogTime.second()) : String(LogTime.second()));
			// LogList[MAX_LOGS - 1 - i] += "  " + (LogTime.day() < 10 ? "0" + String(LogTime.day()) : String(LogTime.day()));
			// LogList[MAX_LOGS - 1 - i] += "/" + (LogTime.month() < 10 ? "0" + String(LogTime.month()) : String(LogTime.month()));
			// LogList[MAX_LOGS - 1 - i] += "/" + String(LogTime.year() % 100);
			LogList[MAX_LOGS - 1 - i] += " " + String(LogBufferReordered[i].logMeasure, 3);
			switch(MeasureToLog)
			{
				case CURRENT_LOG:
					LogList[MAX_LOGS - 1 - i] += "A";
					break;
				case VOLTAGE_LOG:
					LogList[MAX_LOGS - 1 - i] += "V";
					break;
				case P_ATT_LOG:
					LogList[MAX_LOGS - 1 - i] += "W";
					break;
				case PREA_LOG:
					LogList[MAX_LOGS - 1 - i] += "VAr";
					break;
				case P_APP_LOG:
					LogList[MAX_LOGS - 1 - i] += "VA";
					break;
				case PF_LOG:
					LogList[MAX_LOGS - 1 - i] += "";
					break;
				default:
					break;
			}
		}
		else
		{
			LogList[MAX_LOGS - 1 - i] = "";
			if(i == MAX_LOGS - 1)
				LogList[MAX_LOGS - 1 - i] = "Caricamento valore...";
		}
	}
}


void DrawLogsList()
{
	bool ExitLogs = false, RefreshNumPage = false;
	uint8_t TopItem = 1, LogItem = 1, OldItem = 0, KeyPress = MAX_KEY;
	uint8_t MaxList = 0;
	String NumPage = "";
	Chrono RefreshList;
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);	
	if(LastLogIndex != 0)
	{
		while(!ExitLogs)
		{
			if(!LogFull)
				MaxList = LastLogIndex;
			else
				MaxList = MAX_LOGS - 1;
			DoTasks();
			FillLogList();
			if(DisplayRefresh.hasPassed(500, true))
			{
				DrawTopInfo();
				DrawNavButtons(DRAW_OK);
				RefreshNumPage = true;
			}
			if(EnableLog)
			{
				if(RefreshList.hasPassed((SettingsVals[SET_LOG_TIME] * 1000) / 2, true))
					ClearMenu();
			}
			Display.setTextColor(ILI9341_WHITE);
			Display.setFont(Arial_12);
			NumPage = String(LogItem + 1) + "/" + String(MaxList);
			Display.setCursor(RIGHT_ALIGN(NumPage.c_str()) - 10, MENU_TITLE_POS + 10);
			Display.print(NumPage.c_str());
			Display.setFont(Arial_24_Bold);
			Display.setCursor(CENTER_ALIGN("Lista log"), MENU_TITLE_POS);
			Display.print("Lista log");
			Display.setFont(Arial_13);
			for(int i = 0; i < MAX_MENU_VIEW_ITEMS; i++)
			{
				int NewItem = TopItem + i;
				if(NewItem >= MaxList)
					break;
				if(NewItem == LogItem)
				{
					Display.setTextColor(ILI9341_GREENYELLOW);
					Display.drawFastHLine(CENTER_ALIGN(LogList[NewItem].c_str()) - 10, MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + Display.fontCapHeight(),
											TEXT_LENGHT(LogList[NewItem].c_str()), ILI9341_GREENYELLOW);
					Display.drawFastHLine(CENTER_ALIGN(LogList[NewItem].c_str()) - 10, MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 1 + Display.fontCapHeight(),
											TEXT_LENGHT(LogList[NewItem].c_str()), ILI9341_GREENYELLOW);
					Display.drawFastHLine(CENTER_ALIGN(LogList[NewItem].c_str()) - 10, MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 2 + Display.fontCapHeight(),
											TEXT_LENGHT(LogList[NewItem].c_str()), ILI9341_GREENYELLOW);
				}
				else
				{
					Display.setTextColor(ILI9341_WHITE);
				}
				Display.setCursor(CENTER_ALIGN(LogList[NewItem].c_str()) - 10, MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)));
				Display.print(LogList[NewItem].c_str());
			}
			KeyPress = ButtonPressed();
			switch(KeyPress)
			{
				case UP:	
					if(LogItem > 1)
						LogItem--;
					else
						LogItem = MaxList - 1;
					break;
				case DOWN:
					if(LogItem < MaxList - 1)
						LogItem++;
					else
						LogItem = 1;			
					break;
				case OK:	
					break;
				case BACK:
					AnalyzerPage = LOGS;
					ExitLogs = true;
					break;
				default:
					break;
			}
		
			if(RefreshNumPage)
			{
				Display.fillRect(RIGHT_ALIGN(NumPage.c_str()) - 22, MENU_TITLE_POS + 8, TEXT_LENGHT(NumPage.c_str()) + 22, 16, ILI9341_BLACK);
				RefreshNumPage = false;
			}
		
			if(OldItem != LogItem)
			{
				ClearMenu();
				OldItem = LogItem;
			}	

			if(LogItem > MAX_MENU_VIEW_ITEMS - 1)
			{
				TopItem = LogItem - (MAX_MENU_VIEW_ITEMS - 1);
			}
			else
				TopItem = 1;
		}
	}
	else
	{
		AnalyzerPage = LOGS;
		DrawPopUp("Nessun log", 2000);
	}
}

static void DrawGraphLog(LOGS_DEF *LogBufferLocal, bool CursorOn, uint16_t CursorPos)
{
	int32_t y = 0, y_old = 0, MaxVal = 0, y_cursor = 0;
	int LogBufferIndex = 0;
	CursorPos /= 2;
	Display.drawRoundRect(LOG_GRAPHIC_X, LOG_GRAPHIC_Y, LOG_GRAPHIC_W, LOG_GRAPHIC_H, 1, ILI9341_WHITE);
	Display.drawFastVLine(LOG_GRAPHIC_X + (LOG_GRAPHIC_W / 2), LOG_GRAPHIC_Y, LOG_GRAPHIC_H, ILI9341_DARKGREY);
	Display.drawFastHLine(LOG_GRAPHIC_X , LOG_GRAPHIC_HALF, LOG_GRAPHIC_W, ILI9341_DARKGREY);

	for(int i = 0; i < MAX_LOGS; i++)
	{
		if(MaxVal < (int32_t)LogBufferLocal[i].logMeasure)
			MaxVal = (int32_t)LogBufferLocal[i].logMeasure;

	}

	for(int i = 0; i < LOG_GRAPHIC_W; i++)
	{
		if((int32_t)LogBufferLocal[LogBufferIndex].logMeasure > 0)
			y = LOG_GRAPHIC_HALF - ((int32_t)LogBufferLocal[MAX_LOGS - 1 - LogBufferIndex].logMeasure * (LOG_GRAPHIC_H / 3) / MaxVal);
		else
			y = LOG_GRAPHIC_HALF - ((int32_t)LogBufferLocal[MAX_LOGS - 1 - LogBufferIndex].logMeasure * (LOG_GRAPHIC_H / 3) / MaxVal);
		Display.drawPixel(LOG_GRAPHIC_X + i, y, ILI9341_YELLOW);
		if(i % 2 == 0)
			LogBufferIndex++;
		if(i == 0)
			y_old = y;
		if(y_old != y && abs(y_old - y) > 1)
		{
			if(y_old > y)
				Display.drawFastVLine(LOG_GRAPHIC_X + i, y, y_old - y, ILI9341_YELLOW);
			else
				Display.drawFastVLine(LOG_GRAPHIC_X + i, y_old, y - y_old, ILI9341_YELLOW);
			y_old = y;
		}
		if(CursorOn)
		{
			y_cursor = LOG_GRAPHIC_HALF - ((int32_t)LogBufferLocal[MAX_LOGS - 1 - CursorPos].logMeasure * (LOG_GRAPHIC_H / 3) / MaxVal);
			Display.drawFastHLine(CursorPos - 5, y_cursor, 10, ILI9341_RED);
			Display.drawFastVLine(CursorPos, y_cursor - 5, 10, ILI9341_RED);
		}
	}	
	if(CursorOn)
	{
		String LogMeasStr = "";
		if(LogBufferLocal[MAX_LOGS - 1 - CursorPos].timeStamp != 0)
		{
			LogMeasStr = "Misura: ";
			LogMeasStr += String(LogBufferLocal[MAX_LOGS - 1 - CursorPos].logMeasure, 3);
			switch(MeasureToLog)
			{
				case CURRENT_LOG:
					LogMeasStr += "A";
					break;
				case VOLTAGE_LOG:
					LogMeasStr += "V";
					break;
				case P_ATT_LOG:
					LogMeasStr += "W";
					break;
				case PREA_LOG:
					LogMeasStr += "VAr";
					break;
				case P_APP_LOG:
					LogMeasStr += "VA";
					break;
				case PF_LOG:
					LogMeasStr += "";
					break;
				default:
					break;
			}
			Display.setFont(Arial_20);
			Display.setTextColor(ILI9341_WHITE);
			Display.setCursor(CENTER_ALIGN(LogMeasStr.c_str()), LOG_GRAPHIC_Y + LOG_GRAPHIC_H + 20);
			Display.print(LogMeasStr.c_str());	
			LogMeasStr = "Data: ";
			LogMeasStr += TimeStamp2String(LogBufferLocal[MAX_LOGS - 1 - CursorPos].timeStamp);
			Display.setCursor(CENTER_ALIGN(LogMeasStr.c_str()), LOG_GRAPHIC_Y + LOG_GRAPHIC_H + 60);
			Display.print(LogMeasStr.c_str());	
		}
		else		
		{
			Display.setFont(Arial_20);
			Display.setTextColor(ILI9341_WHITE);
			LogMeasStr = "Misura: ----";
			Display.setCursor(CENTER_ALIGN(LogMeasStr.c_str()), LOG_GRAPHIC_Y + LOG_GRAPHIC_H + 20);
			Display.print(LogMeasStr.c_str());	
			LogMeasStr = "Data: --:-- --/--/--";
			Display.setCursor(CENTER_ALIGN(LogMeasStr.c_str()), LOG_GRAPHIC_Y + LOG_GRAPHIC_H + 60);
			Display.print(LogMeasStr.c_str());				
		}
	}

}

void DrawLogGraphic()
{
	bool ExitGraphics = false, CursorOn = false;
	uint8_t KeyPress = MAX_KEY;
	uint16_t CursorPos = 0;
	if(LastLogIndex != 0)
	{
		ClearDisplay(false);
		DisplayRefresh.restart();
		DrawTopInfo();
		DrawNavButtons(DRAW_OK);	
		while(!ExitGraphics)
		{
			DoTasks();
			memset(LogBufferReordered, 0x00, sizeof(LOGS_DEF) * MAX_LOGS);
			if(DisplayRefresh.hasPassed(500, true))
			{
				DrawTopInfo();
				DrawNavButtons(DRAW_OK);
				Display.fillRect(LOG_GRAPHIC_X + 1, LOG_GRAPHIC_Y + 1, LOG_GRAPHIC_W - 1, LOG_GRAPHIC_H - 1, ILI9341_BLACK);
			}
			Display.setFont(Arial_24_Bold);
			Display.setTextColor(ILI9341_WHITE);
			Display.setCursor(CENTER_ALIGN("Log grafico"), MENU_TITLE_POS);
			Display.print("Log grafico");	
			for(int i = LastLogIndex; i < MAX_LOGS; i++)
			{
				LogBufferReordered[i - LastLogIndex] = LogBuffer[i];
			}
			for(int i = 0; i < LastLogIndex; i++)
			{
				LogBufferReordered[MAX_LOGS - 1 - LastLogIndex + i] = LogBuffer[i];
			}
			DrawGraphLog(LogBufferReordered, CursorOn, CursorPos);
			KeyPress = ButtonPressed();
			switch(KeyPress)
			{
				case UP:	
					if(CursorPos > 0)
						CursorPos--;
					else
						CursorPos = LOG_GRAPHIC_W - 1;
					break;
				case DOWN:	
					if(CursorPos < LOG_GRAPHIC_W - 1)
						CursorPos++;
					else
						CursorPos = 0;
					break;
				case OK:	
					CursorOn = !CursorOn;
					break;
				case BACK:
					AnalyzerPage = LOGS;
					ExitGraphics = true;
					break;
				default:
					break;
			}
		
			// if(OldItem != Item)
			// {
				// ClearMenu();
				// OldItem = Item;
			// }
		}	
	}
	else
	{
		AnalyzerPage = LOGS;
		DrawPopUp("Nessun log", 2000);
	}	
}


void DrawLogMenu()
{
	bool ExitLogMenu = false;
	uint8_t TopItem = 0, LogMenuItem = 0, OldItem = 0, KeyPress = MAX_KEY;
	String NumPage = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);	
	while(!ExitLogMenu)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
		}
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_12);
		NumPage = String(LogMenuItem + 1) + "/" + String(MAX_LOGS_PAGES);
		Display.setCursor(RIGHT_ALIGN(NumPage.c_str()) - 10, MENU_TITLE_POS + 10);
		Display.print(NumPage.c_str());
		Display.setFont(Arial_24_Bold);
		Display.setCursor(CENTER_ALIGN("Log menu"), MENU_TITLE_POS);
		Display.print("Log menu");
		Display.setFont(Arial_18);
		for(int i = 0; i < MAX_MENU_VIEW_ITEMS; i++)
		{
			int NewItem = TopItem + i;
			if(NewItem >= MAX_LOGS_PAGES)
				break;
			if(NewItem == LogMenuItem)
			{
				Display.setTextColor(ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(LogTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + Display.fontCapHeight(),
										TEXT_LENGHT(LogTitle[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(LogTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 1 + Display.fontCapHeight(),
										TEXT_LENGHT(LogTitle[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(LogTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 2 + Display.fontCapHeight(),
										TEXT_LENGHT(LogTitle[NewItem]), ILI9341_GREENYELLOW);
			}
			else
			{
				Display.setTextColor(ILI9341_WHITE);
			}
			Display.setCursor(CENTER_ALIGN(LogTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)));
			Display.print(LogTitle[NewItem]);
		}
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				if(LogMenuItem > 0)
					LogMenuItem--;
				else
					LogMenuItem = MAX_LOGS_PAGES - 1;
				break;
			case DOWN:
				if(LogMenuItem < MAX_LOGS_PAGES - 1)
					LogMenuItem++;
				else
					LogMenuItem = 0;			
				break;
			case OK:	
				if(LogMenuItem == LOG_LIST)
					AnalyzerPage = LIST_LOG;
				else
					AnalyzerPage = GRAPHIC_LOG;
				ExitLogMenu = true;
				ClearDisplay(false);
				break;
			case BACK:
				AnalyzerPage = MAIN_MENU;
				ExitLogMenu = true;
				break;
			default:
				break;
		}
	
		if(OldItem != LogMenuItem)
		{
			ClearMenu();
			Display.fillRect(RIGHT_ALIGN(NumPage.c_str()) - 14, MENU_TITLE_POS + 8, 50, 16, ILI9341_BLACK);
			OldItem = LogMenuItem;
		}	
		if(LogMenuItem > MAX_MENU_VIEW_ITEMS - 1)
		{
			TopItem = LogMenuItem - (MAX_MENU_VIEW_ITEMS - 1);
		}
		else
			TopItem = 0;
	}	
}

static void ViewDetailAlarm(uint8_t AlarmIndex)
{
	bool ExitAlarmsView = false;
	uint8_t KeyPress = MAX_KEY;
	String AlarmTime = "", NOccur = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_BACK);	
	while(!ExitAlarmsView)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_BACK);
		}
		Display.setFont(Arial_28_Bold);
		if(TEXT_LENGHT(AlarmsTab[AlarmIndex].alarmName.c_str()) > DISPLAY_WIDTH)
			Display.setFont(Arial_20_Bold);
		Display.setFont(Arial_24_Bold);
		Display.setCursor(CENTER_ALIGN(AlarmsTab[AlarmIndex].alarmName.c_str()), MENU_TITLE_POS);
		Display.print(AlarmsTab[AlarmIndex].alarmName.c_str());
		Display.setFont(Arial_18);
		if(AlarmsTab[AlarmIndex].isEnabled)
		{
			Display.setCursor(CENTER_ALIGN("Abilitato"), MENU_TITLE_POS + 65);
			Display.print("Abilitato");	
			if(AlarmsTab[AlarmIndex].isActive)
			{
				Display.setCursor(CENTER_ALIGN("ATTIVO"), MENU_TITLE_POS + 105);
				Display.print("ATTIVO");
			}
			else
			{
				Display.setCursor(CENTER_ALIGN("NON ATTIVO"), MENU_TITLE_POS + 105);
				Display.print("NON ATTIVO");				
			}
			if(AlarmsTab[AlarmIndex].nOccurence > 0)
			{
				AlarmTime = (AlarmsTab[AlarmIndex].alarmHour < 10 ? "0" + String(AlarmsTab[AlarmIndex].alarmHour) : String(AlarmsTab[AlarmIndex].alarmHour));
				AlarmTime += ":" + (AlarmsTab[AlarmIndex].alarmMinute < 10 ? "0" + String(AlarmsTab[AlarmIndex].alarmMinute) : String(AlarmsTab[AlarmIndex].alarmMinute));
				AlarmTime += "  " + (AlarmsTab[AlarmIndex].alarmDay < 10 ? "0" + String(AlarmsTab[AlarmIndex].alarmDay) : String(AlarmsTab[AlarmIndex].alarmDay));
				AlarmTime += "/" + (AlarmsTab[AlarmIndex].alarmMonth < 10 ? "0" + String(AlarmsTab[AlarmIndex].alarmMonth) : String(AlarmsTab[AlarmIndex].alarmMonth));
				AlarmTime += "/" + (AlarmsTab[AlarmIndex].alarmYear < 10 ? "0" + String(AlarmsTab[AlarmIndex].alarmYear) : String(AlarmsTab[AlarmIndex].alarmYear));

				Display.setCursor(CENTER_ALIGN(AlarmTime.c_str()), MENU_TITLE_POS + 135);
				Display.print(AlarmTime.c_str());	

				NOccur = "Occorrenze: " + String(AlarmsTab[AlarmIndex].nOccurence);
				Display.setCursor(CENTER_ALIGN(NOccur.c_str()), MENU_TITLE_POS + 165);
				Display.print(NOccur.c_str());
			}

			// }
			// else
			// {
			// 	Display.setCursor(CENTER_ALIGN(" NON ATTIVO"), MENU_TITLE_POS + 105);
			// 	Display.print("NON ATTIVO");					
			// }
		}
		else
		{
			Display.setCursor(CENTER_ALIGN("Non abilitato"), MENU_TITLE_POS + 65);
			Display.print("Non abilitato");			
		}
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				break;
			case DOWN:		
				break;
			case OK:	
				break;
			case BACK:
				ExitAlarmsView = true;
				ClearDisplay(false);
				break;
			default:
				break;
		}
	
	}	
}

void DrawAlarmPage()
{
	bool ExitAlarms = false;
	uint8_t TopItem = 0, AlarmItem = 0, OldItem = 0, KeyPress = MAX_KEY;
	String NumPage = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);	
	while(!ExitAlarms)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
		}
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_12);
		NumPage = String(AlarmItem + 1) + "/" + String(MAX_ALARMS);
		Display.setCursor(RIGHT_ALIGN(NumPage.c_str()) - 10, MENU_TITLE_POS + 10);
		Display.print(NumPage.c_str());
		Display.setFont(Arial_24_Bold);
		Display.setCursor(CENTER_ALIGN("Allarmi"), MENU_TITLE_POS);
		Display.print("Allarmi");
		Display.setFont(Arial_18);
		for(int i = 0; i < MAX_MENU_VIEW_ITEMS; i++)
		{
			int NewItem = TopItem + i;
			if(NewItem >= MAX_ALARMS)
				break;
			if(NewItem == AlarmItem)
			{
				Display.setTextColor(ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(AlarmsTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + Display.fontCapHeight(),
										TEXT_LENGHT(AlarmsTitle[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(AlarmsTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 1 + Display.fontCapHeight(),
										TEXT_LENGHT(AlarmsTitle[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(AlarmsTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 2 + Display.fontCapHeight(),
										TEXT_LENGHT(AlarmsTitle[NewItem]), ILI9341_GREENYELLOW);
			}
			else
			{
				Display.setTextColor(ILI9341_WHITE);
			}
			Display.setCursor(CENTER_ALIGN(AlarmsTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)));
			Display.print(AlarmsTitle[NewItem]);
		}
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				if(AlarmItem > 0)
					AlarmItem--;
				else
					AlarmItem = MAX_ALARMS - 1;
				break;
			case DOWN:
				if(AlarmItem < MAX_ALARMS - 1)
					AlarmItem++;
				else
					AlarmItem = 0;			
				break;
			case OK:	
				ViewDetailAlarm(AlarmItem);
				ClearDisplay(false);
				break;
			case BACK:
				AnalyzerPage = MAIN_MENU;
				ExitAlarms = true;
				break;
			default:
				break;
		}
	
		if(OldItem != AlarmItem)
		{
			ClearMenu();
			Display.fillRect(RIGHT_ALIGN(NumPage.c_str()) - 14, MENU_TITLE_POS + 8, 50, 16, ILI9341_BLACK);
			OldItem = AlarmItem;
		}	
		if(AlarmItem > MAX_MENU_VIEW_ITEMS - 1)
		{
			TopItem = AlarmItem - (MAX_MENU_VIEW_ITEMS - 1);
		}
		else
			TopItem = 0;
	}
}


static void SetAndShowSwitchStatus()
{
	bool ExitStatusRele = false;
	uint8_t KeyPress = MAX_KEY;
	bool SwitchStatus = false, OldSwitchStatus = false;
	String SwitchStatusStr = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_BACK);	
	SwitchStatus = Switch.isActive;
	OldSwitchStatus = !SwitchStatus;
	while(!ExitStatusRele)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_BACK);
		}
		if(OldSwitchStatus != SwitchStatus)
		{
			ClearMenu();
			OldSwitchStatus = SwitchStatus;
			Display.setFont(Arial_24_Bold);
			if(SwitchStatus)
			{
				SwitchStatusStr = "ACCESA";
				Display.fillRoundRect(CENTER_ALIGN_BUTT(SwitchStatusStr.c_str()) - 2, CENTER_POS - 12, TEXT_LENGHT(SwitchStatusStr.c_str()) + 2, Display.fontCapHeight() + 2, 2, ILI9341_RED);
			}
			else
			{
				SwitchStatusStr = "SPENTA";
				Display.fillRoundRect(CENTER_ALIGN_BUTT(SwitchStatusStr.c_str()) - 2, CENTER_POS - 12, TEXT_LENGHT(SwitchStatusStr.c_str()) + 2, Display.fontCapHeight() + 2, 2, ILI9341_GREEN);
			}
			Display.setTextColor(ILI9341_WHITE);
			Display.setCursor(CENTER_ALIGN_BUTT(SwitchStatusStr.c_str()), CENTER_POS - 10);
			Display.print(SwitchStatusStr.c_str());			
		}

		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:
			case DOWN:
				SwitchStatus = !SwitchStatus;			
				break;
			case OK:	
				if(SwitchStatus)
				{
					if(AlarmPresence())
						DrawInfoPopUp("Allarme attivo", 1000);
					else
						Switch.isActive = SwitchStatus;
				}
				else
					Switch.isActive = SwitchStatus;
				ExitStatusRele = true;
				break;
			case BACK:
				ExitStatusRele = true;
				break;
			default:
				break;
		}
	
	}	
}

static void DrawReleStatisticsList()
{
	bool ExitReleStatistics = false;
	uint8_t KeyPress = MAX_KEY;
	uint8_t sec = 0, min = 0, hour = 0, day = 0;
	String TimeOnStr = "", NSwitchStr = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_BACK);	
	hour = Switch.powerOnTime / 3600;
	min = (Switch.powerOnTime / 60) % 60;
	sec = Switch.powerOnTime % 60;
	day = Switch.powerOnTime / 86400;
	TimeOnStr += String(day) + "d ";
	TimeOnStr += (hour < 10 ? ("0" + String(hour) + "h "): String(hour) + "h ");
	TimeOnStr += (min < 10 ? ("0" + String(min) + "m "): String(min) + "m ");
	TimeOnStr += (sec < 10 ? ("0" + String(sec) + "s"): String(sec) + "s");
	NSwitchStr = String(Switch.nSwitch);
	while(!ExitReleStatistics)
	{
		DoTasks();
		hour = Switch.powerOnTime / 3600;
		min = (Switch.powerOnTime / 60) % 60;
		sec = Switch.powerOnTime % 60;
		day = Switch.powerOnTime / 86400;
		TimeOnStr = "Tempo attivo: ";
		TimeOnStr += String(day) + "d ";
		TimeOnStr += (hour < 10 ? ("0" + String(hour) + "h "): String(hour) + "h ");
		TimeOnStr += (min < 10 ? ("0" + String(min) + "m "): String(min) + "m ");
		TimeOnStr += (sec < 10 ? ("0" + String(sec) + "s"): String(sec) + "s");
		NSwitchStr = "Accensioni: " + String(Switch.nSwitch);
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_BACK);
			ClearMenu();
		}
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_24_Bold);
		Display.setCursor(CENTER_ALIGN("Stat. presa"), MENU_TITLE_POS);
		Display.print("Stat. presa");
		Display.setFont(Arial_13);
		Display.setCursor(CENTER_ALIGN_BUTT(TimeOnStr.c_str()), MENU_TITLE_POS + 80);
		Display.print(TimeOnStr.c_str());
		Display.setCursor(CENTER_ALIGN_BUTT(NSwitchStr.c_str()), MENU_TITLE_POS + 120);
		Display.print(NSwitchStr.c_str());
		
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				break;
			case DOWN:		
				break;
			case OK:	
				break;
			case BACK:
				ExitReleStatistics = true;
				break;
			default:
				break;
		}

	}

}

void DrawRelePage()
{
	bool ExitRelePage = false;
	uint8_t TopItem = 0, ReleItem = 0, OldItem = 0, KeyPress = MAX_KEY;
	String NumPage = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);	
	while(!ExitRelePage)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
		}
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_12);
		NumPage = String(ReleItem + 1) + "/" + String(MAX_RELE_ITEM);
		Display.setCursor(RIGHT_ALIGN(NumPage.c_str()) - 10, MENU_TITLE_POS + 10);
		Display.print(NumPage.c_str());
		Display.setFont(Arial_24_Bold);
		Display.setCursor(CENTER_ALIGN("Presa"), MENU_TITLE_POS);
		Display.print("Presa");
		Display.setFont(Arial_18);
		for(int i = 0; i < MAX_MENU_VIEW_ITEMS; i++)
		{
			int NewItem = TopItem + i;
			if(NewItem >= MAX_RELE_ITEM)
				break;
			if(NewItem == ReleItem)
			{
				Display.setTextColor(ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(ReleTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + Display.fontCapHeight(),
										TEXT_LENGHT(ReleTitle[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(ReleTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 1 + Display.fontCapHeight(),
										TEXT_LENGHT(ReleTitle[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(ReleTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 2 + Display.fontCapHeight(),
										TEXT_LENGHT(ReleTitle[NewItem]), ILI9341_GREENYELLOW);
			}
			else
			{
				Display.setTextColor(ILI9341_WHITE);
			}
			Display.setCursor(CENTER_ALIGN(ReleTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)));
			Display.print(ReleTitle[NewItem]);
		}
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				if(ReleItem > 0)
					ReleItem--;
				else
					ReleItem = MAX_RELE_ITEM - 1;
				break;
			case DOWN:
				if(ReleItem < MAX_RELE_ITEM - 1)
					ReleItem++;
				else
					ReleItem = 0;			
				break;
			case OK:	
				switch(ReleItem)
				{
					case STATUS:
						SetAndShowSwitchStatus();
						break;
					case SET_TIMER:
						ChangeTimeDate(false, true);
						break;
					// case SET_ALARM:
						// ChangeEnum(0, true);
						// break;
					case STATISTICS:
						DrawReleStatisticsList();
						break;
					default:
						break;
				}
				ClearDisplay(false);
				break;
			case BACK:
				AnalyzerPage = MAIN_MENU;
				ExitRelePage = true;
				break;
			default:
				break;
		}
	
		if(OldItem != ReleItem)
		{
			ClearMenu();
			Display.fillRect(RIGHT_ALIGN(NumPage.c_str()) - 14, MENU_TITLE_POS + 8, 50, 16, ILI9341_BLACK);
			OldItem = ReleItem;
		}	
		if(ReleItem > MAX_MENU_VIEW_ITEMS - 1)
		{
			TopItem = ReleItem - (MAX_MENU_VIEW_ITEMS - 1);
		}
		else
			TopItem = 0;
	}
}

void ChangeTimeDate(bool isTime, bool isSwitch)
{
	bool ExitChangeTimeDate = false;
	uint8_t hour_day = 0, minute_month = 0, year = 0, BoxPos = 0,  KeyPress = MAX_KEY;
	String TimeDateStr = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);
	if(isTime || isSwitch)
	{
		if(isTime)
		{
			hour_day = Time.hour;
			minute_month = Time.minute;
			if(hour_day < 10)
				TimeDateStr = "0" + String(hour_day);
			else
				TimeDateStr = String(hour_day);
			if(minute_month < 10)
				TimeDateStr += ":0" + String(minute_month);
			else
				TimeDateStr += ":" + String(minute_month);
		}
		else
		{
			TimeDateStr = "00:00";
		}
	}
	else
	{
		hour_day = Time.day;
		minute_month = Time.month;
		BoxPos = 1;
		year = Time.year % 100;
		if(hour_day < 10)
			TimeDateStr = "0" + String(hour_day);
		else
			TimeDateStr = String(hour_day);
		if(minute_month < 10)
			TimeDateStr += "/0" + String(minute_month);
		else
			TimeDateStr += "/" + String(minute_month);
		
		TimeDateStr += "/" + String(year);
	}
	while(!ExitChangeTimeDate)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
		}
		if(isTime || isSwitch)
		{
			if(hour_day < 10)
				TimeDateStr = "0" + String(hour_day);
			else
				TimeDateStr = String(hour_day);
			if(minute_month < 10)
				TimeDateStr += ":0" + String(minute_month);
			else
				TimeDateStr += ":" + String(minute_month);
		}
		else
		{
			if(hour_day < 10)
				TimeDateStr = "0" + String(hour_day);
			else
				TimeDateStr = String(hour_day);
			if(minute_month < 10)
				TimeDateStr += "/0" + String(minute_month);
			else
				TimeDateStr += "/" + String(minute_month);
			
			TimeDateStr += "/" + String(year);
		}
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_24_Bold);
		if(isTime || isSwitch)
		{
			if(isTime)
			{
				Display.setCursor(CENTER_ALIGN("Cambia ora"), MENU_TITLE_POS);
				Display.print("Cambia ora");
			}
			else
			{
				Display.setCursor(CENTER_ALIGN("Imposta timer"), MENU_TITLE_POS);
				Display.print("Imposta timer");				
			}
		}
		else
		{
			Display.setCursor(CENTER_ALIGN("Cambia data"), MENU_TITLE_POS);
			Display.print("Cambia data");			
		}
		Display.setFont(Arial_32_Bold);
		Display.setCursor(CENTER_ALIGN(TimeDateStr.c_str()), CENTER_POS);
		Display.print(TimeDateStr.c_str());
		Display.drawRoundRect(CENTER_ALIGN(TimeDateStr.c_str()) - 2 + (BoxPos * (50 + 10)), CENTER_POS - 2, 
							  52, 35, 1, ILI9341_CYAN);
		
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				if(isTime || isSwitch)
				{
					switch(BoxPos)
					{
						case 0:
							if(hour_day > 0)
								hour_day--;
							else
								hour_day = 23;
							break;
						case 1:
							if(minute_month > 0)
								minute_month--;
							else
								minute_month = 59;							
							break;
						default:
							break;
					}
				}
				else
				{
					switch(BoxPos)
					{
						case 0:
							if(hour_day > 1)
								hour_day--;
							else
								hour_day = Day4Month[minute_month - 1];
							break;
						case 1:
							if(minute_month > 1)
								minute_month--;
							else
								minute_month = 12;
							break;
						case 2:
							if(year > 20)
								year--;
							else
								year = 99;
							break;
						default:
							break;
					}					
				}
				break;
			case DOWN:	
				if(isTime || isSwitch)
				{
					switch(BoxPos)
					{
							case 0:
							if(hour_day < 23)
								hour_day++;
							else
								hour_day = 0;
							break;
						case 1:
							if(minute_month < 59)
								minute_month++;
							else
								minute_month = 0;							
							break;
						default:
							break;
					}					
				}
				else
				{
					switch(BoxPos)
					{
						case 0:
							if(hour_day < Day4Month[minute_month - 1])
								hour_day++;
							else
								hour_day = 1;
							break;
						case 1:
							if(minute_month < 12)
								minute_month++;
							else
								minute_month = 1;
							break;
						case 2:
							if(year < 99)
								year++;
							else
								year = 20;
							break;
						default:
							break;
					}					
				}
				break;
			case OK:
				if(isTime || isSwitch)
					BoxPos++;
				else
				{
					if(BoxPos == 1)
						BoxPos = 0;
					else if(BoxPos == 0)
						BoxPos = 2;
					else if(BoxPos == 2)
						BoxPos++;
				}
				if((isTime || isSwitch) && BoxPos > 1)
				{
					if(isTime)
					{
						SetTime(hour_day, minute_month);
						ExitChangeTimeDate = true;
						DrawPopUp("Ora cambiata", 1000);
					}
					else
					{
						if(hour_day == 0 && minute_month == 0)
						{
							if(Switch.haveTimer)
								DrawPopUp("Timer cancellato", 1000);	
							Switch.haveTimer = false;
							Switch.timerDuration = 0;
						}
						else
						{
							Switch.timerDuration = (hour_day * 3600) + (minute_month * 60);
							Switch.haveTimer = true;
							DrawPopUp("Timer impostato", 1000);		
						}
						ExitChangeTimeDate = true;
					}
				}
				if((!isTime && !isSwitch) && BoxPos > 2)
				{
					SetDate(hour_day, minute_month, year);
					ExitChangeTimeDate = true;
					DrawPopUp("Data cambiata", 1000);	
				}
				break;
			case BACK:
				ExitChangeTimeDate = true;
				break;
			default:
				break;
		}
	
		if(KeyPress != MAX_KEY)
		{
			ClearMenu();
		}
	}
}

static void ChangeValue(uint8_t SettingIndex)
{
	bool ExitChangeValue = false;
	uint8_t KeyPress = MAX_KEY, BoxPos = 0;
	int32_t NewValue = *(int32_t*)Settings[SettingIndex].settingVal;
	String ValueStr = "";
	uint8_t MaxValueLenght = String(Settings[SettingIndex].settingMax).length(), ActualValueLen = String(NewValue).length();
	char SingleVal = ' ';
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);	
	for(int i = 0; i < (MaxValueLenght - ActualValueLen); i++)
		ValueStr += "0";
	ValueStr += String(NewValue);
	
	SingleVal = ValueStr.charAt(BoxPos);
	while(!ExitChangeValue)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
		}
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_18_Bold);
		Display.setCursor(CENTER_ALIGN(Settings[SettingIndex].settingName), MENU_TITLE_POS);
		Display.print(Settings[SettingIndex].settingName);
		Display.setFont(Arial_32_Bold);
		Display.setCursor(CENTER_ALIGN(ValueStr.c_str()), CENTER_POS);
		Display.print(ValueStr.c_str());
		Display.drawRoundRect(CENTER_ALIGN(ValueStr.c_str()) - 2 + (BoxPos * 25), CENTER_POS - 2, 
							  25, 35, 1, ILI9341_CYAN);
							  
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_16_Bold);	
		Display.setCursor(CENTER_ALIGN(Settings[SettingIndex].udm), CENTER_POS + 55);
		Display.print(Settings[SettingIndex].udm);
		
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:
				if(SingleVal > '0')
					SingleVal--;
				else 
					SingleVal = '9';
				ValueStr.setCharAt(BoxPos, SingleVal);
				break;
			case DOWN:
				if(SingleVal < '9')
					SingleVal++;
				else 
					SingleVal = '0';
				ValueStr.setCharAt(BoxPos, SingleVal);
				break;
			case OK:
				BoxPos++;
				SingleVal = ValueStr.charAt(BoxPos);
				if(BoxPos > MaxValueLenght - 1)
				{
					NewValue = ValueStr.toInt();
					if(NewValue >= Settings[SettingIndex].settingMin && NewValue <= Settings[SettingIndex].settingMax)
					{
						*(int32_t*)Settings[SettingIndex].settingVal = NewValue;
						WriteSetting(SettingIndex, NewValue);
						DrawPopUp("Valore salvato", 1000);
						ExitChangeValue = true;
					}
					else
						DrawPopUp("Valore errato", 1000);
				}
				break;
			case BACK:
				ExitChangeValue = true;
				break;
			default:
				break;
		}
		if(KeyPress != MAX_KEY)
		{
			ClearMenu();
		}				
	}
}

void ChangeEnum(uint8_t SettingIndex, bool isSwitch)
{
	bool ExitChangeEnum = false;
	uint8_t KeyPress = MAX_KEY, BoxPos = 0, MaxEnum = 0;
	int32_t EnumItem = *(int32_t*)Settings[SettingIndex].settingVal;
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);	
	// if(isSwitch)
	// {
		// EnumItem = 2;
		// MaxEnum = 2;
	// }
	// else
	// {
		MaxEnum = Settings[SettingIndex].settingMax;
	// }

	while(!ExitChangeEnum)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
		}
		if(!isSwitch)
		{
			Display.setTextColor(ILI9341_WHITE);
			Display.setFont(Arial_20_Bold);
			Display.setCursor(CENTER_ALIGN(Settings[SettingIndex].settingName), MENU_TITLE_POS);
			Display.print(Settings[SettingIndex].settingName);
			// Display.setFont(Arial_28_Bold);
			// if(TEXT_LENGHT(Settings[SettingIndex].enumPtr[EnumItem].enumName) > DISPLAY_WIDTH - 40)
			Display.setFont(Arial_18_Bold);
			Display.setCursor(CENTER_ALIGN(Settings[SettingIndex].enumPtr[EnumItem].enumName), CENTER_POS);
			Display.print(Settings[SettingIndex].enumPtr[EnumItem].enumName);
			Display.drawRoundRect(CENTER_ALIGN(Settings[SettingIndex].enumPtr[EnumItem].enumName) - 2 + (BoxPos * TEXT_LENGHT(Settings[SettingIndex].enumPtr[EnumItem].enumName)), 
									CENTER_POS - 2, 
									TEXT_LENGHT(Settings[SettingIndex].enumPtr[EnumItem].enumName) + 3, Display.fontCapHeight() + 5, 1, ILI9341_CYAN);
								  
		}
		// else
		// {
			// Display.setTextColor(ILI9341_WHITE);
			// Display.setFont(Arial_24_Bold);
			// Display.setCursor(CENTER_ALIGN("Associa allarme"), MENU_TITLE_POS);
			// Display.print("Associa allarme");
			// Display.setFont(Arial_28_Bold);
			// if(CENTER_ALIGN_BUTT(AlarmSwitchdEnum[EnumItem].enumName) + TEXT_LENGHT(AlarmSwitchdEnum[EnumItem].enumName) > DISPLAY_WIDTH - 40)
				// Display.setFont(Arial_16_Bold);
			// Display.setCursor(CENTER_ALIGN_BUTT(AlarmSwitchdEnum[EnumItem].enumName), CENTER_POS);
			// Display.print(AlarmSwitchdEnum[EnumItem].enumName);
			// Display.drawRoundRect(CENTER_ALIGN_BUTT(AlarmSwitchdEnum[EnumItem].enumName) - 2 + (BoxPos * TEXT_LENGHT(AlarmSwitchdEnum[EnumItem].enumName)), 
									// CENTER_POS - 2, 
									// TEXT_LENGHT(AlarmSwitchdEnum[EnumItem].enumName) + 3, Display.fontCapHeight() + 5, 1, ILI9341_CYAN);
		// }
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:
				if(EnumItem > 0)
					EnumItem--;
				else
					EnumItem = MaxEnum;
				break;
			case DOWN:
				if(EnumItem < MaxEnum)
					EnumItem++;
				else
					EnumItem = 0;
				break;
			case OK:
				if(!isSwitch)
				{
					*(int32_t*)Settings[SettingIndex].settingVal = EnumItem;
					WriteSetting(SettingIndex, EnumItem);
					switch(Settings[SettingIndex].enumPtr[EnumItem].enumType)
					{
						case BOOLEAN_TYPE:
							*(bool *)Settings[SettingIndex].enumPtr[EnumItem].enumValuePtr = (bool)EnumItem;
							break;
						case UINT8_TYPE:
							*(uint8_t *)Settings[SettingIndex].enumPtr[EnumItem].enumValuePtr = (uint8_t)EnumItem;
							break;
						default:
							break;
					}
					DrawPopUp("Valore salvato", 1000);
				}
				// else
				// {
					// if(EnumItem != 2)
					// {
						// *(uint8_t *)AlarmSwitchdEnum[EnumItem].enumValuePtr = (uint8_t)EnumItem;
						// DrawPopUp("Allarme impostato", 1000);
						// Switch.haveAlarm = true;
						// // Switch.alarmShutDown = true;
					// }
					// else
					// {
						// Switch.haveAlarm = false;
						// Switch.alarmShutDown = false;						
					// }
				// }
				ExitChangeEnum = true;
				break;
			case BACK:
				ExitChangeEnum = true;
				break;
			default:
				break;
		}
		if(KeyPress != MAX_KEY)
		{
			ClearMenu();
		}				
	}	
}

void DrawSettingPage()
{
	bool ExitSetting = false, ChangeSettingPage = false;
	uint8_t TopItem = 0, Item = 0, OldItem = 0, KeyPress = MAX_KEY;
	String NumPage = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);
	while(!ExitSetting)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
		}
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_12);
		NumPage = String(Item + 1) + "/" + String(MAX_SETTINGS);
		Display.setCursor(RIGHT_ALIGN(NumPage.c_str()) - 10, MENU_TITLE_POS + 10);
		Display.print(NumPage.c_str());
		Display.setFont(Arial_24_Bold);
		Display.setCursor(CENTER_ALIGN("Impostazioni"), MENU_TITLE_POS);
		Display.print("Impostazioni");
		Display.setFont(Arial_18);
		for(int i = 0; i < MAX_MENU_VIEW_ITEMS; i++)
		{
			int NewItem = TopItem + i;
			if(NewItem >= MAX_SETTINGS)
				break;
			if(NewItem == Item)
			{
				Display.setTextColor(ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(Settings[NewItem].settingName), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + Display.fontCapHeight(),
										TEXT_LENGHT(Settings[NewItem].settingName), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(Settings[NewItem].settingName), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 1 + Display.fontCapHeight(),
										TEXT_LENGHT(Settings[NewItem].settingName), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(Settings[NewItem].settingName), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 2 + Display.fontCapHeight(),
										TEXT_LENGHT(Settings[NewItem].settingName), ILI9341_GREENYELLOW);
			}
			else
			{
				Display.setTextColor(ILI9341_WHITE);
			}
			Display.setCursor(CENTER_ALIGN(Settings[NewItem].settingName), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)));
			Display.print(Settings[NewItem].settingName);
		}
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				if(Item > 0)
					Item--;
				else
					Item = MAX_SETTINGS - 1;
				break;
			case DOWN:
				if(Item < MAX_SETTINGS - 1)
					Item++;
				else
					Item = 0;			
				break;
			case OK:	
				ChangeSettingPage = true;
				break;
			case BACK:
				AnalyzerPage = MAIN_MENU;
				ExitSetting = true;
				break;
			default:
				break;
		}
	
		if(OldItem != Item)
		{
			ClearMenu();
			Display.fillRect(RIGHT_ALIGN(NumPage.c_str()) - 14, MENU_TITLE_POS + 8, 50, 16, ILI9341_BLACK);
			OldItem = Item;
		}	
		if(Item > MAX_MENU_VIEW_ITEMS - 1)
		{
			TopItem = Item - (MAX_MENU_VIEW_ITEMS - 1);
		}
		else
			TopItem = 0;
		if(ChangeSettingPage)
		{
			ChangeSettingPage = false;
			switch(Settings[Item].type)
			{
				case DATE_TYPE:
					if(Item == CHANGE_TIME)
						ChangeTimeDate(true, false);
					else
						ChangeTimeDate(false, false);
					break;
				case VALUE_TYPE:
					ChangeValue(Item);
					break;
				case ENUM_TYPE:
					ChangeEnum(Item, false);
					break;
				default:
					break;
			}
			ClearDisplay(false);
		}
	}
}

void DrawResetPage()
{
	bool ExitSetting = false, ConfirmReset = false;
	uint8_t TopItem = 0, ResetItem = 0, OldItem = 0, KeyPress = MAX_KEY;
	String NumPage = "";
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_OK);
	while(!ExitSetting)
	{
		DoTasks();
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_OK);
		}
		Display.setTextColor(ILI9341_WHITE);
		Display.setFont(Arial_12);
		NumPage = String(ResetItem + 1) + "/" + String(MAX_RESETS);
		Display.setCursor(RIGHT_ALIGN(NumPage.c_str()) - 10, MENU_TITLE_POS + 10);
		Display.print(NumPage.c_str());
		Display.setFont(Arial_24_Bold);
		Display.setCursor(CENTER_ALIGN("Reset"), MENU_TITLE_POS);
		Display.print("Reset");
		Display.setFont(Arial_18);
		for(int i = 0; i < MAX_MENU_VIEW_ITEMS; i++)
		{
			int NewItem = TopItem + i;
			if(NewItem >= MAX_RESETS)
				break;
			if(NewItem == ResetItem)
			{
				Display.setTextColor(ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(ResetsTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + Display.fontCapHeight(),
										TEXT_LENGHT(ResetsTitle[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(ResetsTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 1 + Display.fontCapHeight(),
										TEXT_LENGHT(ResetsTitle[NewItem]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(ResetsTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 2 + Display.fontCapHeight(),
										TEXT_LENGHT(ResetsTitle[NewItem]), ILI9341_GREENYELLOW);
			}
			else
			{
				Display.setTextColor(ILI9341_WHITE);
			}
			Display.setCursor(CENTER_ALIGN(ResetsTitle[NewItem]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)));
			Display.print(ResetsTitle[NewItem]);
		}
		KeyPress = ButtonPressed();
		switch(KeyPress)
		{
			case UP:	
				if(ResetItem > 0)
					ResetItem--;
				else
					ResetItem = MAX_RESETS - 1;
				break;
			case DOWN:
				if(ResetItem < MAX_RESETS - 1)
					ResetItem++;
				else
					ResetItem = 0;			
				break;
			case OK:
				ConfirmReset = DrawAskPopUp("Confermare");			
				switch(ResetItem)
				{
					case RESET_DFLT:
						if(ConfirmReset)
						{
							WriteResetDeflt();
							delay(10);
							CPU_RESTART;
						}
					case RESTART:
						if(ConfirmReset)
						{
							delay(10);
							CPU_RESTART;
						}
						break;
					case RESET_MAX_MIN:
						if(ConfirmReset)
							ResetMaxMin();
						break;
					case RESET_AVG:
						if(ConfirmReset)
							ResetAvg();					
						break;
					case RESET_ENERGIES:
						if(ConfirmReset)
							ResetEnergies();					
						break;
					case RESET_LOG:
						ResetLogs();
						break;
					case RESET_SWITCH_STAT:
						ResetSwitchStatistics();
						WriteSwitchStatistics(true);
						break;
					default:
						break;
				}
				break;
			case BACK:
				AnalyzerPage = MAIN_MENU;
				ExitSetting = true;
				break;
			default:
				break;
		}
	
		if(OldItem != ResetItem)
		{
			ClearMenu();
			Display.fillRect(RIGHT_ALIGN(NumPage.c_str()) - 14, MENU_TITLE_POS + 8, 50, 16, ILI9341_BLACK);
			OldItem = ResetItem;
		}	
		if(ResetItem > MAX_MENU_VIEW_ITEMS - 1)
		{
			TopItem = ResetItem - (MAX_MENU_VIEW_ITEMS - 1);
		}
		else
			TopItem = 0;
	}
}
