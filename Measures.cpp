#include "PhaseAnalyzer.h"
#include "Measures.h"
#include <ADC.h>
#include <ADC_util.h>
#include "Settings.h"
#include "EepromAnalyzer.h"

#define CURRENT_PIN	A9
#define VOLTAGE_PIN	A2

#define N_SAMPLE	200

#define TO_RAD(grd) (grd * M_PI / 180)


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

static double CurrentAvgAcc, VoltageAvgAcc, PfAvgAcc, PAttAvgAcc, PReaAvgAcc, PAppAvgAcc;
static uint32_t CurrentAvgCnt, VoltageAvgCnt, PfAvgCnt, PAttAvgCnt, PReaAvgCnt, PAppAvgCnt;

static bool InvalidPf;

static double EnAttAcc, EnReaAcc, EnAppAcc;
static uint32_t EnAttCnt, EnReaCnt, EnAppCnt;

int32_t CurrentRawVal[N_SAMPLE], VoltageRawVal[N_SAMPLE];

double SimCurrentRawVal[N_SAMPLE], SimVoltageRawVal[N_SAMPLE];

bool simulationMode = SIM_ON;

Chrono AvgTimer_1(Chrono::SECONDS), AvgTimer_2, EnergyTimer;

static void SimWaves()
{
	double SimIAcc = 0.0, SimVAcc = 0.0;
	double PAttAcc = 0.0;
	for(int i = 0; i < N_SAMPLE; i++)
	{
		SimCurrentRawVal[i] = (SIM_I_AMP(SIM_CURR) * sin((TO_RAD(2 * M_PI * SIM_FRQ) * i) + TO_RAD(SIM_DELAY_I)));
		SimVoltageRawVal[i] = (SIM_V_AMP(SIM_VOLT) * sin((TO_RAD(2 * M_PI * SIM_FRQ) * i) + TO_RAD(SIM_DELAY_V)));
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
	
	adc->adc0->setAveraging(10); // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
	
	adc->adc1->setAveraging(10); // set number of averages
    adc->adc1->setResolution(12); // set bits of resolution
	
	adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); // change the conversion speed
	adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); 
	
	adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed
	adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed
	
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

void GetMeasure()
{
	uint32_t CurrentAcc = 0, VoltageAcc = 0;
	double sqrtCurrent = 0.0, sqrtVoltage = 0.0;
	double PAttAcc = 0.0;
	bool InvalidPf = false;
	if(!simulationMode)
	{
		for(int i = 0; i < N_SAMPLE; i++)
		{
			CurrentRawVal[i] = adc->adc0->analogRead(CURRENT_PIN);
			VoltageRawVal[i] = adc->adc1->analogRead(VOLTAGE_PIN);
			PAttAcc += ((CurrentRawVal[i] * 16 / 4096) * (VoltageRawVal[i] * 230 / 4096));
			CurrentAcc += CurrentRawVal[i] * CurrentRawVal[i];
			VoltageAcc += VoltageRawVal[i] * VoltageRawVal[i];
		}
		
		PAtt.actual = PAttAcc / N_SAMPLE;
		
		CurrentAcc /= N_SAMPLE;
		VoltageAcc /= N_SAMPLE;
		
		sqrtCurrent = sqrt((double)CurrentAcc);
		sqrtVoltage = sqrt((double)VoltageAcc);
		
		CurrentAcc = 0;
		VoltageAcc = 0;
		Current.actual = sqrtCurrent * 16 / 4096;
		Voltage.actual = sqrtVoltage * 230 / 4096;
		
		if(Current.actual < 0.05)
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
		
		// Current.actual = (sqrtCurrent * 3.3/adc->adc0->getMaxValue()); 
		// Voltage.actual = (sqrtVoltage * 3.3/adc->adc1->getMaxValue()); 
	}
	else
	{
		SimWaves();
	}

	PApp.actual = Current.actual * Voltage.actual;
	if(!InvalidPf)
		Pf.actual = PAtt.actual / PApp.actual;
	else
		Pf.actual = PF_INVALID;
	
	PRea.actual = PApp.actual - PAtt.actual;
	
	
	CalcMaxMin(&Current);
	CalcMaxMin(&Voltage);
	CalcMaxMin(&Pf);
	CalcMaxMin(&PAtt);
	CalcMaxMin(&PRea);
	CalcMaxMin(&PApp);
	CalcAvg();
	CalcMaxAvg(&Current);
	CalcMaxAvg(&Voltage);
	CalcMaxAvg(&Pf);
	CalcMaxAvg(&PAtt);
	CalcMaxAvg(&PRea);
	CalcMaxAvg(&PApp);
	CalcEnergy();
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
