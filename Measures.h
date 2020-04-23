#ifndef MEASURES_H
#define MEASURES_H

#define SIM_ON		true
#define SIM_OFF		false

#define SIM_CURR	        16
#define SIM_VOLT	        220
#define SIM_I_AMP(curr)		((curr * 4096 / 16) * 1.4142)
#define SIM_V_AMP(volt)		((volt * 4096 / 230) * 1.4142)
#define SIM_FRQ		         0.28
#define SIM_DELAY_I          20
#define SIM_DELAY_V          0

#define VOLT_ADCVAL_CONV	(3.3 / 4096.0)


#define PF_INVALID 2.0
#define TO_ADC_VAL(volt)   (volt * 4096 / 3.3)

#define CURRENT_BIAS 	1.915
#define VOLTAGE_BIAS 	1.704

typedef struct
{
	double actual;
	double max;
	double min;
	double avg;
	double maxAvg;
}MEASURES_VAR;

extern MEASURES_VAR Current;
extern MEASURES_VAR Voltage;
extern MEASURES_VAR Pf;
extern MEASURES_VAR PAtt;
extern MEASURES_VAR PRea;
extern MEASURES_VAR PApp;
extern MEASURES_VAR EnAtt;
extern MEASURES_VAR EnRea;
extern MEASURES_VAR EnApp;

extern volatile double CurrentRawVal[];
extern volatile double VoltageRawVal[];

extern double SimCurrentRawVal[];
extern double SimVoltageRawVal[];

extern bool simulationMode;
extern double SimCurrent;
extern double SimVoltage;
extern double SimDelayI;
extern double SimDelayV;

void AnalogInit();
void GetMeasure();
void ResetMaxMin();
void ResetAvg();
void ResetEnergies();

#endif