#ifndef DISPLAY_H
#define DISPLAY_H

#include <ILI9341_t3.h>
#include <font_Arial.h> // from ILI9341_t3
#include <font_ArialBold.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Arduino.h>
#include "Measures.h"

#define TOUCH_CS_PIN 	  6
#define TFT_DC  		  9
#define TFT_CS 			 10


#define DISPLAY_WIDTH		(Display.width())
#define DISPLAY_HIGH		(Display.height())

#define TEXT_LENGHT(Str)			(Display.strPixelLen(Str))

#define LEFT_ALIGN				0
#define CENTER_ALIGN(Str)		((DISPLAY_WIDTH - Display.strPixelLen(Str)) / 2)
#define RIGHT_ALIGN(Str)     	(DISPLAY_WIDTH - Display.strPixelLen(Str))
#define CENTER_ALIGN_BUTT(Str)	((DISPLAY_WIDTH - 40 - Display.strPixelLen(Str)) / 2)

#define TOP_POS					0
#define CENTER_POS				(DISPLAY_HIGH / 2)
#define BOTTOM_POS				(DISPLAY_HIGH - Display.fontCapHeight())

#define NAV_BUTT_X_START		280
#define NAV_BUTT_Y_START		 60
#define NAV_BUTT_WIDTH		     40
#define NAV_BUTT_HIGH		     40
#define NAV_BUTT_INTERLINE		 20

#define MENU_TITLE_POS			(TOP_POS + 20)
#define MEASURE_POS				(TOP_POS + 25)

#define MAX_MENU_VIEW_ITEMS		4

#define MENU_ITEMS_POS			(TOP_POS + 70)

#define GRAPHIC_X				0
#define GRAPHIC_Y				(MENU_TITLE_POS + 28)
#define GRAPHIC_W				N_SAMPLE //(DISPLAY_WIDTH - (DISPLAY_WIDTH - NAV_BUTT_X_START) - 2)
#define GRAPHIC_H				190
#define GRAPHIC_HALF			(GRAPHIC_Y + (GRAPHIC_H / 2))

#define LOG_GRAPHIC_Y			(MENU_TITLE_POS + 50)
#define LOG_GRAPHIC_W			256 
#define LOG_GRAPHIC_X			((DISPLAY_WIDTH - 40 - LOG_GRAPHIC_W) / 2)
#define LOG_GRAPHIC_H			90
#define LOG_GRAPHIC_HALF		(LOG_GRAPHIC_Y + (LOG_GRAPHIC_H / 2))

// // Color definitions
// #define ILI9341_BLACK       0x0000      /*   0,   0,   0 */
// #define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
// #define ILI9341_DARKGREEN   0x03E0      /*   0, 128,   0 */
// #define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
// #define ILI9341_MAROON      0x7800      /* 128,   0,   0 */
// #define ILI9341_PURPLE      0x780F      /* 128,   0, 128 */
// #define ILI9341_OLIVE       0x7BE0      /* 128, 128,   0 */
// #define ILI9341_LIGHTGREY   0xC618      /* 192, 192, 192 */
// #define ILI9341_DARKGREY    0x7BEF      /* 128, 128, 128 */
// #define ILI9341_BLUE        0x001F      /*   0,   0, 255 */
// #define ILI9341_GREEN       0x07E0      /*   0, 255,   0 */
// #define ILI9341_CYAN        0x07FF      /*   0, 255, 255 */
// #define ILI9341_RED         0xF800      /* 255,   0,   0 */
// #define ILI9341_MAGENTA     0xF81F      /* 255,   0, 255 */
// #define ILI9341_YELLOW      0xFFE0      /* 255, 255,   0 */
// #define ILI9341_WHITE       0xFFFF      /* 255, 255, 255 */
// #define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
// #define ILI9341_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
// #define ILI9341_PINK        0xF81F
// #define CL(_r,_g,_b) ((((_r)&0xF8)<<8)|(((_g)&0xFC)<<3)|((_b)>>3))

// Font arial 
// extern const ILI9341_t3_font_t Arial_8;  char lenght = 4px            extern const ILI9341_t3_font_t Arial_8_Bold;
// extern const ILI9341_t3_font_t Arial_9;                               extern const ILI9341_t3_font_t Arial_9_Bold;
// extern const ILI9341_t3_font_t Arial_10; char lenght = 5px            extern const ILI9341_t3_font_t Arial_10_Bold;
// extern const ILI9341_t3_font_t Arial_11;                              extern const ILI9341_t3_font_t Arial_11_Bold;
// extern const ILI9341_t3_font_t Arial_12;                              extern const ILI9341_t3_font_t Arial_12_Bold;
// extern const ILI9341_t3_font_t Arial_13;                              extern const ILI9341_t3_font_t Arial_13_Bold;
// extern const ILI9341_t3_font_t Arial_14;                              extern const ILI9341_t3_font_t Arial_14_Bold;
// extern const ILI9341_t3_font_t Arial_16;                              extern const ILI9341_t3_font_t Arial_16_Bold;
// extern const ILI9341_t3_font_t Arial_18;                              extern const ILI9341_t3_font_t Arial_18_Bold;
// extern const ILI9341_t3_font_t Arial_20; char lenght = 9px            extern const ILI9341_t3_font_t Arial_20_Bold;
// extern const ILI9341_t3_font_t Arial_24;                              extern const ILI9341_t3_font_t Arial_24_Bold;
// extern const ILI9341_t3_font_t Arial_28;                              extern const ILI9341_t3_font_t Arial_28_Bold;
// extern const ILI9341_t3_font_t Arial_32;                              extern const ILI9341_t3_font_t Arial_32_Bold;
// extern const ILI9341_t3_font_t Arial_40;                              extern const ILI9341_t3_font_t Arial_40_Bold;
// extern const ILI9341_t3_font_t Arial_48;                              extern const ILI9341_t3_font_t Arial_48_Bold;
// extern const ILI9341_t3_font_t Arial_60;                              extern const ILI9341_t3_font_t Arial_60_Bold;
// extern const ILI9341_t3_font_t Arial_72;                              extern const ILI9341_t3_font_t Arial_72_Bold;
// extern const ILI9341_t3_font_t Arial_96;                              extern const ILI9341_t3_font_t Arial_96_Bold;


typedef enum
{
	D_LANDSCAPE_1 = 0,
	D_LONG_1,
	D_LONG_2,
	D_LANDSCAPE_2,
	D_MAX_ROTATION
}DISPLAY_ROTATIONS;

typedef enum
{
	T_LONG_1 = 0,
	T_LANDSCAPE_1,
	T_LONG_2,
	T_LANDSCAPE_2,
	T_MAX_ROTATION
}TOUCH_ROTATIONS;

typedef struct
{
	TS_Point touchCoord;
	uint32_t display_x;
	uint32_t display_y;
	uint32_t max_x;
	uint32_t max_y;
	uint32_t min_analog_x;
	uint32_t min_analog_y;	
	bool 	 isTouched;
	bool 	 isLongTouched;
	uint8_t  displayRotation;
}DISPLAY_VAR;

typedef struct 
{
	double value;
	char   odg;
	double rescale;
}RANGES;

extern XPT2046_Touchscreen Touch;
extern DISPLAY_VAR DisplayParam;
extern const RANGES RangeTab[];

uint8_t SearchRange(double Value2Search);
void DisplaySetup(uint8_t Rotation);
void DisplaySetRotation(uint8_t Rotation);
void DrawPopUp(char *Msg, uint16_t Delay);
void DrawAlarmPopUp(char *Msg, uint16_t Delay);
void DrawInfoPopUp(char *Msg, uint16_t Delay);

void DrawMainMenu();
void DrawMeasurePage();
void DrawGraphicsPage();
void DrawLogsList();
void DrawLogGraphic();
void DrawLogMenu();
void DrawAlarmPage();
void DrawRelePage();
void DrawSettingPage();
void DrawResetPage();


void ChangeTimeDate(bool isTime, bool isSwitch);
void ChangeEnum(uint8_t SettingIndex, bool isSwitch);
#endif