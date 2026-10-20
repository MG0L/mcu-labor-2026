#ifndef __OLED_H__
#define __OLED_H__

#include <stddef.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "OLED_fonts.h"

// Declaration of I2C port
extern I2C_HandleTypeDef OLED_I2C_PORT;

// Enumeration for screen colors
typedef enum
{
    BLACK = 0x00,       // Black color, no pixel
    WHITE = 0x01        // Pixel is set. Color depends on OLED
} OLED_Color;

// Enumeration for switch
typedef enum
{
    OFF = 0x00,         // Turn off the display
    ON = 0x01           // Turn on the display
} OLED_Switch;

// Struct to store properties of OLED display
typedef struct
{
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
    uint8_t DisplayOn;
} OLED_t;

// Struct to store the coordinates of vertex
typedef struct
{
    uint8_t x;
    uint8_t y;
} OLED_Vertex;

void OLED_WriteCommand(uint8_t byte);
void OLED_WriteData(uint8_t* buffer, size_t buff_size);
void OLED_SetDisplay(OLED_Switch display_switch);
void OLED_SetDisplayInverse(OLED_Switch inverse_switch);
OLED_Switch OLED_GetDisplayInverse(void);
void OLED_SetContrast(const uint8_t value);
void OLED_SetCursor(uint8_t x, uint8_t y);
void OLED_Fill(OLED_Color color);
void OLED_UpdateScreen(void);
void OLED_Init(void);
void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_Color color);
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_Color color);
void OLED_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_Color color);
void OLED_DrawPolyline(const OLED_Vertex *par_vertex, uint16_t par_size, OLED_Color color);
void OLED_DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, OLED_Color color);
void OLED_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, OLED_Color color);
char OLED_WriteChar(char ch, FontDef Font, OLED_Color color);
char OLED_WriteString(char* str, FontDef Font, OLED_Color color);

#endif // __OLED_H__
