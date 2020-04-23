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
	RECEIVE_SIM_I,
	RECEIVE_SIM_V,
	RECEIVE_SIM_DELAY_I,
	RECEIVE_SIM_DELAY_V,
	MAX_STATES
}BT_STATES;

const BT_RESPONSES_DEF BtResponseTab[MAX_STATES] = 
{
	// {"*B"   , SEND_WELCOME     , false },
	{"ON"   			, SWICH_ON_RELE    		, true  },
	{"OFF"  			, SWICH_OFF_RELE   		, true  },
	{"*a"   			, SEND_CURRENT     		, false },
	{"*b"   			, SEND_VOLTAGE     		, false },
	{"*c"   			, SEND_PF  	       		, false },
	{"*d"   			, SEND_PATT        		, false },
	{"*e"   			, SEND_PREA  	   		, false },
	{"*f"   			, SEND_PAPP        		, false },
	{"*g"   			, SEND_EN_ATT      		, false },
	{"*h"   			, SEND_EN_REA      		, false },
	{"*i"   			, SEND_EN_APP      		, false },
	{"SIM_I"   			, RECEIVE_SIM_I         , true  },
	{"SIM_V"   			, RECEIVE_SIM_V         , true  },
	{"SIM_DELAY_I"      , RECEIVE_SIM_DELAY_I   , true  },
	{"SIM_DELAY_V"      , RECEIVE_SIM_DELAY_V   , true  },
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
			break;
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
			if(BtResponseTab[TabIndex].stringKey.equals(Req))
			{
				break;
			}	
			else if(Req.length() > 3)
			{
				char ReqChar[Req.length()];
				Req.toCharArray(ReqChar, Req.length());
				if(ReqChar[4] == 'I')
				{
					TabIndex = RECEIVE_SIM_I;
					Req = String(ReqChar[5]) + (ReqChar[6] <= '9' ? String(ReqChar[6]) : "");
				}
				else if(ReqChar[4] == 'V')
				{
					TabIndex = RECEIVE_SIM_V;
					Req = String(ReqChar[5]) + (ReqChar[6] <= '9' ? String(ReqChar[6]) : "") + (ReqChar[7] <= '9' ? String(ReqChar[7]) : "");					
				}
				else
				{
					if(ReqChar[10] == 'I')
					{
						TabIndex = RECEIVE_SIM_DELAY_I;
						Req = String(ReqChar[11]) + (ReqChar[12] <= '9' ? String(ReqChar[12]) : "") + (ReqChar[13] <= '9' ? String(ReqChar[13]) : "");						
					}
					else
					{
						TabIndex = RECEIVE_SIM_DELAY_V;
						Req = String(ReqChar[11]) + (ReqChar[12] <= '9' ? String(ReqChar[12]) : "") + (ReqChar[13] <= '9' ? String(ReqChar[13]) : "");	
					}
				}
				break;
			}	
		}
	}
	if(TabIndex != MAX_STATES)
	{
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
			case RECEIVE_SIM_I:
				SimCurrent = (double)(Req.toInt());
				break;
			case RECEIVE_SIM_V:
				SimVoltage = (double)(Req.toInt());
				break;
			case RECEIVE_SIM_DELAY_I:
				SimDelayI = (double)(Req.toInt());
				break;
			case RECEIVE_SIM_DELAY_V:
				SimDelayV = (double)(Req.toInt());
				break;
			default:
				break;
		}
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
	// DBG("Controllo BT");
	if(digitalRead(BT_STATE_PIN))
		BtDeviceConnected = true;
	else
		BtDeviceConnected = false;
}
