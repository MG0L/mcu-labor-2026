#ifndef INC_BUTTONS_H_
#define INC_BUTTONS_H_

#include <stdbool.h>
//Enumeration for the state of the buttons
typedef enum
{
  BUTTON_RELEASED = 0,
  BUTTON_PRESSED
} ButtonState_TypeDef;

// Struct for edge detecting
typedef struct
{
  ButtonState_TypeDef state;   // state of the buttons
  bool updated;                // Are there any status change?
} Button_TypeDef;

// Structure for storing button parameters
typedef struct
{
  Button_TypeDef up;
  Button_TypeDef down;
  Button_TypeDef middle;
  Button_TypeDef left;
  Button_TypeDef right;
} Buttons_TypeDef;

// Get the state of the buttons
void Buttons_GetStates(Buttons_TypeDef * const buttons);

#endif /* INC_BUTTONS_H_ */
