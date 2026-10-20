#include <math.h>
#include <OLED.h>
#include <stdlib.h>
#include <string.h>

// Send a byte to the command register
void OLED_WriteCommand(uint8_t byte) {
    HAL_I2C_Mem_Write(&OLED_I2C_PORT, OLED_I2C_ADDR, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
}

// Send data
void OLED_WriteData(uint8_t* buffer, size_t buff_size) {
    HAL_I2C_Mem_Write(&OLED_I2C_PORT, OLED_I2C_ADDR, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}

// Screenbuffer
static uint8_t OLED_Buffer[OLED_BUFFER_SIZE];

// Screen object
static OLED_t SSD1306;

/* Fills the Screenbuffer with values from a given buffer of a fixed length */
OLED_Error_t OLED_FillBuffer(uint8_t* buf, uint32_t len)
{
    OLED_Error_t ret = OLED_ERR;

    if (len <= OLED_BUFFER_SIZE) {
        memcpy(OLED_Buffer,buf,len);
        ret = OLED_OK;
    }

    return ret;
}

// Initialize the OLED screen
void OLED_Init(void)
{

    // Wait for the screen to boot
    HAL_Delay(100);

    // Init OLED
    OLED_SetDisplayOn(0); 	 // Turn display off

    OLED_WriteCommand(0x20); // Set Memory Addressing Mode
    OLED_WriteCommand(0x00); // 0b00 - Horizontal Addressing Mode
    						 //	0b01 - Vertical Addressing Mode
                             // 0b10 - Page Addressing Mode (RESET)
    						 // 0b11 - Invalid

    OLED_WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode, 0-7

#ifdef OLED_MIRROR_VERT
    OLED_WriteCommand(0xC0); // Mirror screen vertically
#else
    OLED_WriteCommand(0xC8); // Set COM Output Scan Direction
#endif

    OLED_WriteCommand(0x00); // Set low column address
    OLED_WriteCommand(0x10); // Set high column address

    OLED_WriteCommand(0x40); // Set start line address

    OLED_SetContrast(0xFF);

#ifdef OLED_MIRROR_HORIZ
    OLED_WriteCommand(0xA0); // Mirror horizontally
#else
    OLED_WriteCommand(0xA1); // Set segment re-map 0 to 127
#endif

#ifdef OLED_INVERSE_COLOR
    OLED_WriteCommand(0xA7); // Set inverse color
#else
    OLED_WriteCommand(0xA6); // Set normal color
#endif

    // Set multiplex ratio.
    OLED_WriteCommand(0xA8); // Set multiplex ratio(1 to 64)

    OLED_WriteCommand(0x3F); // Set screen height to 64px


    OLED_WriteCommand(0xA4); // 0xa4 - Output follows RAM content
    						 // 0xa5 - Output ignores RAM content

    OLED_WriteCommand(0xD3); // Set display offset
    OLED_WriteCommand(0x00); // No offset

    OLED_WriteCommand(0xD5); // Set display clock divide ratio/oscillator frequency
    OLED_WriteCommand(0xF0); // Set divide ratio

    OLED_WriteCommand(0xD9); // Set pre-charge period
    OLED_WriteCommand(0x22); //

    OLED_WriteCommand(0xDA); // Set COM pins hardware configuration

    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0xDB); // Set Vcomh
    OLED_WriteCommand(0x20); // 0x20 - 0.77xVcc

    OLED_WriteCommand(0x8D); // Set DC-DC enable
    OLED_WriteCommand(0x14);
    OLED_SetDisplayOn(1);    // Turn on OLED panel

    // Clear screen
    OLED_Fill(Black);
    
    // Flush buffer to screen
    OLED_UpdateScreen();
    
    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;
    
    SSD1306.Initialized = 1;
}

// Fill the whole screen with the given color
void OLED_Fill(OLED_COLOR color)
{
    /* Set memory */
    uint32_t i;

    for(i = 0; i < sizeof(OLED_Buffer); i++) {
        OLED_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

// Write the screenbuffer with changed to the screen
void OLED_UpdateScreen(void)
{
    for(uint8_t i = 0; i < OLED_HEIGHT/8; i++)
    {
        OLED_WriteCommand(0xB0 + i); // Set the current RAM page address.
        OLED_WriteCommand(0x00);
        OLED_WriteCommand(0x10);
        OLED_WriteData(&OLED_Buffer[OLED_WIDTH*i],OLED_WIDTH);
    }
}

//    Draw one pixel in the screenbuffer
//    X => X Coordinate
//    Y => Y Coordinate
//    color => Pixel color
void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_COLOR color)
{
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        // Don't write outside the buffer
        return;
    }
    
    // Check if pixel should be inverted
    if(SSD1306.Inverted) {
        color = (OLED_COLOR)!color;
    }
    
    // Draw in the right color
    if(color == White)
    {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
    }
}

// Draw 1 char to the screen buffer
// ch       => char
// Font     => Font
// color    => Black or White
char OLED_WriteChar(char ch, FontDef Font, OLED_COLOR color)
{
    uint32_t i, b, j;
    
    // Check if character is valid
    if (ch < 32 || ch > 126)
    {
        return 0;
    }
    
    // Check remaining space on current line
    if (OLED_WIDTH < (SSD1306.CurrentX + Font.FontWidth) ||
        OLED_HEIGHT < (SSD1306.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }
    
    // Use the font to write
    for(i = 0; i < Font.FontHeight; i++)
    {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for(j = 0; j < Font.FontWidth; j++)
        {
            if((b << j) & 0x8000)
            {
                OLED_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (OLED_COLOR) color);
            }
            else
            {
                OLED_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (OLED_COLOR)!color);
            }
        }
    }
    
    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;
    
    // Return written char for validation
    return ch;
}

// Write full string to screenbuffer
char OLED_WriteString(char* str, FontDef Font, OLED_COLOR color)
{

	// Write until null-byte
    while (*str)
    {
        if (OLED_WriteChar(*str, Font, color) != *str)
        {
            // Char could not be written
            return *str;
        }
        
        // Next char
        str++;
    }
    
    // Everything ok
    return *str;
}

// Position the cursor
void OLED_SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

// Draw line by Bresenhem's algorithm
void OLED_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_COLOR color)
{
	int32_t deltaX = abs(x2 - x1);
	int32_t deltaY = abs(y2 - y1);
	int32_t signX = ((x1 < x2) ? 1 : -1);
	int32_t signY = ((y1 < y2) ? 1 : -1);
	int32_t error = deltaX - deltaY;
	int32_t error2;

	OLED_DrawPixel(x2, y2, color);
	while((x1 != x2) || (y1 != y2))
	{
		OLED_DrawPixel(x1, y1, color);
		error2 = error * 2;

		if(error2 > -deltaY)
		{
			error -= deltaY;
			x1 += signX;
		}

		if(error2 < deltaX)
		{
			error += deltaX;
			y1 += signY;
		}
	}

	return;
}

//Draw polyline
void OLED_Polyline(const OLED_VERTEX *par_vertex, uint16_t par_size, OLED_COLOR color) {
  uint16_t i;
  if (par_vertex != 0)
  {
    for (i = 1; i < par_size; i++)
    {
    	OLED_Line(par_vertex[i - 1].x, par_vertex[i - 1].y, par_vertex[i].x, par_vertex[i].y, color);
    }
  }
  return;
}

// Convert Degrees to Radians
static float OLED_DegToRad(float par_deg)
{
    return par_deg * 3.14 / 180.0;
}

// Normalize degree to [0;360]
static uint16_t OLED_NormalizeTo0_360(uint16_t par_deg)
{
  uint16_t loc_angle;
  if (par_deg <= 360)
  {
    loc_angle = par_deg;
  }
  else
  {
    loc_angle = par_deg % 360;
    loc_angle = ((par_deg != 0)?par_deg:360);
  }
  return loc_angle;
}

/* DrawArc. Draw angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle in degree
 * sweep in degree
 */
void OLED_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, OLED_COLOR color)
{
    #define CIRCLE_APPROXIMATION_SEGMENTS 36

	float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1,xp2;
    uint8_t yp1,yp2;
    uint32_t count = 0;
    uint32_t loc_sweep = 0;
    float rad;
    
    loc_sweep = OLED_NormalizeTo0_360(sweep);
    
    count = (OLED_NormalizeTo0_360(start_angle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;

    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;

    while(count < approx_segments)
    {
        rad = OLED_DegToRad(count*approx_degree);
        xp1 = x + (int8_t)(sin(rad)*radius);
        yp1 = y + (int8_t)(cos(rad)*radius);

        count++;

        if(count != approx_segments)
        {
            rad = OLED_DegToRad(count*approx_degree);
        }
        else
        {            
            rad = OLED_DegToRad(loc_sweep);
        }

        xp2 = x + (int8_t)(sin(rad)*radius);
        yp2 = y + (int8_t)(cos(rad)*radius);

        OLED_Line(xp1,yp1,xp2,yp2,color);
    }
    
    return;
}

// Draw circle by Bresenhem's algorithm
void OLED_DrawCircle(uint8_t par_x,uint8_t par_y,uint8_t par_r,OLED_COLOR par_color)
{
	int32_t x = -par_r;
	int32_t y = 0;
	int32_t err = 2 - 2 * par_r;
	int32_t e2;

	if (par_x >= OLED_WIDTH || par_y >= OLED_HEIGHT)
	{
		return;
	}

	do {
		OLED_DrawPixel(par_x - x, par_y + y, par_color);
		OLED_DrawPixel(par_x + x, par_y + y, par_color);
		OLED_DrawPixel(par_x + x, par_y - y, par_color);
		OLED_DrawPixel(par_x - x, par_y - y, par_color);

		e2 = err;

		if (e2 <= y)
		{
			y++;
			err = err + (y * 2 + 1);

			if (-x == y && e2 <= x)
			{
				e2 = 0;
			}
		}
		if (e2 > x)
		{
			x++;
			err = err + (x * 2 + 1);
		}
	} while (x <= 0);

    return;
}

// Draw rectangle
void OLED_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_COLOR color)
{
	OLED_Line(x1,y1,x2,y1,color);
	OLED_Line(x2,y1,x2,y2,color);
	OLED_Line(x2,y2,x1,y2,color);
	OLED_Line(x1,y2,x1,y1,color);

	return;
}

void OLED_SetContrast(const uint8_t value)
{
    const uint8_t kSetContrastControlRegister = 0x81;

    OLED_WriteCommand(kSetContrastControlRegister);
    OLED_WriteCommand(value);
}

void OLED_SetDisplayOn(const uint8_t on)
{
    uint8_t value;

    if (on)
    {
        value = 0xAF;   // Display on
        SSD1306.DisplayOn = 1;
    } else {
        value = 0xAE;   // Display off
        SSD1306.DisplayOn = 0;
    }

    OLED_WriteCommand(value);
}

uint8_t OLED_GetDisplayOn()
{
    return SSD1306.DisplayOn;
}
