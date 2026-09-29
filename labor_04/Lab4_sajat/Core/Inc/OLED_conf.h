#ifndef __OLED_CONF_H__
#define __OLED_CONF_H__

// I2C Configuration
#define OLED_I2C_PORT               hi2c2
#define OLED_I2C_ADDR               (0x3C << 1)

// OLED height and width in pixel
#define OLED_HEIGHT                 64
#define OLED_WIDTH                  128

// Calculated buffer size in byte
#define OLED_BUFFER_SIZE            OLED_WIDTH * OLED_HEIGHT / 8

// Mirror the screen if needed
//#define OLED_MIRROR_VERT
//#define OLED_MIRROR_HORIZ

// Set inverse color if needed
//#define OLED_INVERSE_COLOR

// Include only needed fonts
#define OLED_INCLUDE_FONT_6x8
#define OLED_INCLUDE_FONT_7x10
#define OLED_INCLUDE_FONT_11x18
#define OLED_INCLUDE_FONT_16x26

#endif /* __OLED_CONF_H__ */
