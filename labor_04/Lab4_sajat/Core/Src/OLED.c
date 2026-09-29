#include <math.h>
#include <OLED.h>
#include <stdlib.h>
#include <string.h>

// Degree to radian conversion: Pi/180 degrees
#define OLED_DEG_TO_RADIAN              0.01745329

#define CIRCLE_APPROXIMATION_SEGMENTS   36

// Screenbuffer
static uint8_t OLED_Buffer[OLED_BUFFER_SIZE];

// Screen object
static OLED_t SSD1306;

/**
  * @brief  Send a byte to the command register.
  * @param  Byte to be copied.
  * @retval None
  */
void OLED_WriteCommand(uint8_t byte)
{
    HAL_I2C_Mem_Write(&OLED_I2C_PORT, OLED_I2C_ADDR, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
    return;
}

/**
  * @brief  Send data to the command register.
  * @param  Values in buffer to be copied.
  * @param  Size of the buffer.
  * @retval None
  */
void OLED_WriteData(uint8_t* buffer, size_t buff_size)
{
    HAL_I2C_Mem_Write(&OLED_I2C_PORT, OLED_I2C_ADDR, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
    return;
}

/**
  * @brief  Sets Display ON/OFF.
  * @param  Switch value: ON or OFF.
  * @retval None.
  */
void OLED_SetDisplay(OLED_Switch display_switch)
{
    uint8_t command;

    if (ON == display_switch)
    {
        // Set display on
        command = 0xAF;
        SSD1306.DisplayOn = 1;
    }
    else
    {
        // Set display off
        command = 0xAE;
        SSD1306.DisplayOn = 0;
    }

    // Send the selected command value to the OLED display
    OLED_WriteCommand(command);
    
    return;
}

/**
  * @brief  Sets Display Inverse ON/OFF.
  * @param  Switch value: ON or OFF.
  * @retval None.
  */
void OLED_SetDisplayInverse(OLED_Switch inverse_switch)
{
    uint8_t command;

    if (ON == inverse_switch)
    {
        // Set inverse color
        command = 0xA7;
        SSD1306.Inverted = 1;
    }
    else
    {
        // Set normal color
        command = 0xA6;
        SSD1306.Inverted = 0;
    }

    // Send the selected command value to the OLED display
    OLED_WriteCommand(command);

    return;
}

/**
  * @brief  Get Display Inverse state.
  * @param  None.
  * @retval Display Inverse state.
  */
OLED_Switch OLED_GetDisplayInverse(void)
{
    return (SSD1306.Inverted);
}

/**
  * @brief  Sets the contrast of the display.
  * @param  Contrast value to set.
  * @retval None.
  */
void OLED_SetContrast(const uint8_t value)
{
    // Contrast Control Register
    OLED_WriteCommand(0x81);
    
    // Send the selected command value to the OLED display
    OLED_WriteCommand(value);
    
    return;
}

/**
  * @brief  Sets the position of cursor.
  * @param  Position on X axis.
  * @param  Position on Y axis.
  * @retval None.
  */
void OLED_SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;

    return;
}

/**
  * @brief  Sets the screen with the given color.
  * @param  Color to set.
  * @retval None.
  */
void OLED_Fill(OLED_Color color)
{
    uint32_t i;
    uint8_t color_value = 0x00u;	// BLACK

    // Set the output value based on the selected color
    if (WHITE == color)
    {
    	color_value = 0xFF;
    }
    // Set the values in the buffer
    for (i = 0; i < OLED_BUFFER_SIZE; i++)
    {
        OLED_Buffer[i] = color_value;
    }
    
    return;
}

/**
  * @brief  Update the screen using the buffer of screen.
  * @param  None.
  * @retval None.
  */
void OLED_UpdateScreen(void)
{
    // Iterates through the whole buffer
    for (uint8_t i = 0; i < (OLED_HEIGHT/8); i++)
    {
        // Set the current RAM page address
        OLED_WriteCommand(0xB0 + i);
        OLED_WriteCommand(0x00);
        OLED_WriteCommand(0x10);
        
        // Write the values
        OLED_WriteData(&OLED_Buffer[OLED_WIDTH*i],OLED_WIDTH);
    }
    
    return;
}

/**
  * @brief  Initializes the OLED screen.
  * @param  None.
  * @retval None.
  */
void OLED_Init(void)
{
    // Wait for the screen to boot
    HAL_Delay(100);

    // Turn display off
    OLED_SetDisplay(OFF);

    // Set display clock divide ratio/oscillator frequency
    // 0x80 = 150 Hz
    OLED_WriteCommand(0xD5);
    OLED_WriteCommand(0xF0);

    // Select multiplex ratio
    // Default => 0~0x3F (1/64 duty)
    OLED_WriteCommand(0xA8);
    OLED_WriteCommand(0x3F);

    // Setting display offset
    // 0x00 Reset, set common start
    OLED_WriteCommand(0xD3);
    OLED_WriteCommand(0x00);

    // Set display start line
    OLED_WriteCommand(0x40);

    // Set Charge Pump
    // Enable Charge Pump
    OLED_WriteCommand(0x8D);
    OLED_WriteCommand(0x14);

    // Set Segment Re-Map
#ifdef OLED_MIRROR_HORIZ
    // Column address 0 mapped to 127
    // Mirror the screen horizontally
    OLED_WriteCommand(0xA0);
#else
    // Column address 127 mapped to 0
    OLED_WriteCommand(0xA1);
#endif

    // Set COM output scan direction
#ifdef OLED_MIRROR_VERT
    // Mirror the screen vertically
    OLED_WriteCommand(0xC0);
#else
    // Set COM Output Scan Direction
    OLED_WriteCommand(0xC8);
#endif

    // Set COM Hardware Configuration
    // Alternative COM pin - See IC spec page 34
    OLED_WriteCommand(0xDA);
    OLED_WriteCommand(0x12);

    // Set the contrast
    OLED_SetContrast(0xFF);

    // Set Pre-charge Period
    OLED_WriteCommand(0xD9);
    OLED_WriteCommand(0x22);

    // Set Vcomh Deselect Level
    // 0x20 - 0.77xVcc
    OLED_WriteCommand(0xDB);
    OLED_WriteCommand(0x20);

    // Set the output
    // 0xa4 - Output follows RAM content
    // 0xa5 - Output ignores RAM content
    OLED_WriteCommand(0xA4);

    // Inverse
#ifdef OLED_INVERSE_COLOR
    // Set inverse color
    OLED_SetDisplayInverse(ON);
#else
    // Set normal color
    OLED_SetDisplayInverse(OFF);
#endif

    // Set Memory Addressing Mode
    // 0b00 - Horizontal Addressing Mode
    // 0b01 - Vertical Addressing Mode
    // 0b10 - Page Addressing Mode (RESET)
    // 0b11 - Invalid
    OLED_WriteCommand(0x20);
    OLED_WriteCommand(0x00); 

    //Set Page Start Address for Page Addressing Mode, 0-7
    OLED_WriteCommand(0xB0); 

    // Set low and high column addresses
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x10);
    
    // Turn display on
    OLED_SetDisplay(ON);

    // Clear the whole screen
    OLED_Fill(BLACK);
    
    // Flush the buffer to the screen
    OLED_UpdateScreen();
    
    // Set default values for screen object
    OLED_SetCursor(0, 0);
    
    // OLED display initialization was successful
    SSD1306.Initialized = 1;
}

/**
  * @brief  Draw one pixel
  * @param  Position on X axis.
  * @param  Position on Y axis.
  * @param  Color of pixels.
  * @retval None.
  */
void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_Color color)
{
    // Range of the display shall be checked
    if (OLED_WIDTH > x && OLED_HEIGHT > y)
    {
        // Draw pixel
        if (color == WHITE)
        {
            OLED_Buffer[x + (y / 8) * OLED_WIDTH] |= 1 << (y % 8);
        }
        else
        {
            OLED_Buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
        } 
    }
    
    return;
}

/**
  * @brief  Draw line using Bresenham's algorithm
  * @param  X coordinate of start point.
  * @param  Y coordinate of start point.
  * @param  X coordinate of stop point.
  * @param  Y coordinate of stop point.
  * @param  Color of pixels.
  * @retval None.
  */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_Color color)
{
    int32_t deltaX = abs(x2 - x1);
    int32_t deltaY = abs(y2 - y1);
    int32_t signX = ((x1 < x2) ? 1 : -1);
    int32_t signY = ((y1 < y2) ? 1 : -1);
    int32_t error = deltaX - deltaY;
    int32_t error2;

    OLED_DrawPixel(x2, y2, color);
    while ((x1 != x2) || (y1 != y2))
    {
        OLED_DrawPixel(x1, y1, color);
        error2 = error * 2;

        if (error2 > -deltaY)
        {
            error -= deltaY;
            x1 += signX;
        }

        if (error2 < deltaX)
        {
            error += deltaX;
            y1 += signY;
        }
    }

    return;
}

/**
  * @brief  Draw rectangle
  * @param  X coordinate of start point.
  * @param  Y coordinate of start point.
  * @param  X coordinate of stop point.
  * @param  Y coordinate of stop point.
  * @param  Color of pixels.
  * @retval None.
  */
void OLED_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_Color color)
{
    // Draw rectangle line by line
    OLED_DrawLine(x1, y1, x2, y1, color);
    OLED_DrawLine(x2, y1, x2, y2, color);
    OLED_DrawLine(x2, y2, x1, y2, color);
    OLED_DrawLine(x1, y2, x1, y1, color);

    return;
}

/**
  * @brief  Draw polyline
  * @param  Input buffer, that contains the coordinates.
  * @param  Size of the input buffer.
  * @param  Color of pixels.
  * @retval None.
  */
void OLED_DrawPolyline(const OLED_Vertex *vertex, uint16_t size, OLED_Color color)
{
    uint16_t i;

    if (0 != vertex)
    {
        for (i = 1; i < size; i++)
        {
            OLED_DrawLine(vertex[i - 1].x, vertex[i - 1].y, vertex[i].x, vertex[i].y, color);
        }
    }

    return;
}

/**
  * @brief  Draw circle based on Bresenham's circle drawing algorithm
  * @param  X coordinate of origo.
  * @param  Y coordinate of origo.
  * @param  Radius in pixel.
  * @param  Color of pixel.
  * @retval None.
  */
void OLED_DrawCircle(uint8_t x_coordinate, uint8_t y_coordinate, uint8_t radius, OLED_Color color)
{
    int32_t x = -radius;
    int32_t y = 0;
    int32_t error = 2 - 2 * radius;
    int32_t error2;

    // Range of the display shall be checked
    if (OLED_WIDTH > x && OLED_HEIGHT > y)
    {
        do {
            OLED_DrawPixel(x_coordinate - x, y_coordinate + y, color);
            OLED_DrawPixel(x_coordinate + x, y_coordinate + y, color);
            OLED_DrawPixel(x_coordinate + x, y_coordinate - y, color);
            OLED_DrawPixel(x_coordinate - x, y_coordinate - y, color);

            error2 = error;

            if (error2 <= y)
            {
                y++;
                error = error + (y * 2 + 1);

                if (-x == y && error2 <= x)
                {
                    error2 = 0;
                }
            }
            if (error2 > x)
            {
                x++;
                error = error + (x * 2 + 1);
            }
        } while (x <= 0);
    }

    return;
}

/**
  * @brief  Normalize angle to [0;360]
  * @param  Input angle to be normalized.
  * @retval None.
  */
static uint16_t OLED_NormalizeAngle(uint16_t angle)
{
  uint16_t loc_angle;

  if (angle <= 360)
  {
    loc_angle = angle;
  }
  else
  {
    loc_angle = angle % 360;
    loc_angle = ((0 != angle) ? angle : 360);
  }
  return (loc_angle);
}

/**
  * @brief  Draw arc
  * @param  X coordinate of origo.
  * @param  Y coordinate of origo.
  * @param  Radius in pixel.
  * @param  Start angle in degree, that is beginning from 4 quart of trigonometric circle
  * @param  Stop angle in degree
  * @param  Color of pixel.
  * @retval None.
  */
void OLED_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, OLED_Color color)
{
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1,xp2;
    uint8_t yp1,yp2;
    uint32_t count = 0;
    uint32_t loc_sweep = 0;
    float rad;

    loc_sweep = OLED_NormalizeAngle(sweep);
    
    count = (OLED_NormalizeAngle(start_angle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;

    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;

    while (count < approx_segments)
    {
        rad = count * approx_degree * OLED_DEG_TO_RADIAN;
        xp1 = x + (int8_t)(sin(rad)*radius);
        yp1 = y + (int8_t)(cos(rad)*radius);

        count++;

        if (count != approx_segments)
        {
            rad = count * approx_degree * OLED_DEG_TO_RADIAN;
        }
        else
        {            
            rad = loc_sweep * OLED_DEG_TO_RADIAN;
        }

        xp2 = x + (int8_t)(sin(rad)*radius);
        yp2 = y + (int8_t)(cos(rad)*radius);

        OLED_DrawLine(xp1,yp1,xp2,yp2,color);
    }
    
    return;
}

/**
  * @brief  Draw character to the screen buffer.
  * @param  Character you want to draw.
  * @param  Font defines the character set you want to use.
  * @param  Color of character.
  * @retval Written char for validation.
  */
char OLED_WriteChar(char ch, FontDef Font, OLED_Color color)
{
    uint8_t i, j;
    uint16_t font_part;
    char retVal = 0;
    
    // Check if character is valid
    if (32 <= ch && 126 >= ch)
    {
        // Check remaining space on current line
        if (OLED_WIDTH >= (SSD1306.CurrentX + Font.FontWidth) && OLED_HEIGHT >= (SSD1306.CurrentY + Font.FontHeight))
        {
            // Use the font to write
            for (i = 0; i < Font.FontHeight; i++)
            {
            	font_part = Font.data[(ch - 32) * Font.FontHeight + i];
                
                for (j = 0; j < Font.FontWidth; j++)
                {
                    if ((font_part << j) & 0x8000)
                    {
                        OLED_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (OLED_Color) color);
                    }
                    else
                    {
                        OLED_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (OLED_Color)!color);
                    }
                }
            }

            // The current space is now taken
            SSD1306.CurrentX += Font.FontWidth;
            
            // Return written char for validation
            retVal = ch;            
        }
    }

    // Char or zero (in case of invalid character or not enough space on screen)
    return (retVal);
}

/**
  * @brief  Draw strin to the screen buffer.
  * @param  String you want to draw.
  * @param  Font defines the character set you want to use.
  * @param  Color of character.
  * @retval Written char for validation.
  */
char OLED_WriteString(char* str, FontDef Font, OLED_Color color)
{
    // Write until null-byte
    while (*str) 
    {
        if (OLED_WriteChar(*str, Font, color) != *str) 
        {
            // Char could not be written
            return (*str);
        }

        // Next char
        str++;
    }

    // Everything ok
    return (*str);
}
