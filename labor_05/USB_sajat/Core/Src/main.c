/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : USB HID eger 6 gombbal – STM32F446 (Nucleo-F446ZE)
  *
  *  Gomb1  PE1 -> fel
  *  Gomb2  PE2 -> le
  *  Gomb3  PE3 -> balra
  *  Gomb4  PE4 -> jobbra
  *  Gomb5  PE5 -> bal klikk
  *  Gomb6  PE6 -> jobb klikk
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include "usbd_hid.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define BTN_PORT        GPIOE
#define BTN_UP_Pin      GPIO_PIN_1   /* Gomb1 */
#define BTN_DOWN_Pin    GPIO_PIN_2   /* Gomb2 */
#define BTN_LEFT_Pin    GPIO_PIN_3   /* Gomb3 */
#define BTN_RIGHT_Pin   GPIO_PIN_4   /* Gomb4 */
#define BTN_LCLICK_Pin  GPIO_PIN_5   /* Gomb5 */
#define BTN_RCLICK_Pin  GPIO_PIN_6   /* Gomb6 */

#define MOUSE_STEP_SIZE   4
#define MOUSE_BTN_LEFT    0x01   /* bit0 */
#define MOUSE_BTN_RIGHT   0x02   /* bit1 */
/* USER CODE END PD */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void Buttons_GPIO_Init(void);

/* USER CODE BEGIN 0 */
struct mouse_report_t {
    uint8_t buttons;
    int8_t  x;
    int8_t  y;
    int8_t  wheel;
};
struct mouse_report_t mouseHID = {0};

extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  Buttons_GPIO_Init();
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    mouseHID.buttons = 0;
    mouseHID.x = 0;
    mouseHID.y = 0;

    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_LCLICK_Pin)) mouseHID.buttons |= MOUSE_BTN_LEFT;
    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_RCLICK_Pin)) mouseHID.buttons |= MOUSE_BTN_RIGHT;

    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_UP_Pin))    mouseHID.y = -MOUSE_STEP_SIZE;
    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_DOWN_Pin))  mouseHID.y =  MOUSE_STEP_SIZE;
    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_LEFT_Pin))  mouseHID.x = -MOUSE_STEP_SIZE;
    if (HAL_GPIO_ReadPin(BTN_PORT, BTN_RIGHT_Pin)) mouseHID.x =  MOUSE_STEP_SIZE;

    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&mouseHID, sizeof(mouseHID));

    HAL_Delay(50);  */
  }
  /* USER CODE END WHILE */
}

static void Buttons_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();   /* USB: PA11, PA12 */

  GPIO_InitStruct.Pin  = BTN_UP_Pin | BTN_DOWN_Pin | BTN_LEFT_Pin
                       | BTN_RIGHT_Pin | BTN_LCLICK_Pin | BTN_RCLICK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(BTN_PORT, &GPIO_InitStruct);
}

/**
  * @brief System Clock Configuration
  *        HSE 8 MHz (bypass, ST-LINK) -> SYSCLK 168 MHz, USB 48 MHz
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;    /* 336 / 7 = 48 MHz az USB-nek */
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
