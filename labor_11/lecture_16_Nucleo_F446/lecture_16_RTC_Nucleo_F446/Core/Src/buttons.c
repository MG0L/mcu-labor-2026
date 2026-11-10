#include <string.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_cortex.h"
#include "main.h"
#include "buttons.h"

static Buttons_TypeDef Buttons;

/**
 * @brief If there is an interrupt caused by a button, then this function will be called
 * @param[in] GPIO_Pin Button identification number
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  ButtonState_TypeDef state;

  // Read the state of the button causing the interrupt and save it
  if (GPIO_Pin == HMI_BTN_1_Pin)
  {
	  state = (ButtonState_TypeDef)HAL_GPIO_ReadPin(HMI_BTN_1_GPIO_Port, HMI_BTN_1_Pin);
	  if (state != Buttons.up.state && Buttons.up.updated == false)
	  {
		  Buttons.up.state = state;
		  Buttons.up.updated = true;
	  }
  }
  if (GPIO_Pin == HMI_BTN_2_Pin)
  {
	  state = (ButtonState_TypeDef)HAL_GPIO_ReadPin(HMI_BTN_2_GPIO_Port, HMI_BTN_2_Pin);
	  if (state != Buttons.left.state && Buttons.left.updated == false)
	  {
		  Buttons.left.state = state;
		  Buttons.left.updated = true;
	  }
  }
  if (GPIO_Pin == HMI_BTN_3_Pin)
  {
	  state = (ButtonState_TypeDef)HAL_GPIO_ReadPin(HMI_BTN_3_GPIO_Port, HMI_BTN_3_Pin);
	  if (state != Buttons.middle.state && Buttons.middle.updated == false)
	  {
		  Buttons.middle.state = state;
		  Buttons.middle.updated = true;
	  }
  }
  if (GPIO_Pin == HMI_BTN_4_Pin)
  {
	  state = (ButtonState_TypeDef)HAL_GPIO_ReadPin(HMI_BTN_4_GPIO_Port, HMI_BTN_4_Pin);
	  if (state != Buttons.right.state && Buttons.right.updated == false)
	  {
		  Buttons.right.state = state;
		  Buttons.right.updated = true;
	  }
  }
  if (GPIO_Pin == HMI_BTN_5_Pin)
  {
	  state = (ButtonState_TypeDef)HAL_GPIO_ReadPin(HMI_BTN_5_GPIO_Port, HMI_BTN_5_Pin);
	  if (state != Buttons.down.state && Buttons.down.updated == false)
	  {
		  Buttons.down.state = state;
		  Buttons.down.updated = true;
	  }
  }
}

/**
 * @brief               Get the actual state of the buttons
 * @param[out] buttons  The received button struct, which is filled with the new data
 * @note                This function clear the update flags of the buttons.
 */
void Buttons_GetStates(Buttons_TypeDef * const buttons){
  // Disable the interrupts to avoid competing data access
  HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
  HAL_NVIC_DisableIRQ(EXTI4_IRQn);
  HAL_NVIC_DisableIRQ(EXTI3_IRQn);
  HAL_NVIC_DisableIRQ(EXTI2_IRQn);

  *buttons = Buttons;

  Buttons.down.updated = false;
  Buttons.left.updated = false;
  Buttons.middle.updated = false;
  Buttons.right.updated = false;
  Buttons.up.updated = false;

  // re-enable the interrupts
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

}
