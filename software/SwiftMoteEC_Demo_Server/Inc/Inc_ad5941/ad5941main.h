
#include "AD5941.h"

//#define APPBUFF_SIZE 1024
#define APPBUFF_SIZE 1200   //when step=1




//CMD received from client GUI

//float LFOSCFreq;    /* Measured LFOSC frequency */

//void AD5941RampStructInit(void *data1, void *data2, void *data3);
void AD5941RampStructInit(void *data1, void *data2, void *data3, void *data4, void *data5, void *data6, void *data7);
//void AD5941RampStructInit(void *data1, void *data2, void *data3, void *data4, void *data5);
void AD5941_Data_Generation(void);
	
void processData(void *data);