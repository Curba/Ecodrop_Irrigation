/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void delay_us (uint16_t us);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define tankmotor_relay_Pin GPIO_PIN_13
#define tankmotor_relay_GPIO_Port GPIOC
#define line1_relay_Pin GPIO_PIN_14
#define line1_relay_GPIO_Port GPIOC
#define line0_relay_Pin GPIO_PIN_15
#define line0_relay_GPIO_Port GPIOC
#define moisture_adc_sens0_Pin GPIO_PIN_0
#define moisture_adc_sens0_GPIO_Port GPIOA
#define moisture_adc_sens1_Pin GPIO_PIN_1
#define moisture_adc_sens1_GPIO_Port GPIOA
#define moisture_adc_sens2_Pin GPIO_PIN_2
#define moisture_adc_sens2_GPIO_Port GPIOA
#define moisture_adc_sens3_Pin GPIO_PIN_3
#define moisture_adc_sens3_GPIO_Port GPIOA
#define rain_adc_sens_Pin GPIO_PIN_4
#define rain_adc_sens_GPIO_Port GPIOA
#define waterlevel_adc_sens_Pin GPIO_PIN_5
#define waterlevel_adc_sens_GPIO_Port GPIOA
#define waterflow_adc_sens_Pin GPIO_PIN_6
#define waterflow_adc_sens_GPIO_Port GPIOA
#define light_exti_sens_Pin GPIO_PIN_7
#define light_exti_sens_GPIO_Port GPIOA
#define button_up_Pin GPIO_PIN_0
#define button_up_GPIO_Port GPIOB
#define button_up_EXTI_IRQn EXTI0_IRQn
#define button_down_Pin GPIO_PIN_1
#define button_down_GPIO_Port GPIOB
#define button_down_EXTI_IRQn EXTI1_IRQn
#define wellmotor_relay_Pin GPIO_PIN_2
#define wellmotor_relay_GPIO_Port GPIOB
#define ac_source_Pin GPIO_PIN_10
#define ac_source_GPIO_Port GPIOB
#define ac_source_EXTI_IRQn EXTI15_10_IRQn
#define button_press_Pin GPIO_PIN_12
#define button_press_GPIO_Port GPIOA
#define button_press_EXTI_IRQn EXTI15_10_IRQn
#define dht_in_Pin GPIO_PIN_5
#define dht_in_GPIO_Port GPIOB
#define I2C1_SCL_LCD_Pin GPIO_PIN_6
#define I2C1_SCL_LCD_GPIO_Port GPIOB
#define I2C1_SDA_LCD_Pin GPIO_PIN_7
#define I2C1_SDA_LCD_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
