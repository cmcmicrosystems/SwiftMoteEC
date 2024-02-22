/**
******************************************************************************
* @file    rf_device_it.c 
* @author  VMA RF Application Team
* @version V1.0.0
* @date    29-March-2018
* @brief   Main Interrupt Service Routines.
*          This file provides template for all exceptions handler and
*          peripherals interrupt service routine.
******************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2019 STMicroelectronics</center></h2>
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/

#include "SensorDemo_config.h"
#include "rf_device_it.h"
#include "ble_const.h"
#include "bluenrg_lp_stack.h"
#include "bluenrg_lp_evb_com.h"
#include "rf_driver_hal_vtimer.h"
#include "hal_miscutil.h"
#include "crash_handler.h"
#include "sensor.h"

#ifndef SENSOR_ACCELEROMETER_EMULATION

#include "gatt_db.h"

#endif


/** @addtogroup BlueNRG1_StdPeriph_Examples
* @{
*/

/** @addtogroup GPIO_Examples
* @{
*/ 

/** @addtogroup GPIO_IOToggle
* @{
*/ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  */
NOSTACK_FUNCTION(NORETURN_FUNCTION(void NMI_IRQHandler(void)))
{
  HAL_CrashHandler(__get_MSP(), NMI_SIGNATURE);  
  /* Go to infinite loop when NMI exception occurs */
  while (1)
  {}
}

/**
* @brief  This function handles Hard Fault exception.
*/
NOSTACK_FUNCTION(NORETURN_FUNCTION(void HardFault_IRQHandler(void)))
{
  HAL_CrashHandler(__get_MSP(), HARD_FAULT_SIGNATURE);  
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles SVCall exception.
  */
void SVC_IRQHandler(void)
{
	//printf("IRQ 1 \r\n");
}


/**
* @brief  This function handles SysTick Handler.
*/
void SysTick_IRQHandler(void)
{
	HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}


/******************************************************************************/
/*                 BLUENRG_LP Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (EXTI), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_bluenrg_lp.s).                                               */
/******************************************************************************/
/**
* @brief  This function handles UART interrupt request.
* @param  None
* @retval None
*/

/*
void USART1_IRQHandler(void)
{  
  BSP_COM_IRQHandler();
	//printf("IRQ 3 \r\n");
}
*/

/**
* @brief  This function handles DMA Handler.
*/
void DMA_IRQHandler(void)
{
  //printf("IRQ 4 \r\n");
}


/**
* @brief  This function handles GPIO interrupt request.
* @param  None
* @retval None
*/




/* User callback to be called if an interrupt is associated to a wakeup source */
void HAL_PWR_MNGR_WakeupIOCallback(uint32_t source)
{
#if POWER_SAVE
	GPIOB_IRQHandler();
#endif
}

void BLE_WKUP_IRQHandler(void)
{
  HAL_VTIMER_WakeUpCallback();
	//printf("IRQ 7 \r\n");
}

void CPU_WKUP_IRQHandler(void) 
{
  HAL_VTIMER_TimeoutCallback();
	//printf("IRQ 8 \r\n");
}

void BLE_ERROR_IRQHandler(void)
{
  volatile uint32_t debug_cmd;
  
  BLUE->DEBUGCMDREG |= 1;

  /* If the device is configured with 
     System clock = 64 MHz and BLE clock = 16 MHz
     a register read is necessary to end fine  
     the clear interrupt register operation,
     due the AHB down converter latency */ 
  debug_cmd = BLUE->DEBUGCMDREG;
	//printf("IRQ 9 \r\n");
}

void BLE_TX_RX_IRQHandler(void)
{
  uint32_t blue_status = BLUE->STATUSREG;
  uint32_t blue_interrupt = BLUE->INTERRUPT1REG;
  
  /** clear all pending interrupts */
  BLUE->INTERRUPT1REG = blue_interrupt;

  HAL_VTIMER_EndOfRadioActivityIsr();
  BLE_STACK_RadioHandler(blue_status|blue_interrupt);
  HAL_VTIMER_RadioTimerIsr();
  
  /* If the device is configured with 
     System clock = 64 MHz and BLE clock = 16 MHz
     a register read is necessary to end fine  
     the clear interrupt register operation,
     due the AHB down converter latency */ 
  blue_interrupt = BLUE->INTERRUPT1REG;
	//printf("IRQ 10 \r\n");
}

void BLE_RXTX_SEQ_IRQHandler(void)
{
  HAL_RXTX_SEQ_IRQHandler();
	//printf("IRQ 11 \r\n");
}


/******************************************************************************/
/* Peripheral Interrupt Handlers                                              */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file.                                          */
/******************************************************************************/

/**
  * @brief This function handles RTC A.
  */
//void RTC_IRQHandler(void)
//{
 // HAL_RTC_AlarmIRQHandler(&hrtc);
	//printf("IRQ 12 \r\n");
//}


/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/ 

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
