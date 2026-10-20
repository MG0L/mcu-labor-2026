/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HMI_BTN_2_Pin GPIO_PIN_2
#define HMI_BTN_2_GPIO_Port GPIOE
#define HMI_BTN_2_EXTI_IRQn EXTI2_IRQn
#define HMI_BTN_3_Pin GPIO_PIN_3
#define HMI_BTN_3_GPIO_Port GPIOE
#define HMI_BTN_3_EXTI_IRQn EXTI3_IRQn
#define HMI_BTN_5_Pin GPIO_PIN_4
#define HMI_BTN_5_GPIO_Port GPIOE
#define HMI_BTN_5_EXTI_IRQn EXTI4_IRQn
#define HMI_BTN_1_Pin GPIO_PIN_5
#define HMI_BTN_1_GPIO_Port GPIOE
#define HMI_BTN_1_EXTI_IRQn EXTI9_5_IRQn
#define HMI_BTN_4_Pin GPIO_PIN_6
#define HMI_BTN_4_GPIO_Port GPIOE
#define HMI_BTN_4_EXTI_IRQn EXTI9_5_IRQn
#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC
#define USER_Btn_EXTI_IRQn EXTI15_10_IRQn
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define HMI_LED_4_Pin GPIO_PIN_4
#define HMI_LED_4_GPIO_Port GPIOD
#define HMI_LED_3_Pin GPIO_PIN_5
#define HMI_LED_3_GPIO_Port GPIOD
#define HMI_LED_2_Pin GPIO_PIN_6
#define HMI_LED_2_GPIO_Port GPIOD
#define HMI_LED_1_Pin GPIO_PIN_7
#define HMI_LED_1_GPIO_Port GPIOD
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
