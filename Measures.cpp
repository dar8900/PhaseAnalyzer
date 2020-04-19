#include "PhaseAnalyzer.h"
#include "Measures.h"
#include <ADC.h>
#include <ADC_util.h>
#include <AnalogBufferDMA.h>
#include "Settings.h"
#include "EepromAnalyzer.h"

#define CURRENT_PIN	A2  // ADC1
#define VOLTAGE_PIN	A9  // ADC0

#define N_SAMPLE	200

#define TO_RAD(grd) (grd * M_PI / 180)

#define TO_ADC_VAL(volt)   (volt * 4096 / 3.3)

enum
{
	CURRENT_BUFF = 0,
	VOLTAGE_BUFF
};

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



DMAMEM static volatile uint16_t __attribute__((aligned(32))) dma_adc_buff1[N_SAMPLE];
DMAMEM static volatile uint16_t __attribute__((aligned(32))) dma_adc_buff2[N_SAMPLE];
AnalogBufferDMA abdma1(dma_adc_buff1, N_SAMPLE, dma_adc_buff2, N_SAMPLE);

DMAMEM static volatile uint16_t __attribute__((aligned(32))) dma_adc2_buff1[N_SAMPLE];
DMAMEM static volatile uint16_t __attribute__((aligned(32))) dma_adc2_buff2[N_SAMPLE];
AnalogBufferDMA abdma2(dma_adc2_buff1, N_SAMPLE, dma_adc2_buff2, N_SAMPLE);

bool BothVectorReady[2];

static double CurrentAvgAcc, VoltageAvgAcc, PfAvgAcc, PAttAvgAcc, PReaAvgAcc, PAppAvgAcc;
static uint32_t CurrentAvgCnt, VoltageAvgCnt, PfAvgCnt, PAttAvgCnt, PReaAvgCnt, PAppAvgCnt;

static bool InvalidPf;

static double EnAttAcc, EnReaAcc, EnAppAcc;
static uint32_t EnAttCnt, EnReaCnt, EnAppCnt;

int32_t CurrentBaseMeasure, VoltageBaseMeasure;
double CurrentRawVal[N_SAMPLE], VoltageRawVal[N_SAMPLE];

double SimCurrent = 16.0, SimVoltage = 220.0, SimDelayI = 30, SimDelayV = 0.0;
double SimCurrentRawVal[N_SAMPLE], SimVoltageRawVal[N_SAMPLE];

bool simulationMode = SIM_ON;

Chrono AvgTimer_1(Chrono::SECONDS), AvgTimer_2, EnergyTimer;

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
	
	adc->adc0->setAveraging(10); // set number of averages
    adc->adc0->setResolution(12); // set bits of resolution
	
	adc->adc1->setAveraging(10); // set number of averages
    adc->adc1->setResolution(12); // set bits of resolution
	
	// adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); // change the conversion speed
	// adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); 
	
	// adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed
	// adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed


    abdma1.init(adc, ADC_0);
    abdma1.userData((uint32_t)TO_ADC_VAL(1.730)); // save away initial starting average

    abdma2.init(adc, ADC_1);
    abdma2.userData((uint32_t)TO_ADC_VAL(2.130)); // save away initial starting average

    adc->adc1->startContinuous(CURRENT_PIN);
    adc->adc0->startContinuous(VOLTAGE_PIN);
	
	Current.min = 16.0;
	Voltage.min = 230.0;
	PAtt.min = 3680.0;
	PRea.min = 3680.0;
	PApp.min = 3680.0;
	Pf.min = 1.000;
}


void ProcessAnalogData(AnalogBufferDMA *pabdma, uint8_t WichBuff) 
{
	uint16_t BuffCnt = 0;
	volatile uint16_t *pbuffer = pabdma->bufferLastISRFilled();
	volatile uint16_t *end_pbuffer = pbuffer + pabdma->bufferCountLastISRFilled();
	if ((uint32_t)pbuffer >= 0x20200000u)  arm_dcache_delete((void*)pbuffer, sizeof(dma_adc_buff1));
	while (pbuffer < end_pbuffer) 
	{
		if(WichBuff == CURRENT_BUFF)
		{
			CurrentRawVal[BuffCnt] = (double)(*pbuffer);
			BothVectorReady[0] = true;
			// DBG("Current Raw: " + String(CurrentRawVal[BuffCnt]));
		}
		else
		{
			VoltageRawVal[BuffCnt] = (double)(*pbuffer);
			BothVectorReady[1] = true;
			// DBG("Voltage Raw: " + String(VoltageRawVal[BuffCnt]));
		}
		BuffCnt++;
		pbuffer++;
	}
	pabdma->clearInterrupt();
;
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



void GetMeasure()
{
	double CurrentAcc = 0, VoltageAcc = 0;
	double sqrtCurrent = 0.0, sqrtVoltage = 0.0;
	double PAttAcc = 0.0;
	bool InvalidPf = false;
	int32_t DbgCurr = 0, DbgVolt = 0;
	if(!simulationMode)
	{
		if (abdma1.interrupted() && abdma2.interrupted())
		{
	        if (abdma1.interrupted()) 
          		ProcessAnalogData(&abdma1, CURRENT_BUFF);
        	if (abdma2.interrupted()) 
        		ProcessAnalogData(&abdma2, VOLTAGE_BUFF);
			if(BothVectorReady[0] && BothVectorReady[1])
			{
				for(int i = 0; i < N_SAMPLE; i++)
				{
					// CurrentRawVal[i] = (double)adc->adc1->analogRead(CURRENT_PIN) - TO_ADC_VAL(2.130);
					// VoltageRawVal[i] = (double)adc->adc0->analogRead(VOLTAGE_PIN) - TO_ADC_VAL(1.730);
					CurrentRawVal[i] -= TO_ADC_VAL(2.130);
					VoltageRawVal[i] -= TO_ADC_VAL(1.730);
					DBG("Voltage Raw: " + String(VoltageRawVal[i]));
					// delayMicroseconds(10);
					PAttAcc += ((CurrentRawVal[i] * 16 / 4096) * (sqrt(VoltageRawVal[i] * VoltageRawVal[i]) / 2.9));
					CurrentAcc += CurrentRawVal[i] * CurrentRawVal[i];
					VoltageAcc += VoltageRawVal[i] * VoltageRawVal[i];
				}
				
				PAtt.actual = PAttAcc / N_SAMPLE;
				
				CurrentAcc /= N_SAMPLE;
				VoltageAcc /= N_SAMPLE;
				
				sqrtCurrent = sqrt(CurrentAcc);
				sqrtVoltage = sqrt(VoltageAcc);
				
				CurrentAcc = 0.0;
				VoltageAcc = 0.0;
				Current.actual = sqrtCurrent * 16 / 4096;
				Voltage.actual = sqrtVoltage / 2.9; // * 230 / 2048;
				// DBG("Sqrt adc: " + String(sqrtVoltage));
				// DBG("Valore attuale: " + String(Voltage.actual));
				
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

				BothVectorReady[0] = false;
				BothVectorReady[1] = false;
				memset(CurrentRawVal, 0x00, sizeof(CurrentRawVal) / sizeof(CurrentRawVal[0]));
				memset(VoltageRawVal, 0x00, sizeof(VoltageRawVal) / sizeof(VoltageRawVal[0]));
				delay(1);
			}
		}
		
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
