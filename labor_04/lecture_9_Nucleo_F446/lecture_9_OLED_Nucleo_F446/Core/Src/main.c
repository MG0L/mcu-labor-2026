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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// Enumeration for state machine
typedef enum
{
    OLED_STATE_CCE = 0x00,          // Crystal Clear Electronics
    OLED_STATE_FONTS = 0x01,        // Character set review
    OLED_STATE_LINE = 0x02,         // Random Line generator
    OLED_STATE_RECTANGLE = 0x03,    // Rectangle screen saver
    OLED_STATE_CIRCLE = 0x04,       // Pipe - Circle screen saver
    OLED_STATE_ARC = 0x05,          // Arc
    OLED_STATE_POLYLINE = 0x06      // Polyline
} OLED_Test_state;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
OLED_Test_state test_state = OLED_STATE_CCE;

GPIO_PinState previous_button_state[3] = {0u, 0u, 0u};
GPIO_PinState current_button_state[3] = {0u, 0u, 0u};

uint8_t start_x = 0u;
uint8_t start_r = 1u;
uint16_t start_angle = 0u;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Random Line generator
void OLED_TestLine()
{
    int random_x1 = rand() % 128; // random number between 0 and 127
    int random_x2 = rand() % 128; // random number between 0 and 127

    OLED_DrawLine((uint8_t)random_x1, 0, (uint8_t)random_x2, 63, WHITE);

    return;
}

// Rectangle screen saver
void OLED_TestRectangle()
{
    uint32_t delta;

    for (delta = 0; delta < 5; delta ++)
    {
        OLED_DrawRectangle(1 + (5*delta),1 + (5*delta) ,OLED_WIDTH-1 - (5*delta),OLED_HEIGHT-1 - (5*delta), WHITE);
    }

    return;
}

// Pipe - Circle screen saver
void OLED_TestCircle(uint8_t* x, uint8_t* r)
{
    OLED_DrawCircle(*x, 31, *r, WHITE);
    *r += 1;
    *x += 2;

    if (100 < *x)
    {
        *x = 0;
        *r = 1;
        OLED_Fill(BLACK);
    }

    return;
}

// Arc
void OLED_TestArc(uint16_t* angle)
{
    OLED_Fill(BLACK);
    OLED_DrawArc(63, 31, 5, *angle, 270+(*angle), WHITE);
    OLED_DrawArc(63, 31, 10, *angle, 270+(*angle), WHITE);
    OLED_DrawArc(63, 31, 15, *angle, 270+(*angle), WHITE);
    OLED_DrawArc(63, 31, 20, *angle, 270+(*angle), WHITE);

    *angle += 10;

    return;
}

// Polyline
void OLED_TestPolyline()
{
  OLED_Vertex loc_vertex[] =
  {
      {63,11},
      {43,20},
      {63,51},
      {83,20},
      {63,11}
  };

  OLED_DrawPolyline(loc_vertex,sizeof(loc_vertex)/sizeof(loc_vertex[0]),WHITE);

  return;
}

// Character set review
void OLED_TestFonts()
{
    OLED_SetCursor(2, 0);
    OLED_WriteString("16x26", Font_16x26, WHITE);
    OLED_SetCursor(2, 26);
    OLED_WriteString("11x18", Font_11x18, WHITE);
    OLED_SetCursor(2, 26+18);
    OLED_WriteString("7x10", Font_7x10, WHITE);
    OLED_SetCursor(2, 26+18+10);
    OLED_WriteString("6x8", Font_6x8, WHITE);

    return;
}

// Crystal Clear Electronics
void OLED_TestCCE()
{
    OLED_SetCursor(25, 5);
    OLED_WriteString("Crystal", Font_11x18, WHITE);
    OLED_SetCursor(36, 5+18);
    OLED_WriteString("Clear", Font_11x18, WHITE);
    OLED_SetCursor(3, 5+18+18);
    OLED_WriteString("Electronics", Font_11x18, WHITE);

    return;
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

  OLED_Init();

  HAL_Delay(1000);
  OLED_Fill(BLACK);
  OLED_UpdateScreen();
  HAL_Delay(1000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	    // Button handling - read the button states
	    current_button_state[0] = HAL_GPIO_ReadPin(HMI_BTN_2_GPIO_Port, HMI_BTN_2_Pin);
	    current_button_state[1] = HAL_GPIO_ReadPin(HMI_BTN_3_GPIO_Port, HMI_BTN_3_Pin);
	    current_button_state[2] = HAL_GPIO_ReadPin(HMI_BTN_4_GPIO_Port, HMI_BTN_4_Pin);

	    // Left button
	    if ((current_button_state[0] != previous_button_state[0]) && (GPIO_PIN_RESET == current_button_state[0]))
	    {
	        if(OLED_STATE_CCE == test_state)
	        {
	            test_state = OLED_STATE_POLYLINE;
	        }
	        else
	        {
	            test_state -= 1;
	        }

	        // Cleanup the screen
	        OLED_Fill(BLACK);
	        OLED_UpdateScreen();
	    }

	    // Right button
	    if ((current_button_state[2] != previous_button_state[2]) && (GPIO_PIN_RESET == current_button_state[2]))
	    {
	        if(OLED_STATE_POLYLINE == test_state)
	        {
	            test_state = OLED_STATE_CCE;
	        }
	        else
	        {
	            test_state += 1;
	        }

	        // Cleanup the screen
	        OLED_Fill(BLACK);
	        OLED_UpdateScreen();
	    }

	    // Inverse color button
	    if ((current_button_state[1] != previous_button_state[1]) && (GPIO_PIN_RESET == current_button_state[1]))
	    {
	        if(OFF == OLED_GetDisplayInverse())
	        {
	            // Set inverse color
	            OLED_SetDisplayInverse(ON);
	        }
	        else
	        {
	            // Set normal color
	            OLED_SetDisplayInverse(OFF);
	        }
	    }

	    // Button state can be monitored on LEDs
	    HAL_GPIO_WritePin(HMI_LED_2_GPIO_Port, HMI_LED_2_Pin, current_button_state[0]);
	    HAL_GPIO_WritePin(HMI_LED_3_GPIO_Port, HMI_LED_3_Pin, current_button_state[1]);
	    HAL_GPIO_WritePin(HMI_LED_4_GPIO_Port, HMI_LED_4_Pin, current_button_state[2]);

	    // Save the current button states
	    previous_button_state[0] = current_button_state[0];
	    previous_button_state[1] = current_button_state[1];
	    previous_button_state[2] = current_button_state[2];

	    // Select OLED test function
	    switch (test_state)
	    {
	    case OLED_STATE_FONTS:			// Character set review
	        OLED_TestFonts();
	        break;
	    case OLED_STATE_LINE:			// Random Line generator
	        OLED_TestLine();
	        break;
	    case OLED_STATE_RECTANGLE:		// Rectangle screen saver
	        OLED_TestRectangle();
	        break;
	    case OLED_STATE_CIRCLE:			// Pipe - Circle screen saver
	        OLED_TestCircle(&start_x, &start_r);
	        break;
	    case OLED_STATE_ARC:			// Arc
	        OLED_TestArc(&start_angle);
	        break;
	    case OLED_STATE_POLYLINE:		// Polyline
	        OLED_TestPolyline();
	        break;
	    case OLED_STATE_CCE:   			 // Crystal Clear Electronics
	    default:
	        OLED_TestCCE();
	        break;
	    }

	    OLED_UpdateScreen();
	    HAL_Delay(50);

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
