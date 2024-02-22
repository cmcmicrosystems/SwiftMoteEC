/**
  ******************************************************************************
  * @file    bluenrg_lp_evb_ll_led.c
  * @author  RF Application Team
  * @brief   his file provides set of firmware functions to manage LEDs
  *          available on BlueNRGLP-EVB Kit from STMicroelectronics.
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

/* Includes ------------------------------------------------------------------*/
#include "bluenrg_lp_mote_led.h"

/** @addtogroup BSP
  * @{
  */ 

/** @defgroup BSP_BLUENRGLP_LED BSP BlueNRG-LP LED
  * @{
  */
    
/** @addtogroup RCC_LL_Private_Macros
  * @{
  */

#if BSP_LED_INVERTED_LOGIC
#define LED_ON(port,pin) LL_GPIO_ResetOutputPin(port, pin)
#define LED_OFF(port,pin) LL_GPIO_SetOutputPin(port, pin)
#else
#define LED_ON(port,pin) LL_GPIO_SetOutputPin(port, pin)
#define LED_OFF(port,pin) LL_GPIO_ResetOutputPin(port, pin)
#endif 

/**
  * @}
  */
       

/** @defgroup BSP_BLUENRGLP_LED_Exported_Functions Exported Functions
  * @{
  */ 


/**
  * @brief  Configures LED GPIO.
  * @param  Led: LED to be configured. 
  *          This parameter can be one of the following values:
  *            @arg BSP_LED1
  *            @arg BSP_LED2
  *            @arg BSP_LED3
  * @retval None
  */
void BSP_LED_Init(Led__Mote_TypeDef Led)
{
  /* Configure the LED pin and turn off it */
  
    BSP_LED_GPIO_CLK_ENABLE();
    LED_OFF(BSP_LED_GPIO_PORT, BSP_LED_PIN);

    LL_GPIO_SetPinMode(BSP_LED_GPIO_PORT, BSP_LED_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(BSP_LED_GPIO_PORT, BSP_LED_PIN, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(BSP_LED_GPIO_PORT, BSP_LED_PIN, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(BSP_LED_GPIO_PORT, BSP_LED_PIN, BSP_LED_PULL_MODE);
 }


/**
  * @brief  DeInit LEDs.
  * @param  Led: LED to be de-init. 
  *   This parameter can be one of the following values:
  *     @arg  BSP_LED1
  *     @arg  BSP_LED2
  *     @arg  BSP_LED3
  * @note Led DeInit does not disable the GPIO clock
  * @retval None
  */
void BSP_LED_DeInit(Led__Mote_TypeDef Led)
{
  /* Turn off the LED pin and deconfigure it */

    LED_OFF(BSP_LED_GPIO_PORT, BSP_LED_PIN);
    
    LL_GPIO_SetPinMode(BSP_LED_GPIO_PORT, BSP_LED_PIN, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinSpeed(BSP_LED_GPIO_PORT, BSP_LED_PIN, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinOutputType(BSP_LED_GPIO_PORT, BSP_LED_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetPinPull(BSP_LED_GPIO_PORT, BSP_LED_PIN, LL_GPIO_PULL_NO);

  
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on. 
  *   This parameter can be one of following parameters:
  *     @arg BSP_LED1
  *     @arg BSP_LED2
  *     @arg BSP_LED3
  * @retval None
  */
void BSP_LED_On(Led__Mote_TypeDef Led)
{

    LED_OFF(BSP_LED_GPIO_PORT, BSP_LED_PIN);
  
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off. 
  *   This parameter can be one of following parameters:
  *     @arg BSP_LED1
  *     @arg BSP_LED2
  *     @arg BSP_LED3
  * @retval None
  */
void BSP_LED_Off(Led__Mote_TypeDef Led)
{

    LED_ON(BSP_LED_GPIO_PORT, BSP_LED_PIN);
  
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled. 
  *   This parameter can be one of following parameters:
  *     @arg BSP_LED1
  *     @arg BSP_LED2
  *     @arg BSP_LED3
  * @retval None
  */
void BSP_LED_Toggle(Led__Mote_TypeDef Led)
{

    LL_GPIO_TogglePin(BSP_LED_GPIO_PORT, BSP_LED_PIN);
  
}

/**
  * @}
  */ 


/**
  * @}
  */  
    
/**
  * @}
  */  

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
