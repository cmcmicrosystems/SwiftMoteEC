/*!
 *****************************************************************************
 @file:    AD5941Main.c
 @author:  $Author: nxu2 $
 @brief:   Used to control specific application and process data.
 @version: $Revision: 766 $
 @date:    $Date: 2017-08-21 14:09:35 +0100 (Mon, 21 Aug 2017) $
 @Modification: Code is modified by CMC Microsystems in Sept. 2023
 -----------------------------------------------------------------------------

Copyright (c) 2017-2019 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.

*****************************************************************************/
#include "SqrWaveVoltammetry.h"
#include "ad5941main.h"

/**
   User could configure following parameters
**/

//#define APPBUFF_SIZE 1024
uint32_t AppBuff[APPBUFF_SIZE];
float LFOSCFreq;    /* Measured LFOSC frequency */

/**
 * @brief The general configuration to AD5941 like FIFO/Sequencer/Clock. 
 * @note This function will firstly reset AD5941 using reset pin.
 * @return return 0.
*/
static int32_t AD5941PlatformCfg(void)
{
  CLKCfg_Type clk_cfg;
  SEQCfg_Type seq_cfg;  
  FIFOCfg_Type fifo_cfg;
  AGPIOCfg_Type gpio_cfg;
  LFOSCMeasure_Type LfoscMeasure;

  /* Use hardware reset */
  AD5941_HWReset();
  AD5941_Initialize();    /* Call this right after AFE reset */
	
  /* Platform configuration */
  /* Step1. Configure clock */
  clk_cfg.HFOSCEn = bTRUE;
  clk_cfg.HFXTALEn = bFALSE;
  clk_cfg.LFOSCEn = bTRUE;
  clk_cfg.HfOSC32MHzMode = bFALSE;
  clk_cfg.SysClkSrc = SYSCLKSRC_HFOSC;
  clk_cfg.SysClkDiv = SYSCLKDIV_1;
  clk_cfg.ADCCLkSrc = ADCCLKSRC_HFOSC;
  clk_cfg.ADCClkDiv = ADCCLKDIV_1;
  AD5941_CLKCfg(&clk_cfg);
  /* Step2. Configure FIFO and Sequencer*/
  fifo_cfg.FIFOEn = bTRUE;           /* We will enable FIFO after all parameters configured */
  fifo_cfg.FIFOMode = FIFOMODE_FIFO;
  fifo_cfg.FIFOSize = FIFOSIZE_4KB;   /* 2kB for FIFO, The reset 4kB for sequencer */
  fifo_cfg.FIFOSrc = FIFOSRC_SINC3;   /* */
  fifo_cfg.FIFOThresh = 4;            /*  Don't care, set it by application paramter */
  AD5941_FIFOCfg(&fifo_cfg);
  seq_cfg.SeqMemSize = SEQMEMSIZE_2KB;  /* 4kB SRAM is used for sequencer, others for data FIFO */
  seq_cfg.SeqBreakEn = bFALSE;
  seq_cfg.SeqIgnoreEn = bTRUE;
  seq_cfg.SeqCntCRCClr = bTRUE;
  seq_cfg.SeqEnable = bFALSE;
  seq_cfg.SeqWrTimer = 0;
  AD5941_SEQCfg(&seq_cfg);
  /* Step3. Interrupt controller */
  AD5941_INTCCfg(AFEINTC_1, AFEINTSRC_ALLINT, bTRUE);   /* Enable all interrupt in INTC1, so we can check INTC flags */
  AD5941_INTCClrFlag(AFEINTSRC_ALLINT);
  AD5941_INTCCfg(AFEINTC_0, AFEINTSRC_DATAFIFOTHRESH|AFEINTSRC_ENDSEQ|AFEINTSRC_CUSTOMINT0, bTRUE); 
  AD5941_INTCClrFlag(AFEINTSRC_ALLINT);
  /* Step4: Configure GPIOs */
  gpio_cfg.FuncSet = GP0_INT|GP1_SLEEP|GP2_SYNC;  /* GPIO1 indicates AFE is in sleep state. GPIO2 indicates ADC is sampling. */
  gpio_cfg.InputEnSet = 0;
  gpio_cfg.OutputEnSet = AGPIO_Pin0|AGPIO_Pin1|AGPIO_Pin2;
  gpio_cfg.OutVal = 0;
  gpio_cfg.PullEnSet = 0;
  AD5941_AGPIOCfg(&gpio_cfg);
  /* Measure LFOSC frequency */
  /**@note Calibrate LFOSC using system clock. The system clock accuracy decides measurement accuracy. Use XTAL to get better result. */
  LfoscMeasure.CalDuration = 1000.0;  /* 1000ms used for calibration. */
  LfoscMeasure.CalSeqAddr = 0;        /* Put sequence commands from start address of SRAM */
  LfoscMeasure.SystemClkFreq = 16000000.0f; /* 16MHz in this firmware. */
  AD5941_LFOSCMeasure(&LfoscMeasure, &LFOSCFreq);
//  printf("LFOSC Freq:%f\n", LFOSCFreq);
 // AD5941_SleepKeyCtrlS(SLPKEY_UNLOCK);         /*  */
  return 0;
}

/**
 * @brief The interface for user to change application paramters.
* @parameters: data1 - E1, data2 - E2, data3 - Ep, data4 - Gain, data5 - Rtia, data6 - Freq, data7 - Amplitude
 * @return return 0.
*/
uint8_t ramp_init_done = 0; 
void AD5941RampStructInit(void *data1, void *data2, void *data3, void *data4, void *data5, void *data6, void *data7)
//void AD5941RampStructInit(void *data1, void *data2, void *data3, void *data4, void *data5)
{
  
	uint16_t startvolt;
	uint16_t finalvolt;
	uint16_t voltincrement;
	uint16_t gain;
	uint16_t rtia;
	uint16_t freq;
	uint16_t amplitude;
	
	startvolt = *((uint16_t *) data1);
	finalvolt = *((uint16_t *) data2);
	voltincrement = *((uint16_t *) data3);
	gain = *((uint16_t *) data4);
	rtia = *((uint16_t *) data5);
	freq = *((uint16_t *) data6);
	amplitude = *((uint16_t *) data7);

	//freq = *((uint16_t *) data4);
	//amplitude = *((uint16_t *) data5);
	
	AppSWVCfg_Type *pRampCfg;
  
  AppSWVGetCfg(&pRampCfg);
  /* Step1: configure general parmaters */
  pRampCfg->SeqStartAddr = 0x10;                /* leave 16 commands for LFOSC calibration.  */
  pRampCfg->MaxSeqLen = 512-0x10;              /* 4kB/4 = 1024  */
	//pRampCfg->MaxSeqLen = 1024-0x10; 
  pRampCfg->RcalVal = 10000.0;                  /* 10kOhm RCAL */
  //pRampCfg->ADCRefVolt = 1.820f;               /* The real ADC reference voltage. Measure it from capacitor C12 with DMM. */
	pRampCfg->ADCRefVolt = 1820.0f;
  pRampCfg->FifoThresh = 1023;                   /* Maximum value is 2kB/4-1 = 512-1. Set it to higher value to save power. */

  pRampCfg->SysClkFreq = 16000000.0f;           /* System clock is 16MHz by default */
  pRampCfg->LFOSCClkFreq = LFOSCFreq;           /* LFOSC frequency */
	//pRampCfg->AdcPgaGain = ADCPGA_1P5;
	pRampCfg->AdcPgaGain = (uint32_t)gain;
	pRampCfg->ADCSinc3Osr = ADCSINC3OSR_4;
  
	/* Step 2:Configure square wave signal parameters */
	
	pRampCfg->RampStartVolt = (float)startvolt;           /* -1.4V */
  pRampCfg->RampPeakVolt = (float)finalvolt;
  pRampCfg->VzeroStart = 1300.0f;               /* 1.3V */
  pRampCfg->VzeroPeak = 1300.0f;                /* 1.3V */
  pRampCfg->Frequency = (float)freq;                     /* Frequency of square wave in Hz */
  pRampCfg->SqrWvAmplitude = (float)amplitude;
	pRampCfg->SqrWvRampIncrement = (float)voltincrement;
  pRampCfg->SampleDelay = 10.0f;             /* Time between update DAC and ADC sample. Unit is ms and must be < (1/Frequency)/2 - 0.2*/
	//pRampCfg->SampleDelay = 5.0f; 
 // pRampCfg->LPTIARtiaSel = LPTIARTIA_10K;      /* Maximum current decides RTIA value */
	//pRampCfg->LPTIARtiaSel = LPTIARTIA_200R;
	pRampCfg->LPTIARtiaSel = (uint32_t)rtia;
	//pRampCfg->bRampOneDir = bFALSE;//bTRUE;			/* Only measure ramp in one direction */
	pRampCfg->bRampOneDir = bTRUE; 
	
	ramp_init_done = 1;
}


void AD5941_PlatformInit(void)
{
  AD5941PlatformCfg();

}



