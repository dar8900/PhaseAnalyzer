#include "PhaseAnalyzer.h"
#include "BT.h"
#include "Rele.h"
#include "Measures.h"
#include "Display.h"

#define BT_STATE_PIN 4
#define SERIAL_HW	Serial1

typedef struct
{
	String 		stringKey;
	uint16_t	intKey;
	bool 		doAction;
}BT_RESPONSES_DEF;

typedef enum
{
	// SEND_WELCOME
	SWICH_ON_RELE = 0,
	SWICH_OFF_RELE,
	SEND_CURRENT,
	SEND_VOLTAGE,
	SEND_PF,
	SEND_PATT,
	SEND_PREA,
	SEND_PAPP,
	SEND_EN_ATT,
	SEND_EN_REA,
	SEND_EN_APP,
	MAX_STATES
}BT_STATES;

const BT_RESPONSES_DEF BtResponseTab[MAX_STATES] = 
{
	// {"*B"   , SEND_WELCOME     , false },
	{"ON"   , SWICH_ON_RELE    , true  },
	{"OFF"  , SWICH_OFF_RELE   , true  },
	{"*I"   , SEND_CURRENT     , false },
	{"*V"   , SEND_VOLTAGE     , false },
	{"*P"   , SEND_PF  	       , false },
	{"*A"   , SEND_PATT        , false },
	{"*r"   , SEND_PREA  	   , false },
	{"*p"   , SEND_PAPP        , false },
	{"*a"   , SEND_EN_ATT      , false },
	{"*R"   , SEND_EN_REA      , false },
	{"*q"   , SEND_EN_APP      , false },
};

bool BtDeviceConnected;

static void SendTextSM(uint16_t TabIndex)
{
	bool IsAMeasure = false, IsToFormat = true;
	String Text2Send = BtResponseTab[TabIndex].stringKey, Udm = "";
	double Measure2Send = 0.0;
	uint8_t Range = 9;
	switch(BtResponseTab[TabIndex].intKey)
	{
		// case SEND_WELCOME:
			// Text2Send += "Phase Analyzer";
			// break;
		case SEND_CURRENT:
			Measure2Send = Current.actual;
			Udm = "A";
			IsAMeasure = true;
			break;
		case SEND_VOLTAGE:
			Measure2Send = Voltage.actual;
			Udm = "V";
			IsAMeasure = true;
			break;		
		case SEND_PF:  	
			Measure2Send = Pf.actual;
			Udm = "";
			IsAMeasure = true;
			IsToFormat = false;
			break;		
		case SEND_PATT:   
			Measure2Send = PAtt.actual;
			Udm = "W";
			IsAMeasure = true;
			break;		
		case SEND_PREA:  
			Measure2Send = PRea.actual;
			Udm = "VAr";
			IsAMeasure = true;
			break;
		case SEND_PAPP:
			Measure2Send = PApp.actual;
			Udm = "VA";
			IsAMeasure = true;
		case SEND_EN_ATT:   
			Measure2Send = EnAtt.actual;
			Udm = "Wh";
			IsAMeasure = true;
			break;		
		case SEND_EN_REA:  
			Measure2Send = EnRea.actual;
			Udm = "VArh";
			IsAMeasure = true;
			break;
		case SEND_EN_APP:
			Measure2Send = EnApp.actual;
			Udm = "VAh";
			IsAMeasure = true;			
			break;
		default:
			break;
	}
	if(IsAMeasure)
	{
		if(IsToFormat)
		{
			Range = SearchRange(Measure2Send);
			Measure2Send *= RangeTab[Range].rescale;
		}
		Text2Send += String(Measure2Send, 3) + " " + String(RangeTab[Range].odg) + Udm;		
	}
	Text2Send += "*";
	SERIAL_HW.println(Text2Send);
}

static void ActionSM(String Req)
{ 
	int TabIndex = 0;
	for(TabIndex = 0; TabIndex < MAX_STATES; TabIndex++)
	{
		if(!BtResponseTab[TabIndex].doAction)
			continue;
		else
		{
			if(BtResponseTab[TabIndex].stringKey == Req)
			{
				break;
			}
		}
	}
	switch(BtResponseTab[TabIndex].intKey)
	{
		case SWICH_ON_RELE:
			if(!Switch.haveTimer && !Switch.alarmShutDown)
				Switch.isActive = true;
			break;
		case SWICH_OFF_RELE:
			if(!Switch.haveTimer)
				Switch.isActive = false;
			else
			{
				Switch.haveTimer = false;
				Switch.timerDuration = 0;
				Switch.isActive = false;
			}
			break;
		default:
			break;
	}
}


void BtTransaction()
{
	int ReqTabIndex = 0;
	String Req = "";
	for(ReqTabIndex = 0; ReqTabIndex < MAX_STATES; ReqTabIndex++)
	{
		Req = "";
		if(BtResponseTab[ReqTabIndex].doAction)
		{
			if(SERIAL_HW.available())
			{
				while(SERIAL_HW.available() > 0)
				{
					Req += String(char(SERIAL_HW.read()));
				}
			}
			if(Req != "")
			{
				ActionSM(Req);
			}
		}
		else
		{
			SendTextSM(ReqTabIndex);
		}
	}
}


void BtInit()
{
	bool isRisp = false;
	int baudindex = 0;
	SERIAL_HW.begin(9600);
	pinMode(BT_STATE_PIN, INPUT);
	CheckBtDevConn();
}

void CheckBtDevConn()
{
	if(digitalRead(BT_STATE_PIN))
		BtDeviceConnected = true;
	else
		BtDeviceConnected = false;
}
