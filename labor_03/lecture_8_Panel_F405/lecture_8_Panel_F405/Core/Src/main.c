/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "light_sensor.h"
#include "OLED.h"     // [panel] a kijelzo meghajtoja (ugyanaz, mint a labor 4-ben)
#include <stdio.h>    // [panel] sprintf
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// LED limit levels
const uint16_t LIMIT_OFF = 1000;
const uint16_t LIMIT_1 = 2000;
const uint16_t LIMIT_2 = 3000;
const uint16_t LIMIT_3 = 4000;
// [panel] a szenzor cime (a light_sensor.c-ben van megadva)
extern const uint8_t LIGHT_SENSOR_ADDRESS;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// [panel] I2C-busz helyreallitasa. Ha a feltoltes vagy a reset egy atvitel kozepen erte a buszt,
// a szenzor lefogva tarthatja az SDA-t (neki nincs reset laba, csak a tap elvetele allitja vissza).
// Ilyenkor minden I2C-hivas hibat ad. A gyogymod: legfeljebb 9 orajel az SCL-en, amig az SDA
// el nem engedodik, aztan egy STOP. Az MX_I2C1_Init() elott kell lefutnia.
static void I2C_Bus_Recovery(void)
{
  GPIO_InitTypeDef gpio = {0};
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
  gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;          // PB6 = SCL, PB7 = SDA, most sima open-drain kimenetkent
  gpio.Mode = GPIO_MODE_OUTPUT_OD;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &gpio);
  for (int i = 0; i < 9 && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_RESET; i++)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_Delay(1);
  }
  // START, majd STOP (SDA le, aztan fel, mikozben az SCL magas): minden eszkoz alapallapotba ker
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
  HAL_Delay(1);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  I2C_Bus_Recovery();   // [panel] lasd fent
/* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  // [panel] A kijelzo inditasa: ezen latszik a mert ertek (ebbol lehet kalibralni)
  OLED_Init();
  OLED_Fill(BLACK);
  OLED_SetCursor(0, 0);
  OLED_WriteString("Feny (CH0):", Font_7x10, WHITE);
  OLED_UpdateScreen();

/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // Switch on the light sensor
  Light_Sensor_Power_On(SIMPLE_MODE);

  // [panel] A szenzor a beallitasait addig megjegyzi, amig van tapja; egy rovid USB ki-be sem torli
  // (kiprobalva). Ezert az alap erositest (1x, 402 ms: TIMING = 0x02) minden indulaskor visszairjuk.
  uint8_t timing_default = 0x02;
  HAL_I2C_Mem_Write(&hi2c1, LIGHT_SENSOR_ADDRESS, TIMING, I2C_MEMADD_SIZE_8BIT, &timing_default, 1, 100);

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // Measured data reading
	  uint16_t light_sensor_value = Light_Sensor_Read_Value(SIMPLE_MODE);

	  // Switch on the LEDs based on the measured value and the limit levels
	  if (light_sensor_value < LIMIT_OFF)
	  {
		  HAL_GPIO_WritePin(HMI_LED_1_GPIO_Port, HMI_LED_1_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(HMI_LED_2_GPIO_Port, HMI_LED_2_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(HMI_LED_3_GPIO_Port, HMI_LED_3_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(HMI_LED_4_GPIO_Port, HMI_LED_4_Pin, GPIO_PIN_RESET);
	  }
	  else if (light_sensor_value < LIMIT_1)
	  {
		  HAL_GPIO_WritePin(HMI_LED_1_GPIO_Port, HMI_LED_1_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HMI_LED_2_GPIO_Port, HMI_LED_2_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(HMI_LED_3_GPIO_Port, HMI_LED_3_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(HMI_LED_4_GPIO_Port, HMI_LED_4_Pin, GPIO_PIN_RESET);
	  }
	  else if (light_sensor_value < LIMIT_2)
	  {
		  HAL_GPIO_WritePin(HMI_LED_1_GPIO_Port, HMI_LED_1_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HMI_LED_2_GPIO_Port, HMI_LED_2_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HMI_LED_3_GPIO_Port, HMI_LED_3_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(HMI_LED_4_GPIO_Port, HMI_LED_4_Pin, GPIO_PIN_RESET);
	  }
	  else if (light_sensor_value < LIMIT_3)
	  {
		  HAL_GPIO_WritePin(HMI_LED_1_GPIO_Port, HMI_LED_1_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HMI_LED_2_GPIO_Port, HMI_LED_2_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HMI_LED_3_GPIO_Port, HMI_LED_3_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HMI_LED_4_GPIO_Port, HMI_LED_4_Pin, GPIO_PIN_RESET);
	  }
	  else
	  {
		  HAL_GPIO_WritePin(HMI_LED_1_GPIO_Port, HMI_LED_1_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HMI_LED_2_GPIO_Port, HMI_LED_2_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HMI_LED_3_GPIO_Port, HMI_LED_3_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HMI_LED_4_GPIO_Port, HMI_LED_4_Pin, GPIO_PIN_SET);
	  }

	  // [panel] A mert ertek kiirasa a kijelzore
	  char text[8];
	  sprintf(text, "%5u", (unsigned) light_sensor_value);
	  OLED_SetCursor(0, 20);
	  OLED_WriteString(text, Font_16x26, WHITE);
	  OLED_UpdateScreen();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  // [panel] Ha a szenzor nem valaszol (rossz cim, rossz bekotes): kiirjuk, es mind a 4 LED egyutt villog
  OLED_SetCursor(0, 52);
  OLED_WriteString("I2C HIBA", Font_7x10, WHITE);
  OLED_UpdateScreen();
  __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOD, HMI_LED_1_Pin | HMI_LED_2_Pin | HMI_LED_3_Pin | HMI_LED_4_Pin);
    for (volatile uint32_t i = 0; i < 3000000; i++);   // kb. 0,2 s (itt a HAL_Delay nem megy: tiltva a megszakitas)
  }
/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
