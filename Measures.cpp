#include "PhaseAnalyzer.h"
#include "Measures.h"
#include <ADC.h>
// #include <ADC_util.h>
#include <IntervalTimer.h>
#include "Settings.h"
#include "EepromAnalyzer.h"
#include "Logs.h"
#include "Rele.h"
#include "Time.h"

#define CURRENT_PIN	A2  // ADC1
#define VOLTAGE_PIN	A9  // ADC0




#define TA_TURN_RATIO		1000.0
#define BURDEN_RESISTOR		70.0




enum
{
	CURRENT_BUFF = 0,
	VOLTAGE_BUFF
};

void AcdCallBackFunc(void);

extern int32_t CurrentGraphicCopy[]; 
extern int32_t VoltageGraphicCopy[];

MEASURES_VAR Current;
MEASURES_VAR Voltage;
MEASURES_VAR Pf;
MEASURES_VAR PAtt;
MEASURES_VAR PRea;
MEASURES_VAR PApp;
MEASURES_VAR EnAtt;
MEASURES_VAR EnRea;
MEASURES_VAR EnApp;

MEASURES_VAR EnAppF1;
MEASURES_VAR EnAppF2;
MEASURES_VAR EnAppF3;

double EnAppTotMoney;

ADC *adc = new ADC(); // adc object

IntervalTimer AdcTimer; // timers
bool TimerStarted;
ADC::Sync_result SyncroMeasureResult;


static double CurrentAvgAcc, VoltageAvgAcc, PfAvgAcc, PAttAvgAcc, PReaAvgAcc, PAppAvgAcc;
static uint32_t CurrentAvgCnt, VoltageAvgCnt, PfAvgCnt, PAttAvgCnt, PReaAvgCnt, PAppAvgCnt;
static double CurrentBias = 0.0;

static bool InvalidPf;

static double EnAttAcc, EnReaAcc, EnAppAcc;
static uint32_t EnAttCnt, EnReaCnt, EnAppCnt;

double DailyEnApp;

volatile double CurrentRawVal[N_SAMPLE], VoltageRawVal[N_SAMPLE];
volatile uint16_t AcdBufferIndex = 0;
volatile bool BuffersFilled = false;
volatile static bool StartCollecting = false;

uint8_t NWindows;

double CurrentAcc = 0, VoltageAcc = 0;
double PAttAcc = 0.0;

double SimCurrent = 16.0, SimVoltage = 220.0, SimDelayI = 30, SimDelayV = 0.0;
double SimCurrentRawVal[N_SAMPLE], SimVoltageRawVal[N_SAMPLE];

double CurrentCorrection;

bool EnableCalcEnergyAvg = false;
bool simulationMode = SIM_ON;
bool CalcCurrent = false;

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
	
	TimerStarted = AdcTimer.begin(AcdCallBackFunc, ADC_TIMER_PERIOD);
	
	Current.min = 16.0;
	Voltage.min = 230.0;
	PAtt.min = 3680.0;
	PRea.min = 3680.0;
	PApp.min = 3680.0;
	Pf.min = 1.000;
	Pf.max = PF_INVALID;
}





static void CalcMaxMin(MEASURES_VAR *Measure)
{
	if(Measure->max < Measure->actual)
		Measure->max = Measure->actual;
	if(Measure->min > Measure->actual || (int32_t)(Measure->min * 100) == 0)
		Measure->min = Measure->actual;
}

static void CalcMaxMinPf()
{
	if((int32_t)Pf.actual != (int32_t)PF_INVALID)
	{
		if((int32_t)Pf.max == (int32_t)PF_INVALID)
			Pf.max = 0.0;
		if(Pf.max < Pf.actual) 
			Pf.max = Pf.actual;
	}
	if(Pf.min > Pf.actual || (int32_t)(Pf.min * 100) == 0)
		Pf.min = Pf.actual;	
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
		DailyEnApp = EnApp.actual; 
		switch(BandHour)
		{
			case F1:
				EnAppF1.actual += (EnAppAcc / EnAppCnt) / 3600;
				break;
			case F2:
				EnAppF2.actual += (EnAppAcc / EnAppCnt) / 3600;
				break;
			case F3:
				EnAppF3.actual += (EnAppAcc / EnAppCnt) / 3600;
				break;
			default:
				break;
		}
		EnAppTotMoney = (EnApp.actual * ((double)SettingsVals[CENT_PER_KVARH] * 0.01) / 1000.0);
		
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
		// if((VoltageRawVal[AcdBufferIndex] > 1800 && VoltageRawVal[AcdBufferIndex] < 1850) && !StartCollecting)
			StartCollecting = true;
		if(StartCollecting)
		{
			AcdBufferIndex++;
			if(AcdBufferIndex >= N_SAMPLE)
			{
				BuffersFilled = true;
				StartCollecting = false;
				AcdBufferIndex = 0;
			}
		}
	}
}

double CurrentBiasTmp = 0.0;
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
			if(CalcCurrent)
			{
				for(int i = 0; i < N_SAMPLE; i++)
				{
					CurrentBiased = CurrentRawVal[i] - TO_ADC_VAL(CurrentBias);
					VoltageBiased = VoltageRawVal[i] - TO_ADC_VAL(VOLTAGE_BIAS);
					// DBG("Current Raw: ," + String(CurrentBiased));
					// TmpCurrentCalc = (((sqrt(CurrentBiased * CurrentBiased) * VOLT_ADCVAL_CONV) / BURDEN_RESISTOR) * TA_TURN_RATIO) - CurrentCorrection;
					TmpCurrentCalc = (((sqrt(CurrentBiased * CurrentBiased) * VOLT_ADCVAL_CONV) / BURDEN_RESISTOR) * TA_TURN_RATIO);
					TmpVoltageCalc = (sqrt(VoltageBiased * VoltageBiased) * VOLTAGE_CORRECTION);
					PAttAcc += (TmpCurrentCalc * TmpVoltageCalc);
					CurrentAcc += (CurrentBiased * CurrentBiased);
					VoltageAcc += (VoltageBiased * VoltageBiased);
				}
			}
			else
			{
				for(int i = 0; i < N_SAMPLE; i++)
				{
					CurrentBiasTmp += (CurrentRawVal[i] * VOLT_ADCVAL_CONV);
					CurrentAcc = 0.0;
					VoltageBiased = VoltageRawVal[i] - TO_ADC_VAL(VOLTAGE_BIAS);
					VoltageAcc += (VoltageBiased * VoltageBiased);
				}				
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
				// Current.actual = (((sqrtCurrent * VOLT_ADCVAL_CONV) / BURDEN_RESISTOR) * TA_TURN_RATIO) - CurrentCorrection;
				Current.actual = (((sqrtCurrent * VOLT_ADCVAL_CONV) / BURDEN_RESISTOR) * TA_TURN_RATIO);
				Current.actual = (roundf(Current.actual * 100.0)) / 100.0;
				Voltage.actual = sqrtVoltage * VOLTAGE_CORRECTION; 
				Voltage.actual = roundf(Voltage.actual);
				// DBG(sqrtVoltage);
				if(!Switch.isActive)
				{
					CalcCurrent = false;
					CurrentBiasTmp /= (N_SAMPLE * MAX_WINDOWS);
					CurrentBias = CurrentBiasTmp;
					CurrentBiasTmp = 0.0;
					DBG("CurrentBias: " + String(CurrentBias));
					CurrentCorrection = CURRENT_CORRECTION;
					Current.actual = 0.0;
					if(Voltage.actual < TARP_V)
						Voltage.actual = 0.0;
					PAtt.actual = 0.0;
					InvalidPf = true;
				}
				else
				{
					CalcCurrent = true;
					// CurrentCorrection = CURRENT_CORRECTION_SW_ACTIVE;
					if(Current.actual < TARP_I)
					{
						Current.actual = 0.0;
						PAtt.actual = 0.0;
					}
					InvalidPf = false;
				}
				// if(Current.actual < TARP_I || Voltage.actual < TARP_V)
				// {
				// 	if(Current.actual < TARP_I)
				// 		Current.actual = 0.0;
				// 	if(Voltage.actual < TARP_V)
				// 		Voltage.actual = 0.0;
				// 	PAtt.actual = 0.0;
				// 	InvalidPf = true;
				// }
				// else
				// 	InvalidPf = false;
				
				PAttAcc = 0.0;
				CurrentAcc = 0.0;
				VoltageAcc = 0.0;
			}
			for(int i = 0; i < N_SAMPLE; i++)
			{
				CurrentGraphicCopy[i] = (int32_t)CurrentRawVal[i];
				VoltageGraphicCopy[i] = (int32_t)VoltageRawVal[i];
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
	if((int64_t)PApp.actual != 0 && PApp.actual >= PAtt.actual)
	{
		Pf.actual = PAtt.actual / PApp.actual;
	}
	else 
		InvalidPf = true;
	if(InvalidPf || isnan(Pf.actual))
	{
		Pf.actual = PF_INVALID;
	}
	
	if(PApp.actual >= PAtt.actual)
		PRea.actual = PApp.actual - PAtt.actual;
	
	if(FirstCalcMaxMinTimer.hasPassed(2500))
	{
		CalcMaxMin(&Current);
		CalcMaxMin(&Voltage);
		// CalcMaxMin(&Pf);
		CalcMaxMinPf();
		CalcMaxMin(&PAtt);
		CalcMaxMin(&PRea);
		CalcMaxMin(&PApp);
		if(EnableCalcEnergyAvg)
		{
			CalcEnergy();
			CalcAvg();
		}
	}
	CalcMaxAvg(&Current);
	CalcMaxAvg(&Voltage);
	CalcMaxAvg(&Pf);
	CalcMaxAvg(&PAtt);
	CalcMaxAvg(&PRea);
	CalcMaxAvg(&PApp);
}

void ResetMaxMin()
{
	Current.min = 16.0;
	Voltage.min = 230.0;
	PAtt.min = 3680.0;
	PRea.min = 3680.0;
	PApp.min = 3680.0;
	Pf.min = 1.000;
	Pf.max = PF_INVALID;
	Current.max = 0.0;
	Voltage.max = 0.0;
	PAtt.max = 0.0;
	PRea.max = 0.0;
	PApp.max = 0.0;
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
	EnAppF1.actual = 0.0;
	EnAppF2.actual = 0.0;
	EnAppF3.actual = 0.0;
	EnAppTotMoney = 0.0;
}
