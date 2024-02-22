/******************** (C) COPYRIGHT 2019 STMicroelectronics ********************
* File Name          : SensorDemo_main.c
* Author             : AMS - RF Application team
* Version            : V1.0.0
* Date               : 05-November-2019
* Description        : BlueNRG-LP Sensor Demo main file
* Modification			 : Code is modified by CMC Microsystems in Sept. 2023
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/** @addtogroup BlueNRGLP_demonstrations_applications
 * BlueNRG-LP SensorDemo \see SensorDemo_main.c for documentation.
 *
 *@{
 */

/** @} */
/** \cond DOXYGEN_SHOULD_SKIP_THIS
 */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "OTA_btl.h" 
#include "rf_device_it.h"
#include "ble_const.h"
#include "bluenrg_lp_stack.h"
#include "rf_driver_hal_power_manager.h"
#include "rf_driver_hal_vtimer.h"
#include "bluenrg_lp_evb_com.h"
#include "sensor.h"
#include "SensorDemo_config.h"
#include "gatt_db.h"
#include "bleplat.h"
#include "nvm_db.h"
#include "OTA_btl.h"
#include "pka_manager.h"
#include "rng_manager.h"
#include "aes_manager.h"
#include "ble_controller.h"
#include "rf_driver_hal.h"
#include "AD5941.h"

//#include "ad5941main.h"
#include "SqrWaveVoltammetry.h"


#if defined(CONFIG_DEVICE_BLUENRG_LP) || defined(CONFIG_DEVICE_BLUENRG_LPS)
#include "bluenrg_lp_mote_config.h"
#endif


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifndef DEBUG
#define DEBUG   0
#endif

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define BLE_SENSOR_VERSION_STRING "1.0.0" 

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
   
NO_INIT(uint32_t dyn_alloc_a[DYNAMIC_MEMORY_SIZE>>2]);

__IO uint32_t RTCStatus = 0;

/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/

void ModulesInit(void)
{
  uint8_t ret;
  BLE_STACK_InitTypeDef BLE_STACK_InitParams = BLE_STACK_INIT_PARAMETERS;
  
  LL_AHB_EnableClock(LL_AHB_PERIPH_PKA|LL_AHB_PERIPH_RNG);

  
  BLECNTR_InitGlobal();

  HAL_VTIMER_InitType VTIMER_InitStruct = {HS_STARTUP_TIME, INITIAL_CALIBRATION, CALIBRATION_INTERVAL};
  HAL_VTIMER_Init(&VTIMER_InitStruct);
  
  BLEPLAT_Init();  
  if (PKAMGR_Init() == PKAMGR_ERROR)
  {
      while(1);
  }
  if (RNGMGR_Init() != RNGMGR_SUCCESS)
  {
      while(1);
  }
  
  /* Init the AES block */
  AESMGR_Init();
  
  /* BlueNRG-LP stack init */
  ret = BLE_STACK_Init(&BLE_STACK_InitParams);
  if (ret != BLE_STATUS_SUCCESS) {
    //printf("Error in BLE_STACK_Init() 0x%02x\r\n", ret);
    while(1);
  }
}

void ModulesTick(void)
{
  /* Timer tick */
  HAL_VTIMER_Tick();
  
  /* Bluetooth stack tick */
  BLE_STACK_Tick();
  
  /* NVM manager tick */
  NVMDB_Tick();
}


//void AD5941_PlatformInit(void);
void AD5941_PlatformInit(void);
extern uint16_t endseq;
//uint32_t temp = 0;

int main(void)
{
  
	uint8_t ret;
  WakeupSourceConfig_TypeDef wakeupIO;
  PowerSaveLevels stopLevel;
	
	
	//static uint32_t temp = 0;

  /* System initialization function */
  if (SystemInit(SYSCLK_64M, BLE_SYSCLK_32M) != SUCCESS) 
  {
    /* Error during system clock configuration take appropriate action */
    while(1);
  }
	
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init(); 
	
  /* Configure IOs for power save modes */
  BSP_IO_Init();
  
  
  ModulesInit(); 
	
	/* Application demo Led Init */
  BSP_LED_Init(BSP_LED); //Activity led
  BSP_LED_Off(BSP_LED);

  
#if CONFIG_OTA_USE_SERVICE_MANAGER
  /* Initialize the button: to be done before Sensor_DeviceInit for avoiding to 
     overwrite pressure/temperature sensor IO configuration when using BUTTON_2 (IO5) */
  
#endif /* CONFIG_OTA_USE_SERVICE_MANAGER */


#if POWER_SAVE
  wakeupIO.IO_Mask_High_polarity = 0; 
  wakeupIO.IO_Mask_Low_polarity = WAKEUP_PB5;
  wakeupIO.RTC_enable = 0;
  wakeupIO.LPU_enable = 0;
#endif

#if SENSOR_DATA
   AD5941_MCUResourceInit(0);
	 AD5941_PlatformInit();
	 
#if POWER_SAVE
	HAL_PWR_MNGR_Request(POWER_SAVE_LEVEL_STOP_NOTIMER, wakeupIO, &stopLevel);
#endif

#endif	 
  /* Sensor Device Init */
 ret = Sensor_DeviceInit();
  if (ret != BLE_STATUS_SUCCESS) {
 //   BSP_LED_On(BSP_LED3);
    while(1);
  }
	
#if DUMMY_DATA_NOTIFY	
#if POWER_SAVE
HAL_PWR_MNGR_Request(POWER_SAVE_LEVEL_STOP_NOTIMER, wakeupIO, &stopLevel);
#endif	
#endif
	
  while(1)
  {		
    ModulesTick();

		/* Application Tick */
		APP_Tick();

#if POWER_SAVE
		//HAL_PWR_MNGR_Request(POWER_SAVE_LEVEL_RUNNING, wakeupIO, &stopLevel);
		HAL_PWR_MNGR_Request(POWER_SAVE_LEVEL_STOP_NOTIMER, wakeupIO, &stopLevel);
#endif
		
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
    /* Check if the OTA firmware upgrade session has been completed */
    if (OTA_Tick() == 1)
    {
      /* Jump to the new application */
      OTA_Jump_To_New_Application();
    }
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */

  }/* while (1) */
}



/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  RTCStatus = 0xE;
  while (1)
  {
    /* Toggle LED3 with a period of one second */
   // BSP_LED_Toggle(BSP_LED3);
    HAL_Delay(1000);
  }
}


/* ***************** BlueNRG-LP Power Management Callback ******************************* */

PowerSaveLevels App_PowerSaveLevel_Check(PowerSaveLevels level)
{
  //if(BSP_COM_UARTBusy())
   // return POWER_SAVE_LEVEL_RUNNING;
  
  return POWER_SAVE_LEVEL_STOP_NOTIMER;
}

/* Event used to notify the Host that a hardware failure has occurred in the Controller. 
   See bluenrg_lp_events.h. */
void hci_hardware_error_event(uint8_t Hardware_Code)
{
  if (Hardware_Code <= 0x03)
  {
    NVIC_SystemReset();
  }
}


/**
  * This event is generated to report firmware error informations.
  * FW_Error_Type possible values: 
  * Values:
  - 0x01: L2CAP recombination failure
  - 0x02: GATT unexpected response
  - 0x03: GATT unexpected request
    After this event with error type (0x01, 0x02, 0x3) it is recommended to disconnect. 
*/
void aci_hal_fw_error_event(uint8_t FW_Error_Type,
                            uint8_t Data_Length,
                            uint8_t Data[])
{
  if (FW_Error_Type <= 0x03)
  {
    uint16_t connHandle;
    
    /* Data field is the connection handle where error has occurred */
    connHandle = LE_TO_HOST_16(Data);
    
    aci_gap_terminate(connHandle, BLE_ERROR_TERMINATED_REMOTE_USER); 
  }
}

/***************************************************************************************/

#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    
    /* Infinite loop */
    while (1)
    {}
}
#endif

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
/** \endcond
 */
