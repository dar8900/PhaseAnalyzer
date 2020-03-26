#include "PhaseAnalyzer.h"
#include "Measures.h"
#include <ADC.h>
#include <ADC_util.h>

#define CURRENT_PIN	A9
#define VOLTAGE_PIN	A3

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

void AnalogInit()
{
	// int32_t Value1 = 0, Value2 = 0;
	// float Value1fl = 0.0, Value2fl = 0.0;
	
	pinMode(CURRENT_PIN, INPUT);
    pinMode(VOLTAGE_PIN, INPUT);
	adc->adc0->setAveraging(16); // set number of averages
    adc->adc0->setResolution(16); // set bits of resolution
	// it can be any of the ADC_CONVERSION_SPEED enum: VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED_16BITS, HIGH_SPEED or VERY_HIGH_SPEED
    // see the documentation for more information
    // additionally the conversion speed can also be ADACK_2_4, ADACK_4_0, ADACK_5_2 and ADACK_6_2,
    // where the numbers are the frequency of the ADC clock in MHz and are independent on the bus speed.
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED_16BITS); // change the conversion speed
    // it can be any of the ADC_MED_SPEED enum: VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED or VERY_HIGH_SPEED
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed
    adc->adc0->startContinuous(CURRENT_PIN);
	
	adc->adc1->setAveraging(16); // set number of averages
    adc->adc1->setResolution(16); // set bits of resolution
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED_16BITS); 
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
    adc->adc1->startContinuous(VOLTAGE_PIN);
	
	// Value1 = (int32_t)adc->adc0->analogReadContinuous();
	// Value2 = (int32_t)adc->adc1->analogReadContinuous();
	// Value1fl = ((float)Value1) * 3.3/adc->adc0->getMaxValue();
	// Value2fl = ((float)Value1) * 3.3/adc->adc1->getMaxValue();
	// DBG("adc0 val = " + String(Value1) + " Voltage = " + String(Value1fl, 1));
	// DBG("adc1 val = " + String(Value2) + " Voltage = " + String(Value2fl));
	// delay(5000);
}




void GetMeasure()
{
	
	
}