
/** 
  ******************************************************************************
  * @file    swiftmoteEC_config.h
  * @author  CMC Microsystems
  * @brief   This file is detrived from the ST steval_idb011V1_config.h file. It contains
	*          resources available on the SwiftMoteEC platform
  *
	******************************************************************************
	*/
/** 
  ******************************************************************************
  * @file    steval_idb011V1_config.h
  * @author  RF Application Team
  * @brief   This file contains definitions for:
  *          Resources available on a specific BlueNRG-LP QFN48 Eval Kit from STMicroelectronics
	*	
	* 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2019 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SWIFTMOTEEC_CONFIG_H
#define __SWIFTMOTEEC_CONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif

/** @addtogroup BSP
  * @{
  */

/** @addtogroup BSP_STEVAL_IDB011V1 BSP STEVAL-IDB011V1
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "bluenrg_lpx.h"
#include "rf_driver_ll_bus.h"
#include "rf_driver_ll_exti.h"
#include "rf_driver_ll_gpio.h"
#include "rf_driver_hal_power_manager.h"
   


typedef enum 
{
  BSP_LED = 0,
  //BSP_LED_GREEN  = BSP_LED,
}Led__Mote_TypeDef;



/* LED on if GPIO output is low */

#define BSP_LED_INVERTED_LOGIC                    1 
#define BSP_LED_PULL_MODE                         LL_GPIO_PULL_UP



#define BSP_LED_PIN                              LL_GPIO_PIN_14  
#define BSP_LED_GPIO_PORT                        GPIOB
#define BSP_LED_GPIO_CLK_ENABLE()                LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOB)
#define BSP_LED_GPIO_CLK_DISABLE()               LL_AHB_DisableClock(LL_AHB_PERIPH_GPIOB)


/*define for ad5941 interrupt input*/
#define BSP_ad5941_INT_PIN                       LL_GPIO_PIN_5
#define BSP_ad5941_INT_GPIO_PORT                 GPIOB
#define BSP_ad5941_INT_GPIO_CLK_ENABLE()         LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOB)
#define BSP_ad5941_INT_GPIO_CLK_DISABLE()        LL_AHB_DisableClock(LL_AHB_PERIPH_GPIOB)
#define BSP_ad5941_INT_EXTI_IRQn                 GPIOB_IRQn
#define BSP_ad5941_INT_EXTI_LINE                 LL_EXTI_LINE_PB5
#define BSP_ad5941_INT_EXTI_LINE_TRIGGER         LL_EXTI_TRIGGER_FALLING_EDGE
/*end ad5941 interrupt input defines*/



/*define for ad5940 GP1_SLEEP*/
#define BSP_ad5940_GP1SLEEP_PIN                  LL_GPIO_PIN_4
#define BSP_ad5940_GP1SLEEP_GPIO_PORT            GPIOA
#define BSP_ad5940_GP1SLEEP_GPIO_CLK_ENABLE()    LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOA)
#define BSP_ad5940_GP1SLEEP_GPIO_CLK_DISABLE()   LL_AHB_DisableClock(LL_AHB_PERIPH_GPIOA)

/*define for ad5940 GP2_SYNC*/
#define BSP_ad5940_GP2SYNC_PIN                  LL_GPIO_PIN_5
#define BSP_ad5940_GP2SYNC_GPIO_PORT            GPIOA
#define BSP_ad5940_GP2SYNC_GPIO_CLK_ENABLE()    LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOA)
#define BSP_ad5940_GP2SYNC_GPIO_CLK_DISABLE()   LL_AHB_DisableClock(LL_AHB_PERIPH_GPIOA)


#define BSP_PDM_CLK_PIN                           LL_GPIO_PIN_1
#define BSP_PDM_CLK_GPIO_PORT                     GPIOB
#define BSP_PDM_CLK_GPIO_AF()                     LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_1, LL_GPIO_AF_1)
#define BSP_PDM_CLK_GPIO_CLK_ENABLE()             LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOB)
#define BSP_PDM_CLK_GPIO_CLK_DISABLE()            LL_AHB_DisableClock(LL_AHB_PERIPH_GPIOB)

#define BSP_PDM_DATA_PIN                          LL_GPIO_PIN_2
#define BSP_PDM_DATA_GPIO_PORT                    GPIOB
#define BSP_PDM_DATA_GPIO_AF()                    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_2, LL_GPIO_AF_1)
#define BSP_PDM_DATA_GPIO_CLK_ENABLE()            LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOB)
#define BSP_PDM_DATA_GPIO_CLK_DISABLE()           LL_AHB_DisableClock(LL_AHB_PERIPH_GPIOB)




#include "bluenrg_lp_mote_io.h"
#include "bluenrg_lp_mote_led.h"


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __SWIFTMOTEEC_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

