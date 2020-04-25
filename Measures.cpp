#include "PhaseAnalyzer.h"
#include "Measures.h"
#include <ADC.h>
#include <ADC_util.h>
#include <IntervalTimer.h>
#include "Settings.h"
#include "EepromAnalyzer.h"

#define CURRENT_PIN	A2  // ADC1
#define VOLTAGE_PIN	A9  // ADC0

#define N_SAMPLE	200

#define TIMER_PERIOD	100

#define MAX_WINDOWS		15

#define TO_RAD(grd) (grd * M_PI / 180)

#define TA_TURN_RATIO		1000.0
#define BURDEN_RESISTOR		70.0



enum
{
	CURRENT_BUFF = 0,
	VOLTAGE_BUFF
};

void AcdCallBackFunc(void);

MEASURES_VAR Current;
MEASURES_VAR Voltage;
MEASURES_VAR Pf;
MEASURES_VAR PAtt;
MEASURES_VAR PRea;
MEASURES_VAR PApp;
MEASURES_VAR EnAtt;
MEASURES_VAR EnRea;
MEASURES_VAR EnApp;

ADC *adc = new ADC(); // adc object

IntervalTimer AdcTimer; // timers
bool TimerStarted;
ADC::Sync_result SyncroMeasureResult;


static double CurrentAvgAcc, VoltageAvgAcc, PfAvgAcc, PAttAvgAcc, PReaAvgAcc, PAppAvgAcc;
static uint32_t CurrentAvgCnt, VoltageAvgCnt, PfAvgCnt, PAttAvgCnt, PReaAvgCnt, PAppAvgCnt;

static bool InvalidPf;

static double EnAttAcc, EnReaAcc, EnAppAcc;
static uint32_t EnAttCnt, EnReaCnt, EnAppCnt;

volatile double CurrentRawVal[N_SAMPLE], VoltageRawVal[N_SAMPLE];
volatile uint16_t AcdBufferIndex = 0;
volatile bool BuffersFilled = false;
uint8_t NWindows;

double CurrentAcc = 0, VoltageAcc = 0;
double PAttAcc = 0.0;

double SimCurrent = 16.0, SimVoltage = 220.0, SimDelayI = 30, SimDelayV = 0.0;
double SimCurrentRawVal[N_SAMPLE], SimVoltageRawVal[N_SAMPLE];

bool simulationMode = SIM_ON;

Chrono AvgTimer_1(Chrono::SECONDS), AvgTimer_2, EnergyTimer, FirstCalcMaxMinTimer;

static void SimWaves()
{
	double SimIAcc = 0.0, SimVAcc = 0.0;
	double PAttAcc = 0.0;
	for(int i = 0; i < N_SAMPLE; i++)
	{
		SimCurrentRawVal[i] = (SIM_I_AMP(SimCurrent) * sin((TO_RAD(2 * M_PI * SIM_FRQ) * i) + TO_RAD(SimDelayI)));
		SimVoltageRawVal[i] = (SIM_V_AMP(SimVoltage) * sin((TO_RAD(2 * M_PI * SIM_FRQ) * i) + TO_RAD(SimDelayV)));
		PAttAcc += ((SimCurrentRawVal[i] * 16 / 4096) * (SimVoltageRawVal[i] * 230 / 4096));
		SimIAcc += (SimCurrentRawVal[i] * SimCurrentRawVal[i]);
		SimVAcc += (SimVoltageRawVal[i] * SimVoltageRawVal[i]);
	}
	PAtt.actual = PAttAcc / N_SAMPLE;
	SimIAcc /= N_SAMPLE;
	SimVAcc /= N_SAMPLE;
	SimIAcc = sqrt(SimIAcc);
	SimVAcc = sqrt(SimVAcc);

	Current.actual = SimIAcc * 16 / 4096;
	Voltage.actual = SimVAcc * 230 / 4096;

	if(Current.actual < 0.01)
	{
		Current.actual = 0.0;
		PAtt.actual = 0.0;
		InvalidPf = true;
	}
	else if(Voltage.actual < 200)
	{
		Voltage.actual = 0.0;
		PAtt.actual = 0.0;
		InvalidPf = true;
	}	
	else 
		InvalidPf = false;
}


void AnalogInit()
{	
	pinMode(CURRENT_PIN, INPUT);
    pinMode(VOLTAGE_PIN, INPUT);
	
	adc->adc0->setAveraging(5); // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
	
	adc->adc1->setAveraging(5); // set number of averages
    adc->adc1->setResolution(12); // set bits of resolution
	
	adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); // change the conversion speed
	adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); 
	
	adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed
	adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed

	adc->startSynchronizedContinuous(VOLTAGE_PIN, CURRENT_PIN);
	
	TimerStarted = AdcTimer.begin(AcdCallBackFunc, TIMER_PERIOD);
	
	Current.min = 16.0;
	Voltage.min = 230.0;
	PAtt.min = 3680.0;
	PRea.min = 3680.0;
	PApp.min = 3680.0;
	Pf.min = 1.000;
}





static void CalcMaxMin(MEASURES_VAR *Measure)
{
	if(Measure->max < Measure->actual)
		Measure->max = Measure->actual;
	if(Measure->min > Measure->actual || (int32_t)(Measure->min * 100) == 0)
		Measure->min = Measure->actual;
}

static void CalcMaxAvg(MEASURES_VAR *Measure)
{
	if(Measure->maxAvg < Measure->avg)
		Measure->maxAvg = Measure->avg;
}

static void CalcAvg()
{
	if(AvgTimer_1.hasPassed(MIN_TO_SEC(SettingsVals[AVG_TIME]), true))
	{
		Current.avg = CurrentAvgAcc / CurrentAvgCnt;
		Voltage.avg = VoltageAvgAcc / VoltageAvgCnt;
		Pf.avg      = PfAvgAcc / PfAvgCnt;
		PAtt.avg    = PAttAvgAcc / PAttAvgCnt;
		PRea.avg    = PReaAvgAcc / PReaAvgCnt;
		PApp.avg    = PAppAvgAcc / PAppAvgCnt;
		CurrentAvgAcc = 0.0;
		VoltageAvgAcc = 0.0;
		PfAvgAcc 	  = 0.0;
		PAttAvgAcc 	  = 0.0;
		PReaAvgAcc    = 0.0;
		PAppAvgAcc    = 0.0;
		CurrentAvgCnt = 0;
		VoltageAvgCnt = 0;
		PfAvgCnt      = 0;	 
		PAttAvgCnt    = 0; 	 
		PReaAvgCnt    = 0;   
		PAppAvgCnt    = 0;  
		AvgTimer_2.restart();
	}
	if(AvgTimer_2.hasPassed(100, true))
	{
		CurrentAvgAcc += Current.actual;
		VoltageAvgAcc += Voltage.actual;
		PfAvgAcc 	  += Pf.actual;
		PAttAvgAcc 	  += PAtt.actual;
		PReaAvgAcc    += PRea.actual;
		PAppAvgAcc    += PApp.actual;
		CurrentAvgCnt++;
		VoltageAvgCnt++;
		PfAvgCnt++; 	 
		PAttAvgCnt++; 	 
		PReaAvgCnt++;   
		PAppAvgCnt++;   
	}
}

static void CalcEnergy()
{
	EnAttAcc += PAtt.actual;
	EnReaAcc += PRea.actual;
	EnAppAcc += PApp.actual;
	EnAttCnt++;
	EnReaCnt++;
	EnAppCnt++;
	if(EnergyTimer.hasPassed(1000, true))
	{
		EnAtt.actual += (EnAttAcc / EnAttCnt) / 3600;
		EnRea.actual += (EnReaAcc / EnReaCnt) / 3600;
		EnApp.actual += (EnAppAcc / EnAppCnt) / 3600;
		EnAttAcc  = 0.0;
		EnReaAcc  = 0.0;
		EnAppAcc  = 0.0;
		EnAttCnt = 0;
		EnReaCnt = 0;
		EnAppCnt = 0;		
	}
}

void AcdCallBackFunc()
{
	if(!BuffersFilled)
	{
		SyncroMeasureResult = adc->analogSynchronizedRead(VOLTAGE_PIN, CURRENT_PIN);
		CurrentRawVal[AcdBufferIndex] = (double) SyncroMeasureResult.result_adc1;
		VoltageRawVal[AcdBufferIndex] = (double) SyncroMeasureResult.result_adc0;
		AcdBufferIndex++;
		if(AcdBufferIndex >= N_SAMPLE)
		{
			BuffersFilled = true;
			AcdBufferIndex = 0;
		}
	}
}


void GetMeasure()
{
	double CurrentBiased = 0.0, VoltageBiased = 0.0;
	double TmpCurrentCalc = 0.0, TmpVoltageCalc = 0.0;
	double sqrtCurrent = 0.0, sqrtVoltage = 0.0;
	bool InvalidPf = false;
	// int32_t DbgCurr = 0, DbgVolt = 0;
	if(!simulationMode)
	{
		if(BuffersFilled)
		{
			for(int i = 0; i < N_SAMPLE; i++)
			{
				CurrentBiased = CurrentRawVal[i] - TO_ADC_VAL(CURRENT_BIAS);
				VoltageBiased = VoltageRawVal[i] - TO_ADC_VAL(VOLTAGE_BIAS);
				// DBG("Current Raw: ," + String(CurrentBiased));
				TmpCurrentCalc = (((sqrt(CurrentBiased * CurrentBiased) * VOLT_ADCVAL_CONV) / BURDEN_RESISTOR) * TA_TURN_RATIO) - 0.2;
				TmpVoltageCalc = (sqrt(VoltageBiased * VoltageBiased) * 0.410);
				PAttAcc += (TmpCurrentCalc * TmpVoltageCalc);
				CurrentAcc += (CurrentBiased * CurrentBiased);
				VoltageAcc += (VoltageBiased * VoltageBiased);
			}
			NWindows++;	
			if(NWindows >= MAX_WINDOWS)
			{
				NWindows = 0;
				PAtt.actual = PAttAcc / (N_SAMPLE  * MAX_WINDOWS);
				
				CurrentAcc /= (N_SAMPLE * MAX_WINDOWS);
				VoltageAcc /= (N_SAMPLE * MAX_WINDOWS);
				
				sqrtCurrent = sqrt(CurrentAcc);
				sqrtVoltage = sqrt(VoltageAcc);
				
				CurrentAcc = 0.0;
				VoltageAcc = 0.0;
				Current.actual = (((sqrtCurrent * VOLT_ADCVAL_CONV) / BURDEN_RESISTOR) * TA_TURN_RATIO) - 0.2;
				Voltage.actual = sqrtVoltage * 0.410; 
				// DBG(sqrtVoltage);
				
				if(Current.actual < TARP_I || Voltage.actual < TARP_V)
				{
					if(Current.actual < TARP_I)
						Current.actual = 0.0;
					if(Voltage.actual < TARP_V)
						Voltage.actual = 0.0;
					PAtt.actual = 0.0;
					InvalidPf = true;
				}
				else
					InvalidPf = false;
				
				PAttAcc = 0.0;
				CurrentAcc = 0.0;
				VoltageAcc = 0.0;
			}
			memset(CurrentRawVal, 0x00, sizeof(CurrentRawVal) / sizeof(CurrentRawVal[0]));
			memset(VoltageRawVal, 0x00, sizeof(VoltageRawVal) / sizeof(VoltageRawVal[0]));
			BuffersFilled = false;
		}
	}
	else
	{
		SimWaves();
	}

	PApp.actual = Current.actual * Voltage.actual;
	if((int64_t)PApp.actual != 0)
		Pf.actual = PAtt.actual / PApp.actual;
	else 
		InvalidPf = true;
	if(InvalidPf || isnan(Pf.actual))
	{
		Pf.actual = PF_INVALID;
	}
	
	
	PRea.actual = PApp.actual - PAtt.actual;
	
	if(FirstCalcMaxMinTimer.hasPassed(2500))
	{
		CalcMaxMin(&Current);
		CalcMaxMin(&Voltage);
		CalcMaxMin(&Pf);
		CalcMaxMin(&PAtt);
		CalcMaxMin(&PRea);
		CalcMaxMin(&PApp);
		CalcEnergy();
	}
	CalcAvg();
	CalcMaxAvg(&Current);
	CalcMaxAvg(&Voltage);
	CalcMaxAvg(&Pf);
	CalcMaxAvg(&PAtt);
	CalcMaxAvg(&PRea);
	CalcMaxAvg(&PApp);
}

void ResetMaxMin()
{
	Current.min = 0.0;
	Voltage.min = 0.0;
	PAtt.min = 0.0;
	PRea.min = 0.0;
	PApp.min = 0.0;
	Pf.min = 0.0;
	Current.max = 0.0;
	Voltage.max = 0.0;
	PAtt.max = 0.0;
	PRea.max = 0.0;
	PApp.max = 0.0;
	Pf.max = 0.0;
}

void ResetAvg()
{
	Current.avg = 0.0;
	Voltage.avg = 0.0;
	PAtt.avg = 0.0;
	PRea.avg = 0.0;
	PApp.avg = 0.0;
	Pf.avg = 0.0;	
	Current.maxAvg = 0.0;
	Voltage.maxAvg = 0.0;
	PAtt.maxAvg = 0.0;
	PRea.maxAvg = 0.0;
	PApp.maxAvg = 0.0;
	Pf.maxAvg = 0.0;
}

void ResetEnergies()
{
	EnAtt.actual = 0.0;
	EnRea.actual = 0.0;
	EnApp.actual = 0.0;
}
