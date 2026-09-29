/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Snake jatek SSD1306 OLED kijelzon, 3 gombbal
  ******************************************************************************
  * Gombok:
  *   1-es gomb (PE2) : elore (jobbra)
  *   2-es gomb (PE3) : lefele
  *   3-as gomb (PE6) : felfele
  *
  * A palya szelei "atjarhatok" (ha kimesz jobbra, balrol jossz vissza,
  * felul/alul ugyanigy). Jatek vege: ha a kigyo sajat magaba harap.
  *
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include <stdlib.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
    DIR_RIGHT = 0,
    DIR_UP,
    DIR_DOWN
} Snake_Dir;

typedef enum
{
    GAME_START = 0,
    GAME_PLAY,
    GAME_OVER
} Game_State;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Gomb hozzarendeles - ha a sorrend mas, csak itt kell atirni */
#define BTN1_PORT   GPIOE           /* elore / jobbra */
#define BTN1_PIN    GPIO_PIN_2
#define BTN2_PORT   GPIOE           /* le */
#define BTN2_PIN    GPIO_PIN_3
#define BTN3_PORT   GPIOE           /* fel */
#define BTN3_PIN    GPIO_PIN_6

/* Lenyomott gomb szintje. A peldaprogramban aktiv alacsony (RESET).
   Ha nalad forditva mukodik, ird at GPIO_PIN_SET-re. */
#define BTN_PRESSED_LEVEL   GPIO_PIN_RESET

/* Racs: 4x4 pixeles cellak -> 32 x 16 cella a 128x64-es kijelzon */
#define CELL_SIZE       4u
#define GRID_W          (OLED_WIDTH  / CELL_SIZE)
#define GRID_H          (OLED_HEIGHT / CELL_SIZE)
#define SNAKE_MAX_LEN   (GRID_W * GRID_H)

/* Sebesseg (ms / lepes) */
#define SPEED_START_MS  160u
#define SPEED_MIN_MS    70u
#define SPEED_STEP_MS   5u

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
static GPIO_PinState previous_button_state[3];
static GPIO_PinState current_button_state[3];

static uint8_t   snake_x[SNAKE_MAX_LEN];
static uint8_t   snake_y[SNAKE_MAX_LEN];
static uint16_t  snake_len;
static Snake_Dir snake_dir;
static Snake_Dir next_dir;

static uint8_t   food_x;
static uint8_t   food_y;

static uint16_t  score;
static uint32_t  move_period;
static uint32_t  last_move_tick;
static uint32_t  game_over_tick;

static Game_State game_state = GAME_START;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Gombok beolvasasa, pressed[i] = 1 ha az adott gombot most nyomtak le (el) */
static void Buttons_Read(uint8_t pressed[3])
{
    uint8_t i;

    current_button_state[0] = HAL_GPIO_ReadPin(BTN1_PORT, BTN1_PIN);
    current_button_state[1] = HAL_GPIO_ReadPin(BTN2_PORT, BTN2_PIN);
    current_button_state[2] = HAL_GPIO_ReadPin(BTN3_PORT, BTN3_PIN);

    for (i = 0; i < 3u; i++)
    {
        pressed[i] = ((current_button_state[i] != previous_button_state[i]) &&
                      (BTN_PRESSED_LEVEL == current_button_state[i])) ? 1u : 0u;
        previous_button_state[i] = current_button_state[i];
    }

    /* Visszajelzes a Nucleo LED-jein: vilagit, amig a gomb le van nyomva */
    HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin,
        (BTN_PRESSED_LEVEL == current_button_state[0]) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin,
        (BTN_PRESSED_LEVEL == current_button_state[1]) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin,
        (BTN_PRESSED_LEVEL == current_button_state[2]) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* Teli 3x3-as negyzet egy cellaba (kigyo teste) */
static void Snake_DrawCell(uint8_t gx, uint8_t gy)
{
    uint8_t px = gx * CELL_SIZE;
    uint8_t py = gy * CELL_SIZE;
    uint8_t i;

    for (i = 0; i < (CELL_SIZE - 1u); i++)
    {
        OLED_DrawLine(px, py + i, px + CELL_SIZE - 2u, py + i, WHITE);
    }
}

/* Ures keret egy cellaba (etel) */
static void Snake_DrawFood(uint8_t gx, uint8_t gy)
{
    uint8_t px = gx * CELL_SIZE;
    uint8_t py = gy * CELL_SIZE;

    OLED_DrawRectangle(px, py, px + CELL_SIZE - 2u, py + CELL_SIZE - 2u, WHITE);
}

static uint8_t Snake_IsOnBody(uint8_t x, uint8_t y)
{
    uint16_t i;

    for (i = 0; i < snake_len; i++)
    {
        if ((snake_x[i] == x) && (snake_y[i] == y))
        {
            return 1u;
        }
    }
    return 0u;
}

/* Uj etel lerakasa szabad helyre. 0-val ter vissza, ha nincs szabad hely (nyertel) */
static uint8_t Snake_SpawnFood(void)
{
    if (snake_len >= SNAKE_MAX_LEN)
    {
        return 0u;
    }

    do
    {
        food_x = (uint8_t)(rand() % GRID_W);
        food_y = (uint8_t)(rand() % GRID_H);
    } while (Snake_IsOnBody(food_x, food_y));

    return 1u;
}

static void Snake_Init(void)
{
    snake_len  = 3u;
    snake_x[0] = 5u; snake_y[0] = GRID_H / 2u;
    snake_x[1] = 4u; snake_y[1] = GRID_H / 2u;
    snake_x[2] = 3u; snake_y[2] = GRID_H / 2u;

    snake_dir = DIR_RIGHT;
    next_dir  = DIR_RIGHT;

    score          = 0u;
    move_period    = SPEED_START_MS;
    last_move_tick = HAL_GetTick();

    (void)Snake_SpawnFood();
}

/* Egy lepes. 0-val ter vissza, ha vege a jateknak */
static uint8_t Snake_Step(void)
{
    uint8_t  nx = snake_x[0];
    uint8_t  ny = snake_y[0];
    uint8_t  grow;
    uint16_t check_len;
    uint16_t i;

    snake_dir = next_dir;

    switch (snake_dir)
    {
    case DIR_UP:
        ny = (0u == ny) ? (GRID_H - 1u) : (ny - 1u);
        break;
    case DIR_DOWN:
        ny = (uint8_t)((ny + 1u) % GRID_H);
        break;
    case DIR_RIGHT:
    default:
        nx = (uint8_t)((nx + 1u) % GRID_W);
        break;
    }

    grow = ((nx == food_x) && (ny == food_y)) ? 1u : 0u;

    /* Utkozes sajat magaval. Ha nem no, a farok elmozdul, azt nem nezzuk. */
    check_len = grow ? snake_len : (snake_len - 1u);
    for (i = 0; i < check_len; i++)
    {
        if ((snake_x[i] == nx) && (snake_y[i] == ny))
        {
            return 0u;
        }
    }

    if (grow && (snake_len < SNAKE_MAX_LEN))
    {
        snake_len++;
    }

    /* Test tolasa hatra, uj fej elore */
    for (i = snake_len - 1u; i > 0u; i--)
    {
        snake_x[i] = snake_x[i - 1u];
        snake_y[i] = snake_y[i - 1u];
    }
    snake_x[0] = nx;
    snake_y[0] = ny;

    if (grow)
    {
        score++;
        if (move_period > SPEED_MIN_MS)
        {
            move_period -= SPEED_STEP_MS;
        }
        if (!Snake_SpawnFood())
        {
            return 0u;  /* tele a palya - nyertel */
        }
    }

    return 1u;
}

static void Snake_Draw(void)
{
    uint16_t i;

    OLED_Fill(BLACK);
    for (i = 0; i < snake_len; i++)
    {
        Snake_DrawCell(snake_x[i], snake_y[i]);
    }
    Snake_DrawFood(food_x, food_y);
    OLED_UpdateScreen();
}

static void Snake_DrawStart(void)
{
    OLED_Fill(BLACK);
    OLED_SetCursor(24, 8);
    OLED_WriteString("SNAKE", Font_16x26, WHITE);
    OLED_SetCursor(22, 46);
    OLED_WriteString("Nyomj gombot", Font_7x10, WHITE);
    OLED_UpdateScreen();
}

static void Snake_DrawGameOver(void)
{
    char buf[20];

    OLED_Fill(BLACK);
    OLED_SetCursor(14, 6);
    OLED_WriteString("GAME OVER", Font_11x18, WHITE);

    snprintf(buf, sizeof(buf), "Pont: %u", (unsigned int)score);
    OLED_SetCursor(30, 30);
    OLED_WriteString(buf, Font_7x10, WHITE);

    OLED_SetCursor(26, 48);
    OLED_WriteString("Gomb = ujra", Font_7x10, WHITE);
    OLED_UpdateScreen();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint8_t pressed[3];
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
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

  OLED_Init();
  OLED_Fill(BLACK);
  OLED_UpdateScreen();

  /* Kezdo gomballapot, hogy indulaskor ne legyen "hamis" lenyomas */
  previous_button_state[0] = HAL_GPIO_ReadPin(BTN1_PORT, BTN1_PIN);
  previous_button_state[1] = HAL_GPIO_ReadPin(BTN2_PORT, BTN2_PIN);
  previous_button_state[2] = HAL_GPIO_ReadPin(BTN3_PORT, BTN3_PIN);

  Snake_DrawStart();
  game_state = GAME_START;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      Buttons_Read(pressed);

      switch (game_state)
      {
      case GAME_START:
          if (pressed[0] || pressed[1] || pressed[2])
          {
              srand(HAL_GetTick());   /* a lenyomas ideje ad veletlenszeru kezdoerteket */
              Snake_Init();
              Snake_Draw();
              game_state = GAME_PLAY;
          }
          break;

      case GAME_PLAY:
          /* Iranyvaltas - a kigyo nem fordulhat vissza sajat magaba */
          if (pressed[0])                                  /* 1-es gomb: elore */
          {
              next_dir = DIR_RIGHT;
          }
          if (pressed[1] && (DIR_UP != snake_dir))         /* 2-es gomb: le */
          {
              next_dir = DIR_DOWN;
          }
          if (pressed[2] && (DIR_DOWN != snake_dir))       /* 3-as gomb: fel */
          {
              next_dir = DIR_UP;
          }

          if ((HAL_GetTick() - last_move_tick) >= move_period)
          {
              last_move_tick = HAL_GetTick();

              if (Snake_Step())
              {
                  Snake_Draw();
              }
              else
              {
                  Snake_DrawGameOver();
                  game_over_tick = HAL_GetTick();
                  game_state = GAME_OVER;
              }
          }
          break;

      case GAME_OVER:
      default:
          /* Kis varakozas, hogy egy veletlen gombnyomas ne inditsa rogton ujra */
          if (((HAL_GetTick() - game_over_tick) > 700u) &&
              (pressed[0] || pressed[1] || pressed[2]))
          {
              Snake_Init();
              Snake_Draw();
              game_state = GAME_PLAY;
          }
          break;
      }

      HAL_Delay(5);

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

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE6 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
