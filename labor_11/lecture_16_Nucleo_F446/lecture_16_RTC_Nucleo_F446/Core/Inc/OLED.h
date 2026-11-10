#ifndef __OLED_H__
#define __OLED_H__

#include <stddef.h>

#include "OLED_conf.h"


#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#include "OLED_fonts.h"

extern I2C_HandleTypeDef OLED_I2C_PORT;

// OLED height in pixels
#define OLED_HEIGHT          64

// OLED width in pixels
#define OLED_WIDTH           128

#define OLED_BUFFER_SIZE   OLED_WIDTH * OLED_HEIGHT / 8

// Enumeration for screen colors
typedef enum
{
    Black = 0x00, // Black color, no pixel
    White = 0x01  // Pixel is set. Color depends on OLED
} OLED_COLOR;

typedef enum
{
    OLED_OK = 0x00,
    OLED_ERR = 0x01  // Generic error.
} OLED_Error_t;

// Struct to store transformations
typedef struct
{
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
    uint8_t DisplayOn;
} OLED_t;
typedef struct
{
    uint8_t x;
    uint8_t y;
} OLED_VERTEX;

void OLED_Init(void);
void OLED_Fill(OLED_COLOR color);
void OLED_UpdateScreen(void);
void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_COLOR color);
char OLED_WriteChar(char ch, FontDef Font, OLED_COLOR color);
char OLED_WriteString(char* str, FontDef Font, OLED_COLOR color);
void OLED_SetCursor(uint8_t x, uint8_t y);
void OLED_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_COLOR color);
void OLED_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, OLED_COLOR color);
void OLED_DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, OLED_COLOR color);
void OLED_Polyline(const OLED_VERTEX *par_vertex, uint16_t par_size, OLED_COLOR color);
void OLED_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_COLOR color);

/**
 * @brief Sets the contrast of the display.
 * @param[in] value contrast to set.
 * @note Contrast increases as the value increases.
 * @note RESET = 7Fh.
 */

void OLED_SetContrast(const uint8_t value);
/**
 * @brief Set Display ON/OFF.
 * @param[in] on 0 for OFF, any for ON.
 */

void OLED_SetDisplayOn(const uint8_t on);

/**
 * @brief Reads DisplayOn state.
 * @return  0: OFF.
 *          1: ON.
 */
uint8_t OLED_GetDisplayOn();

void OLED_WriteCommand(uint8_t byte);

void OLED_WriteData(uint8_t* buffer, size_t buff_size);

OLED_Error_t OLED_FillBuffer(uint8_t* buf, uint32_t len);

#endif // __OLED_H__
