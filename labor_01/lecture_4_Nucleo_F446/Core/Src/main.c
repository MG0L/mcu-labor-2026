/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../External/Inc/example.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	exampleFunction();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  uint32_t GPIOD_base_addr = 0x40020C00;
  uint32_t RCC_base_addr = 0x40023800;
  uint32_t* GPIO_MODDER_addr= (uint32_t *)(GPIOD_base_addr+0x00);
  uint32_t* GPIO_OTYPER_addr = (uint32_t *)(GPIOD_base_addr+0x04);
  uint32_t* GPIO_OSPEEDR_addr = (uint32_t *)(GPIOD_base_addr+0x08);
  uint32_t* GPIO_PUPDR_addr = (uint32_t *)(GPIOD_base_addr+0x0C);
  uint32_t* GPIO_IDR_addr = (uint32_t *)(GPIOD_base_addr+0x10);
  uint32_t* GPIO_ODR_addr = (uint32_t *)(GPIOD_base_addr+0x14);
  uint32_t* GPIO_BSRR_addr = (uint32_t *)(GPIOD_base_addr+0x18);
  uint32_t* GPIO_LCKR_addr = (uint32_t *)(GPIOD_base_addr+0x1C);
  uint32_t* GPIO_AFRL_addr = (uint32_t *)(GPIOD_base_addr+0x20);
  uint32_t* GPIO_AFRH_addr = (uint32_t *)(GPIOD_base_addr+0x24);
  uint32_t* RCC_AHB1 = (uint32_t *)(RCC_base_addr+0x30);

  *RCC_AHB1 = 0x00100008;			// Set the periphery register
  *GPIO_MODDER_addr = 0x00004000; 	// Setting the PD7 GPIO to output, everything else to input
  *GPIO_OTYPER_addr = 0x00000000; 	// Reset, all types set to push-pull
  *GPIO_OSPEEDR_addr = 0x00000000; 	// Low speed
  *GPIO_PUPDR_addr = 0x00000000; 	// No pull-up or pull-down resistor
  *GPIO_IDR_addr = 0x00000000; 		// Input register set to 0
  *GPIO_ODR_addr = 0x00000000; 		// Output register set to 0
  *GPIO_BSRR_addr = 0x00000000; 	// No S-R flip flop to the output
  *GPIO_LCKR_addr = 0x00000000; 	// No locking
  *GPIO_AFRL_addr = 0x00000000;		// Default function
  *GPIO_AFRH_addr = 0x00000000; 	// Default function
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  *GPIO_ODR_addr = *GPIO_ODR_addr^0x00000080; 	// Toggle the PD7 pins state
	  for(uint32_t i = 0; i < 3330000; i++); 		// Wait for 500 milliseconds (clk = 25 MHz)

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
