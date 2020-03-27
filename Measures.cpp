#include "PhaseAnalyzer.h"
#include "Measures.h"
#include <ADC.h>
#include <ADC_util.h>

#define CURRENT_PIN	A9
#define VOLTAGE_PIN	A2

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

double CurrentAvgAcc, VoltageAvgAcc, PfAvgAcc, PAttAvgAcc, PReaAvgAcc, PAppAvgAcc;
uint32_t CurrentAvgCnt, VoltageAvgCnt, PfAvgCnt, PAttAvgCnt, PReaAvgCnt, PAppAvgCnt;

double EnAttAcc, EnReaAcc, EnAppAcc;
uint32_t EnAttCnt, EnReaCnt, EnAppCnt;

Chrono AvgTimer_1(Chrono::SECONDS), AvgTimer_2, EnergyTimer;

void AnalogInit()
{	
	pinMode(CURRENT_PIN, INPUT);
    pinMode(VOLTAGE_PIN, INPUT);
	
	adc->adc0->setAveraging(16); // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
	
	adc->adc1->setAveraging(16); // set number of averages
    adc->adc1->setResolution(12); // set bits of resolution
	
	adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED); // change the conversion speed
	adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED); 
	
	adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED); // change the sampling speed
	adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED); // change the sampling speed
	
	Current.min = 16.0;
	Voltage.min = 230.0;
	PAtt.min = 3.680;
	PRea.min = 3.680;
	PApp.min = 3.680;
	Pf.min = 1.000;
	
}


static void CalcMaxMin(MEASURES_VAR *Measure)
{
	if(Measure->max < Measure->actual)
		Measure->max = Measure->actual;
	if(Measure->min > Measure->actual)
		Measure->min = Measure->actual;
}

static void CalcMaxAvg(MEASURES_VAR *Measure)
{
	if(Measure->maxAvg < Measure->avg)
		Measure->maxAvg = Measure->avg;
}

static void CalcAvg()
{
	if(AvgTimer_1.hasPassed(MIN_TO_SEC(1), true))
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
	int32_t adc0Val = 0, adc1Val = 0;
	adc0Val = adc->adc0->analogRead(CURRENT_PIN);
	adc1Val = adc->adc1->analogRead(VOLTAGE_PIN);
	
	Current.actual = (double)(adc0Val*3.3/adc->adc0->getMaxValue()); 
	Voltage.actual = 220;//(double)(adc1Val*3.3/adc->adc0->getMaxValue()); 
	
	PAtt.actual = Current.actual * Voltage.actual;
	Pf.actual = (double)(rand()%1000) / 1000.0;
	PRea.actual = Current.actual * Voltage.actual * Pf.actual;
	PApp.actual = PAtt.actual + PRea.actual;
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