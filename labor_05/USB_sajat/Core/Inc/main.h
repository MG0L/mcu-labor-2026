/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
#define HMI_BTN_1_Pin        GPIO_PIN_1
#define HMI_BTN_1_GPIO_Port  GPIOE
#define HMI_BTN_2_Pin        GPIO_PIN_2
#define HMI_BTN_2_GPIO_Port  GPIOE
#define HMI_BTN_3_Pin        GPIO_PIN_3
#define HMI_BTN_3_GPIO_Port  GPIOE
#define HMI_BTN_4_Pin        GPIO_PIN_4
#define HMI_BTN_4_GPIO_Port  GPIOE
#define HMI_BTN_5_Pin        GPIO_PIN_5
#define HMI_BTN_5_GPIO_Port  GPIOE
#define HMI_BTN_6_Pin        GPIO_PIN_6
#define HMI_BTN_6_GPIO_Port  GPIOE

#define USB_SOF_Pin          GPIO_PIN_8
#define USB_SOF_GPIO_Port    GPIOA
#define USB_VBUS_Pin         GPIO_PIN_9
#define USB_VBUS_GPIO_Port   GPIOA
#define USB_ID_Pin           GPIO_PIN_10
#define USB_ID_GPIO_Port     GPIOA
#define USB_DM_Pin           GPIO_PIN_11
#define USB_DM_GPIO_Port     GPIOA
#define USB_DP_Pin           GPIO_PIN_12
#define USB_DP_GPIO_Port     GPIOA

#define STLK_RX_Pin          GPIO_PIN_9
#define STLK_RX_GPIO_Port    GPIOD
#define STLK_TX_Pin          GPIO_PIN_8
#define STLK_TX_GPIO_Port    GPIOD

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
