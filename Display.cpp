#include "PhaseAnalyzer.h"
#include "Display.h"
#include "Touch.h"
#include "Measures.h"

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

enum
{
	LINE_1 = 0,
	LINE_2,
	LINE_3,
	MAX_LINES
};

typedef struct
{
	double *actualMeasure;
	double *max;
	double *min;
	String measureUnit;
}MEASURE_LINE;

typedef struct
{
	MEASURE_LINE	measureL1;
	MEASURE_LINE 	measureL2;
	MEASURE_LINE    measureL3;
}MEASURE_PAGE;


typedef struct 
{
	double value;
	char   odg;
	double rescale;
}RANGES;

const MEASURE_PAGE MeasureTab[MAX_MEASURE_PAGES] = 
{
	{{&Current.actual, &Current.max	    , &Current.min, "A" }, {&Voltage.actual, &Voltage.max    , &Voltage.min,   "V" } , {&Pf.actual    , &Pf.max     , &Pf.min  , ""   }},
	{{&PAtt.actual   , &PAtt.max   	    , &PAtt.min   , "W" }, {&PRea.actual   , &PRea.max       , &PRea.min   , "VAr" } , {&PApp.actual  , &PApp.max   , &PApp.min, "VA" }},
	{{&Current.avg   , &Current.maxAvg  , NULL        , "A" }, {&Voltage.avg   , &Voltage.maxAvg , NULL        ,   "V" } , {&Pf.avg       , &Pf.maxAvg  , NULL  , ""      }},
	{{&PAtt.avg      , &PAtt.maxAvg     , NULL        , "W" }, {&PRea.avg      , &PRea.maxAvg    , NULL        , "VAr" } , {&PApp.avg     , &PApp.maxAvg, NULL  , "VA"    }},
	{{&EnAtt.actual  , NULL        		, NULL        , "Wh"}, {&EnRea.actual  , NULL            , NULL        , "VArh"} , {&EnApp.actual , NULL        , NULL  , "VAh"   }},
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

XPT2046_Touchscreen Touch(CS_PIN);
ILI9341_t3 Display = ILI9341_t3(TFT_CS, TFT_DC);
DISPLAY_VAR DisplayParam;

Chrono DisplayRefresh, MeasureRefresh;

NAV_BUTTON_COORD Up = {NAV_BUTT_X_START, NAV_BUTT_Y_START, NAV_BUTT_WIDTH, NAV_BUTT_HIGH, ILI9341_RED};
NAV_BUTTON_COORD Down = {NAV_BUTT_X_START, NAV_BUTT_Y_START + (2 * (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE)), NAV_BUTT_WIDTH, NAV_BUTT_HIGH, ILI9341_RED};
NAV_BUTTON_COORD Ok_Back = {NAV_BUTT_X_START, NAV_BUTT_Y_START + (NAV_BUTT_HIGH + NAV_BUTT_INTERLINE), NAV_BUTT_WIDTH, NAV_BUTT_HIGH, ILI9341_GREEN};
	


const char *MenuVoices[MAX_MENU_ITEMS] = 
{
	"Misure",
	"Grafici",
	"Log",
	"Impostazioni",
};

static uint8_t SearchRange(double Value2Search)
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
				Range = 0;
			break;
		}
	}

	return Range;
}

void DoTasks()
{
	GetMeasure();
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

void DisplaySetup(uint8_t Rotation)
{
	Display.begin();
	ClearDisplay(false);
	Touch.begin();
	DisplaySetRotation(Rotation);
	DisplayParam.displayRotation = Rotation;
}


static void ClearMenu()
{
	Display.fillRect(0, MENU_TITLE_POS + 25, DISPLAY_WIDTH - (DISPLAY_WIDTH - NAV_BUTT_X_START) - 2, DISPLAY_HIGH - (MENU_TITLE_POS + 30), ILI9341_BLACK);
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

static void DrawTopInfo()
{
	Display.setTextColor(ILI9341_WHITE);
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
				Display.drawFastHLine(CENTER_ALIGN(MenuVoices[i]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + Display.fontCapHeight(),
										TEXT_LENGHT(MenuVoices[i]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(MenuVoices[i]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 1 + Display.fontCapHeight(),
										TEXT_LENGHT(MenuVoices[i]), ILI9341_GREENYELLOW);
				Display.drawFastHLine(CENTER_ALIGN(MenuVoices[i]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)) + 2 + Display.fontCapHeight(),
										TEXT_LENGHT(MenuVoices[i]), ILI9341_GREENYELLOW);
			}
			else
			{
				Display.setTextColor(ILI9341_WHITE);
			}
			Display.setCursor(CENTER_ALIGN(MenuVoices[i]), MENU_ITEMS_POS + (i * (Display.fontCapHeight() + 25)));
			Display.print(MenuVoices[i]);
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
			ClearMenu();
			DBG("OldItem = " + String(OldItem) + " Item = " + String(Item));
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


static void FormatMeasure(double *Measure2Format, String *Measure, String *Udm, bool isPF)
{
	double MeasureCpy = *Measure2Format;
	uint8_t Range = 0;
	if(!isPF)
	{
		Range = SearchRange(MeasureCpy);
		MeasureCpy *= RangeTab[Range].rescale;
		if(RangeTab[Range].odg != ' ')
		{
			*Udm = String(RangeTab[Range].odg);
		}
	}
	*Measure = String(MeasureCpy, 3);
}

static void FormatMaxMin(double *Measure2Format, String *Measure, String *Udm, bool isPF)
{
	double MeasureCpy = *Measure2Format;
	uint8_t Range = 0;
	char MaxMinStr[6];
	String MeasureString = "";
	if(!isPF)
	{
		Range = SearchRange(MeasureCpy);
		MeasureCpy *= RangeTab[Range].rescale;
		if(RangeTab[Range].odg != ' ')
		{
			*Udm = String(RangeTab[Range].odg);
		}		
	}
	MeasureString = String(MeasureCpy, 3);
	MeasureString.toCharArray(MaxMinStr, 6);
	*Measure = String(MaxMinStr);
}


static void DrawMeasureLine(uint8_t MeasurePage, uint8_t Line)
{
	uint16_t MeasureLenght = 0;
	String MainMeasure = "", Max = "", Min = "", Udm = "";
	const MEASURE_LINE *MeasureTabLine;
	bool isPF = false;
	switch(Line)
	{
		case LINE_1:
			MeasureTabLine = &MeasureTab[MeasurePage].measureL1;
			break;
		case LINE_2:
			MeasureTabLine = &MeasureTab[MeasurePage].measureL2;
			break;
		case LINE_3:
			if(MeasurePage == V_I_PF || MeasurePage == V_I_PF_AVG)
				isPF = true;
			MeasureTabLine = &MeasureTab[MeasurePage].measureL3;
			break;
		default:
			break;
	}
	
	FormatMeasure(MeasureTabLine->actualMeasure, &MainMeasure, &Udm, isPF);
	Udm += MeasureTabLine->measureUnit;
	
	Display.setTextColor(ILI9341_WHITE);
	Display.setFont(Arial_28_Bold);	
	Display.setCursor(55, MEASURE_POS + 40 + (Line * 65));
	Display.print(MainMeasure.c_str());
	
	MeasureLenght = TEXT_LENGHT(MainMeasure.c_str());
	Display.setFont(Arial_14_Bold);	
	Display.setCursor(55 + MeasureLenght + 2, MEASURE_POS + 55 + (Line * 65));
	Display.print(Udm.c_str());
	
	Display.setFont(Arial_9_Bold);	
	Display.setCursor(0, MEASURE_POS + 30 + (Line * 65));
	Display.print("Max");
	
	Udm = "";
	if(MeasureTabLine->max != NULL)
		FormatMaxMin(MeasureTabLine->max, &Max, &Udm, isPF);
	else
		Max = "----";
	Max += Udm;
	Display.setFont(Arial_10_Bold);
	Display.setCursor(0, MEASURE_POS + 42 + (Line * 65));
	Display.print(Max.c_str());	

	Udm = "";	
	Display.setFont(Arial_9_Bold);
	Display.setCursor(0, MEASURE_POS + 54 + (Line * 65));
	Display.print("Min");

	if(MeasureTabLine->min != NULL)
		FormatMaxMin(MeasureTabLine->min, &Min, &Udm, isPF);
	else
		Min = "----";
	Min += Udm;
	Display.setFont(Arial_10_Bold);
	Display.setCursor(0, MEASURE_POS + 66 + (Line * 65));
	Display.print(Min.c_str());	
	
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
		Display.setCursor(CENTER_ALIGN("Misure"), MENU_TITLE_POS);
		Display.print("Misure");
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
				DBG("Ok premuto");
				break;
			case BACK:
				AnalyzerPage = MAIN_MENU;
				ExitMeasures = true;
				DBG("Back premuto");
				break;
			default:
				break;
		}
	
		if(OldItem != Item)
		{
			Display.fillRect(RIGHT_ALIGN(NumPage.c_str()) - 24, MENU_TITLE_POS + 8, 40, 16, ILI9341_BLACK);
			OldItem = Item;
		}
	}		
}

void DrawGraphicsPage()
{
	bool ExitGraphics = false;
	uint8_t TopItem = 0, Item = 0, OldItem = 0, KeyPress = MAX_KEY;
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_BACK);	
	while(!ExitGraphics)
	{
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_BACK);
		}
		Display.setFont(Arial_24_Bold);
		Display.setTextColor(ILI9341_WHITE);
		Display.setCursor(CENTER_ALIGN("Grafici"), MENU_TITLE_POS);
		Display.print("Grafici");
		
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
				AnalyzerPage = MAIN_MENU;
				ExitGraphics = true;
				DBG("Back premuto");
				break;
			default:
				break;
		}
	
		if(OldItem != Item)
		{
			ClearMenu();
			DBG("OldItem = " + String(OldItem) + " Item = " + String(Item));
			OldItem = Item;
		}
	}		
}

void DrawLogsPage()
{
	bool ExitLogs = false;
	uint8_t TopItem = 0, Item = 0, OldItem = 0, KeyPress = MAX_KEY;
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_BACK);	
	while(!ExitLogs)
	{
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_BACK);
		}
		Display.setFont(Arial_24_Bold);
		Display.setTextColor(ILI9341_WHITE);
		Display.setCursor(CENTER_ALIGN("Log"), MENU_TITLE_POS);
		Display.print("Log");
		
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
				AnalyzerPage = MAIN_MENU;
				ExitLogs = true;
				DBG("Back premuto");
				break;
			default:
				break;
		}
	
		if(OldItem != Item)
		{
			ClearMenu();
			DBG("OldItem = " + String(OldItem) + " Item = " + String(Item));
			OldItem = Item;
		}	
	}
}


void DrawSettingPage()
{
	bool ExitSetting = false;
	uint8_t TopItem = 0, Item = 0, OldItem = 0, KeyPress = MAX_KEY;
	ClearDisplay(false);
	DisplayRefresh.restart();
	DrawTopInfo();
	DrawNavButtons(DRAW_BACK);
	while(!ExitSetting)
	{
		if(DisplayRefresh.hasPassed(500, true))
		{
			DrawTopInfo();
			DrawNavButtons(DRAW_BACK);
		}
		Display.setFont(Arial_24_Bold);
		Display.setTextColor(ILI9341_WHITE);
		Display.setCursor(CENTER_ALIGN("Impostazioni"), MENU_TITLE_POS);
		Display.print("Impostazioni");
		
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
				AnalyzerPage = MAIN_MENU;
				ExitSetting = true;
				DBG("Back premuto");
				break;
			default:
				break;
		}
	
		if(OldItem != Item)
		{
			ClearMenu();
			DBG("OldItem = " + String(OldItem) + " Item = " + String(Item));
			OldItem = Item;
		}	
	}
}
