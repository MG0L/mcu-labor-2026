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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Width of the snake
#define WIDTH 1

// UART receive timeout (ms)
#define UART_RX_TIMEOUT 200

// UART transmit timeout (ms)
#define UART_TX_TIMEOUT 100

// Button definition
#define BTN_UP_GPIO_Port 	HMI_BTN_1_GPIO_Port
#define BTN_UP_Pin		 	HMI_BTN_1_Pin
#define BTN_DOWN_GPIO_Port  HMI_BTN_5_GPIO_Port
#define BTN_DOWN_Pin		HMI_BTN_5_Pin
#define BTN_LEFT_GPIO_Port  HMI_BTN_2_GPIO_Port
#define BTN_LEFT_Pin		HMI_BTN_2_Pin
#define BTN_RIGHT_GPIO_Port HMI_BTN_4_GPIO_Port
#define BTN_RIGHT_Pin		HMI_BTN_4_Pin
#define BTN_RESET_GPIO_Port HMI_BTN_3_GPIO_Port
#define BTN_RESET_Pin		HMI_BTN_3_Pin

// Clear the display
#define RESET_COMMAND		0x10

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

typedef enum {
	Up    = 1,
	Down  = 2,
	Left  = 4,
	Right = 8
} Direction;

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
  MX_I2C2_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  uint8_t x_position = OLED_WIDTH / 2;
  uint8_t y_position = OLED_HEIGHT / 2;
  uint8_t tx_data_byte = 0;
  uint8_t rx_data_byte = 0;
  int8_t x_velocity = 0;
  int8_t y_velocity = 0;

  OLED_COLOR bg_color = White;
  OLED_COLOR draw_color = Black;

  OLED_Init();
  OLED_SetDisplayOn(1);
  OLED_Fill(bg_color);
  OLED_DrawRectangle(x_position, y_position , x_position + WIDTH, y_position + WIDTH, draw_color);
  OLED_UpdateScreen();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  tx_data_byte = 0;
	  rx_data_byte = 0;

	  // Read the background colour and drawing colour toggle button
	  if (HAL_GPIO_ReadPin(BTN_RESET_GPIO_Port, BTN_RESET_Pin))
	  {
		  tx_data_byte |= RESET_COMMAND;
		  // Toggle the drawing and background colour
		  if (bg_color == White)
		  {
			  bg_color = Black;
			  draw_color = White;
		  }
		  else
		  {
			  bg_color = White;
			  draw_color = Black;
		  }
		  OLED_Fill(bg_color);
	  }

	  // Read the status of the direction buttons
	  x_velocity=0;
	  y_velocity=0;
	  if (HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin))
	  {
		  y_velocity = -1;
		  tx_data_byte |= Up;
	  }
	  if (HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin))
	  {
		  y_velocity = 1;
		  tx_data_byte |= Down;
	  }
	  if (HAL_GPIO_ReadPin(BTN_LEFT_GPIO_Port, BTN_LEFT_Pin))
	  {
		  x_velocity = -1;
		  tx_data_byte |= Left;
	  }
	  if (HAL_GPIO_ReadPin(BTN_RIGHT_GPIO_Port, BTN_RIGHT_Pin))
	  {
		  x_velocity = 1;
		  tx_data_byte |= Right;
	  }

	  // Transmit and receive data

	  HAL_UART_Transmit(&huart6, &tx_data_byte, 1, UART_TX_TIMEOUT);
	  HAL_UART_Receive(&huart6, &rx_data_byte, 1, UART_RX_TIMEOUT);

	  // Toggle the drawing and background colour based on the received data
	  if(rx_data_byte & RESET_COMMAND)
	  {
		  // Toggle the drawing and background colour
		  if (bg_color==White)
		  {
			  bg_color = Black;
			  draw_color = White;
		  }
		  else
		  {
			  bg_color = White;
			  draw_color = Black;
		  }
		  OLED_Fill(bg_color);
	  }

	  // Set the speed based on the received data
	  if(rx_data_byte & Up)
	  {
		  y_velocity += -1;
	  }
	  if(rx_data_byte & Down)
	  {
		  y_velocity += 1;
	  }
	  if(rx_data_byte & Left)
	  {
		  x_velocity += -1;
	  }
	  if(rx_data_byte & Right)
	  {
		  x_velocity += 1;
	  }

	  // Set the next position based on the velocity
	  if((x_position + x_velocity > 0) && (x_position + x_velocity < OLED_WIDTH))
	  {
		  x_position = x_position + x_velocity;
	  }
	  if((y_position + y_velocity > 0) && (y_position + y_velocity < OLED_HEIGHT))
	  {
		  y_position = y_position + y_velocity;
	  }

	  // Drawing
	  OLED_DrawRectangle(x_position, y_position , x_position + WIDTH, y_position + WIDTH, draw_color);
	  OLED_UpdateScreen();

	  // Heartbeat signal
	  HAL_GPIO_TogglePin(HMI_LED_3_GPIO_Port, HMI_LED_3_Pin);
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
