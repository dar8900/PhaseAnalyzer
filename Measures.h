#ifndef MEASURES_H
#define MEASURES_H

typedef struct
{
	double actual;
	double max;
	double min;
	double avg;
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

void AnalogInit();

#endif