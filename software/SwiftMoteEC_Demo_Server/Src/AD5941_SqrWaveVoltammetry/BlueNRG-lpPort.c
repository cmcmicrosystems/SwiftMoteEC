/**  
 * @file       BlueNRG-lpPort.c
 * @brief      CMC Microsystems SwiftMoteEC board port file.
 * @date       March 2023
 * @par Revision History: Derived from NUCLEOF411Port.c provided by Analog Devices
 *  
 * Copyright (c) 2017-2019 Analog Devices, Inc. All Rights Reserved. 
 * 
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
**/

#include "ad5941.h"
#include "stdio.h"
#include "rf_driver_hal.h"
#include "swiftmoteEC_config.h"


/**
**Definition for BlueNRG-LP on SwiftMoteEC SPI clock resources when use SPI1.
**set clock source for each GPIO pins used.
**/

#define AD5941SPI                          SPI1
#define AD5941_CLK_ENABLE()                __HAL_RCC_SPI1_CLK_ENABLE()
#define AD5941_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()   //PA9
#define AD5941_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()  //PA10
#define AD5941_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE() //PA8
#define AD5941_CS_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE() //PA11
#define AD5941_RST_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()  //PB4
#define AD5941_GP0INT_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE() //PB5

#define AD5941SPI_FORCE_RESET()               __HAL_RCC_SPI1_FORCE_RESET()
#define AD5941SPI_RELEASE_RESET()             __HAL_RCC_SPI1_RELEASE_RESET()



/* Definition for AD5941 Pins that connects to BlueNRG-LP side */
#define AD5941_SCK_PIN                     GPIO_PIN_9
#define AD5941_SCK_GPIO_PORT               GPIOA
#define AD5941_SCK_AF                      GPIO_AF1_SPI1

#define AD5941_MISO_PIN                    GPIO_PIN_10
#define AD5941_MISO_GPIO_PORT              GPIOA
#define AD5941_MISO_AF                     GPIO_AF1_SPI1

#define AD5941_MOSI_PIN                    GPIO_PIN_8
#define AD5941_MOSI_GPIO_PORT              GPIOA
#define AD5941_MOSI_AF                     GPIO_AF1_SPI1

#define AD5941_CS_PIN                      GPIO_PIN_11
#define AD5941_CS_GPIO_PORT                GPIOA
#define AD5941_CS_AF											 GPIO_AF1_SPI1


#define AD5941_RST_PIN                     GPIO_PIN_4   //AD5941_RESET
#define AD5941_RST_GPIO_PORT               GPIOB

#define AD5941_GP0INT_PIN                  GPIO_PIN_5   // BlueNRG-LP take AD5941 interrupt input on GPIOA pin_5. 
#define AD5941_GP0INT_GPIO_PORT            GPIOB
#define AD5941_GP0INT_IRQn								 GPIOB_IRQn



SPI_HandleTypeDef  SpiHandle;

volatile static uint8_t ucInterrupted = 0;       /* Flag to indicate interrupt occurred */

/**
	@brief Using SPI to transmit N bytes and return the received bytes. This function targets to 
                     provide a more efficent way to transmit/receive data.
	@param pSendBuffer :{0 - 0xFFFFFFFF}
      - Pointer to the data to be sent.
	@param pRecvBuff :{0 - 0xFFFFFFFF}
      - Pointer to the buffer used to store received data.
	@param length :{0 - 0xFFFFFFFF}
      - Data length in SendBuffer.
	@return None.
**/
void AD5941_ReadWriteNBytes(unsigned char *pSendBuffer,unsigned char *pRecvBuff,unsigned long length)
{
  HAL_SPI_TransmitReceive(&SpiHandle, pSendBuffer, pRecvBuff, length, (uint32_t)-1);
}

void AD5941_CsClr(void)
{
  HAL_GPIO_WritePin(AD5941_CS_GPIO_PORT, AD5941_CS_PIN, GPIO_PIN_RESET);
}

void AD5941_CsSet(void)
{
  HAL_GPIO_WritePin(AD5941_CS_GPIO_PORT, AD5941_CS_PIN, GPIO_PIN_SET);
}

void AD5941_RstSet(void)
{
  HAL_GPIO_WritePin(AD5941_RST_GPIO_PORT, AD5941_RST_PIN, GPIO_PIN_SET);
}

void AD5941_RstClr(void)
{
  HAL_GPIO_WritePin(AD5941_RST_GPIO_PORT, AD5941_RST_PIN, GPIO_PIN_RESET);
}

void AD5941_Delay10us(uint32_t time)
{
  time/=100;
  if(time == 0) time =1;
  HAL_Delay(time);
}


uint32_t AD5941_GetMCUIntFlag(void)
{
	return ucInterrupted;
}

uint32_t AD5941_ClrMCUIntFlag(void)
{
	ucInterrupted = 0;
	return 1;
}


uint32_t AD5941_MCUResourceInit(void *pCfg)
{
	
	GPIO_InitTypeDef  GPIO_InitStruct;
  
  /* SPI SCK GPIO pin configuration */
  AD5941_SCK_GPIO_CLK_ENABLE();
  GPIO_InitStruct.Pin       = AD5941_SCK_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = AD5941_SCK_AF;
  HAL_GPIO_Init(AD5941_SCK_GPIO_PORT, &GPIO_InitStruct);
  
  /* SPI MISO GPIO pin configuration  */
	AD5941_MISO_GPIO_CLK_ENABLE();
  GPIO_InitStruct.Pin = AD5941_MISO_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = AD5941_MISO_AF;
  HAL_GPIO_Init(AD5941_MISO_GPIO_PORT, &GPIO_InitStruct);
  
  /* SPI MOSI GPIO pin configuration  */
	AD5941_MOSI_GPIO_CLK_ENABLE();
  GPIO_InitStruct.Pin = AD5941_MOSI_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = AD5941_MOSI_AF;
  HAL_GPIO_Init(AD5941_MOSI_GPIO_PORT, &GPIO_InitStruct);
  
	/* SPI CS GPIO pin configuration  */
	AD5941_CS_GPIO_CLK_ENABLE();
  GPIO_InitStruct.Pin = AD5941_CS_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(AD5941_CS_GPIO_PORT, &GPIO_InitStruct);
  
	AD5941_CLK_ENABLE();
	
  /* SPI RST GPIO pin configuration  */
	AD5941_RST_GPIO_CLK_ENABLE();
  GPIO_InitStruct.Pin = AD5941_RST_PIN;
  HAL_GPIO_Init(AD5941_RST_GPIO_PORT, &GPIO_InitStruct);
  
  AD5941_CsSet();
  AD5941_RstSet();
  
  /* Set the SPI parameters */
	
	SpiHandle.Instance               = AD5941SPI;
	SpiHandle.Init.Mode = SPI_MODE_MASTER;
	SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
	//SpiHandle.Init.NSS               = SPI_NSS_SOFT;
	SpiHandle.Init.NSS               = SPI_NSS_HARD_OUTPUT;
	SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8; //SPI clock should be < AD5941_SystemClock, system clock set for BlueNRG-LP is 64Mhz, therefore, SPI is 8MHz. AD5941 clock is set to 16Mhz in AD5941Main.c.
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  SpiHandle.Init.CRCPolynomial     = 7;
	
	SpiHandle.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  SpiHandle.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  
  
  HAL_SPI_Init(&SpiHandle);
	__HAL_SPI_ENABLE(&SpiHandle);

	/* Step 2: Configure external interrupt line */
	
  AD5941_GP0INT_GPIO_CLK_ENABLE();
	
	LL_GPIO_SetPinMode(BSP_ad5941_INT_GPIO_PORT, BSP_ad5941_INT_PIN, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(BSP_ad5941_INT_GPIO_PORT, BSP_ad5941_INT_PIN, LL_GPIO_PULL_NO);
	
	LL_APB0_EnableClock(LL_APB0_PERIPH_SYSCFG);
	LL_EXTI_SetTrigger(BSP_ad5941_INT_EXTI_LINE_TRIGGER, BSP_ad5941_INT_EXTI_LINE);
  LL_EXTI_ClearInterrupt(BSP_ad5941_INT_EXTI_LINE);
  LL_EXTI_EnableIT(BSP_ad5941_INT_EXTI_LINE);
	

  NVIC_SetPriority(BSP_ad5941_INT_EXTI_IRQn, IRQ_LOW_PRIORITY);
  NVIC_EnableIRQ(BSP_ad5941_INT_EXTI_IRQn);

  return 0;
}

/* MCU related external line interrupt service routine */

void GPIOB_IRQHandler(void)
{
	ucInterrupted = 1;
  __HAL_GPIO_EXTI_CLEAR_IT(AD5941_GP0INT_GPIO_PORT,AD5941_GP0INT_PIN);
}

