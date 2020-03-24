#include <ILI9341_t3.h>
#include <font_Arial.h> // from ILI9341_t3
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

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
// extern const ILI9341_t3_font_t Arial_8;
// extern const ILI9341_t3_font_t Arial_9;
// extern const ILI9341_t3_font_t Arial_10;
// extern const ILI9341_t3_font_t Arial_11;
// extern const ILI9341_t3_font_t Arial_12;
// extern const ILI9341_t3_font_t Arial_13;
// extern const ILI9341_t3_font_t Arial_14;
// extern const ILI9341_t3_font_t Arial_16;
// extern const ILI9341_t3_font_t Arial_18;
// extern const ILI9341_t3_font_t Arial_20;
// extern const ILI9341_t3_font_t Arial_24;
// extern const ILI9341_t3_font_t Arial_28;
// extern const ILI9341_t3_font_t Arial_32;
// extern const ILI9341_t3_font_t Arial_40;
// extern const ILI9341_t3_font_t Arial_48;
// extern const ILI9341_t3_font_t Arial_60;
// extern const ILI9341_t3_font_t Arial_72;
// extern const ILI9341_t3_font_t Arial_96;






#define CS_PIN  8
#define TFT_DC  9
#define TFT_CS 10

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
	uint8_t  displayRotation;
}DISPLAY_VAR;


XPT2046_Touchscreen Touch(CS_PIN);
ILI9341_t3 Display = ILI9341_t3(TFT_CS, TFT_DC);
DISPLAY_VAR DisplayParam;

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

static void DisplaySetup(uint8_t Rotation)
{
	Display.begin();
	ClearDisplay(false);
	Touch.begin();
	DisplaySetRotation(Rotation);
	DisplayParam.displayRotation = Rotation;
}

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

void setup()
{
	Serial.begin(9600);
	DisplaySetup(D_LANDSCAPE_2);
}

void loop()
{
	DisplayParam.isTouched = Touch.touched();
	if(DisplayParam.isTouched)
	{
		ReScaleDisplayCoord(Touch.getPoint());
		Display.fillRect(100, 150, 140, 60, ILI9341_BLACK);
		Display.setTextColor(ILI9341_GREEN);
		Display.setFont(Arial_24);
		Display.setCursor(100, 150);
		Display.print("X = ");
		Display.print(DisplayParam.display_x);
		Display.setCursor(100, 180);
		Display.print("Y = ");
		Display.print(DisplayParam.display_y);
	}
	else
		ClearDisplay(false);
	delay(50);
}
