/*
        OLED显示模块。

        使用SPI通信时，需要#define OLED_USE_SPI
        使用I2C通信时，需要#define OLED_USE_I2C

*/

/* Includes ----------------------------------------------------------------- */
/* Include 自身的头文件。*/
#include "oled.h"

/* Include 标准库。*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Include HAL相关的头文件。*/
#include <gpio.h>

#include "bsp_delay.h"
#include "bsp_spi.h"

#ifdef OLED_USE_SPI
#include "spi.h"
#

#elif defined OLED_USE_I2C
#include "i2c.h"

#endif

/* Include Component相关的头文件 */
/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
#ifdef OLED_USE_SPI

#define OLED_CMD_Set() \
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)
#define OLED_CMD_Clr() \
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)

#define OLED_RST_Set() \
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET)
#define OLED_RST_Clr() \
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET)

#elif defined OLED_USE_I2C
#define OLED_I2C_HANDLE hi2c1

#endif

/* Private typedef ---------------------------------------------------------- */
typedef enum {
  OLED_WriteCMD = 0,
  OLED_WriteData = 1,
} OLED_Write_t;

/* Private variables -------------------------------------------------------- */
static const uint8_t oled_font[95][8] = {
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, /* " ", 0 */
    {
        0x00,
        0x00,
        0x00,
        0xcf,
        0xcf,
        0x00,
        0x00,
        0x00,
    }, /* "!", 1 */
    {
        0x00,
        0x0c,
        0x06,
        0x00,
        0x0c,
        0x06,
        0x00,
        0x00,
    }, /* """, 2 */
    {
        0x24,
        0xe4,
        0x3c,
        0x27,
        0xe4,
        0x3c,
        0x27,
        0x24,
    }, /* "#", 3 */
    {
        0x00,
        0x20,
        0x46,
        0xf9,
        0x9f,
        0x62,
        0x04,
        0x00,
    }, /* "$", 4 */
    {
        0x06,
        0x09,
        0xc6,
        0x30,
        0x0c,
        0x63,
        0x90,
        0x60,
    }, /* "%", 5 */
    {
        0x00,
        0x00,
        0x6e,
        0x91,
        0xa9,
        0x46,
        0xa0,
        0x00,
    }, /* "&", 6 */
    {
        0x00,
        0x00,
        0x00,
        0x1c,
        0x0e,
        0x00,
        0x00,
        0x00,
    }, /* "'", 7 */
    {
        0x00,
        0x00,
        0x3c,
        0x42,
        0x81,
        0x00,
        0x00,
        0x00,
    }, /* "(", 8 */
    {
        0x00,
        0x00,
        0x00,
        0x81,
        0x42,
        0x3c,
        0x00,
        0x00,
    }, /* ")", 9 */
    {
        0x00,
        0x10,
        0x54,
        0x38,
        0x38,
        0x54,
        0x10,
        0x00,
    }, /* "*", 10 */
    {
        0x00,
        0x10,
        0x10,
        0xfc,
        0x10,
        0x10,
        0x00,
        0x00,
    }, /* "+", 11 */
    {
        0x00,
        0x00,
        0x00,
        0xc0,
        0x60,
        0x00,
        0x00,
        0x00,
    }, /* ",", 12 */
    {
        0x00,
        0x00,
        0x10,
        0x10,
        0x10,
        0x10,
        0x00,
        0x00,
    }, /* "-", 13 */
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0xc0,
        0xc0,
        0x00,
        0x00,
    }, /* ".", 14 */
    {
        0x00,
        0x00,
        0x00,
        0xc0,
        0x38,
        0x07,
        0x00,
        0x00,
    }, /* "/", 15 */
    {
        0x00,
        0x00,
        0x7c,
        0x92,
        0x8a,
        0x7c,
        0x00,
        0x00,
    }, /* "0", 16 */
    {
        0x00,
        0x00,
        0x00,
        0x84,
        0xfe,
        0x80,
        0x00,
        0x00,
    }, /* "1", 17 */
    {
        0x00,
        0x00,
        0x8c,
        0xc2,
        0xa2,
        0x9c,
        0x00,
        0x00,
    }, /* "2", 18 */
    {
        0x00,
        0x00,
        0x44,
        0x92,
        0x92,
        0x6c,
        0x00,
        0x00,
    }, /* "3", 19 */
    {
        0x00,
        0x20,
        0x38,
        0x24,
        0xfe,
        0x20,
        0x00,
        0x00,
    }, /* "4", 20 */
    {
        0x00,
        0x00,
        0x5e,
        0x92,
        0x92,
        0x62,
        0x00,
        0x00,
    }, /* "5", 21 */
    {
        0x00,
        0x00,
        0x78,
        0x94,
        0x92,
        0x62,
        0x00,
        0x00,
    }, /* "6", 22 */
    {
        0x00,
        0x00,
        0x82,
        0x62,
        0x1a,
        0x06,
        0x00,
        0x00,
    }, /* "7", 23 */
    {
        0x00,
        0x00,
        0x6c,
        0x92,
        0x92,
        0x6c,
        0x00,
        0x00,
    }, /* "8", 24 */
    {
        0x00,
        0x00,
        0x8c,
        0x52,
        0x32,
        0x1c,
        0x00,
        0x00,
    }, /* "9", 25 */
    {
        0x00,
        0x00,
        0x00,
        0x6c,
        0x6c,
        0x00,
        0x00,
        0x00,
    }, /* ":", 26 */
    {
        0x00,
        0x00,
        0x80,
        0xec,
        0x6c,
        0x00,
        0x00,
        0x00,
    }, /* ";", 27 */
    {
        0x00,
        0x00,
        0x10,
        0x28,
        0x44,
        0x00,
        0x00,
        0x00,
    }, /* "<", 28 */
    {
        0x00,
        0x00,
        0x24,
        0x24,
        0x24,
        0x24,
        0x00,
        0x00,
    }, /* "=", 29 */
    {
        0x00,
        0x00,
        0x00,
        0x44,
        0x28,
        0x10,
        0x00,
        0x00,
    }, /* ">", 30 */
    {
        0x00,
        0x00,
        0x0c,
        0xa2,
        0x92,
        0x1c,
        0x00,
        0x00,
    }, /* "?", 31 */
    {
        0x00,
        0x3c,
        0x42,
        0x99,
        0xa5,
        0xa2,
        0x3c,
        0x00,
    }, /* "@", 32 */
    {
        0x00,
        0xe0,
        0x1c,
        0x12,
        0x12,
        0x1c,
        0xe0,
        0x00,
    }, /* "A", 33 */
    {
        0x00,
        0xfe,
        0x92,
        0x92,
        0x9c,
        0x90,
        0x60,
        0x00,
    }, /* "B", 34 */
    {
        0x00,
        0x38,
        0x44,
        0x82,
        0x82,
        0x82,
        0x44,
        0x00,
    }, /* "C", 35 */
    {
        0x00,
        0xfe,
        0x82,
        0x82,
        0x82,
        0x82,
        0x7c,
        0x00,
    }, /* "D", 36 */
    {
        0x00,
        0xfe,
        0x92,
        0x92,
        0x92,
        0x92,
        0x92,
        0x00,
    }, /* "E", 37 */
    {
        0x00,
        0xfe,
        0x12,
        0x12,
        0x12,
        0x12,
        0x02,
        0x00,
    }, /* "F", 38 */
    {
        0x00,
        0x7c,
        0x82,
        0x92,
        0x92,
        0x72,
        0x00,
        0x00,
    }, /* "G", 39 */
    {
        0x00,
        0xfe,
        0x10,
        0x10,
        0x10,
        0x10,
        0xfe,
        0x00,
    }, /* "H", 40 */
    {
        0x00,
        0x82,
        0x82,
        0xfe,
        0x82,
        0x82,
        0x00,
        0x00,
    }, /* "I", 41 */
    {
        0x00,
        0x82,
        0x82,
        0x7e,
        0x02,
        0x02,
        0x00,
        0x00,
    }, /* "J", 42 */
    {
        0x00,
        0xfe,
        0x10,
        0x28,
        0x44,
        0x82,
        0x00,
        0x00,
    }, /* "K", 43 */
    {
        0x00,
        0xfe,
        0x80,
        0x80,
        0x80,
        0x80,
        0x00,
        0x00,
    }, /* "L", 44 */
    {
        0xfc,
        0x02,
        0x04,
        0xf8,
        0x04,
        0x02,
        0xfc,
        0x00,
    }, /* "M", 45 */
    {
        0x00,
        0xfe,
        0x04,
        0x18,
        0x30,
        0x40,
        0xfe,
        0x00,
    }, /* "N", 46 */
    {
        0x00,
        0x7c,
        0x82,
        0x82,
        0x82,
        0x82,
        0x7c,
        0x00,
    }, /* "O", 47 */
    {
        0x00,
        0x00,
        0xfe,
        0x12,
        0x12,
        0x0c,
        0x00,
        0x00,
    }, /* "P", 48 */
    {
        0x00,
        0x00,
        0x3c,
        0x42,
        0xc2,
        0xbc,
        0x00,
        0x00,
    }, /* "Q", 49 */
    {
        0x00,
        0x00,
        0xfe,
        0x32,
        0x52,
        0x8c,
        0x00,
        0x00,
    }, /* "R", 50 */
    {
        0x00,
        0x00,
        0x4c,
        0x92,
        0x92,
        0x64,
        0x00,
        0x00,
    }, /* "S", 51 */
    {
        0x00,
        0x02,
        0x02,
        0xfe,
        0x02,
        0x02,
        0x00,
        0x00,
    }, /* "T", 52 */
    {
        0x00,
        0x7e,
        0x80,
        0x80,
        0x80,
        0x80,
        0x7e,
        0x00,
    }, /* "U", 53 */
    {
        0x00,
        0x0c,
        0x30,
        0xc0,
        0x30,
        0x0c,
        0x00,
        0x00,
    }, /* "V", 54 */
    {
        0x7c,
        0x80,
        0x80,
        0x78,
        0x80,
        0x80,
        0x7c,
        0x00,
    }, /* "W", 55 */
    {
        0x00,
        0x84,
        0x48,
        0x30,
        0x30,
        0x48,
        0x84,
        0x00,
    }, /* "X", 56 */
    {
        0x00,
        0x06,
        0x08,
        0xf0,
        0x08,
        0x06,
        0x00,
        0x00,
    }, /* "Y", 57 */
    {
        0x00,
        0x00,
        0xc2,
        0xa2,
        0x92,
        0x8e,
        0x00,
        0x00,
    }, /* "Z", 58 */
    {
        0x00,
        0x00,
        0xfe,
        0x82,
        0x82,
        0x82,
        0x00,
        0x00,
    }, /* "[", 59 */
    {
        0x00,
        0x00,
        0x06,
        0x18,
        0x60,
        0x80,
        0x00,
        0x00,
    }, /* "\", 60 */
    {
        0x00,
        0x00,
        0x82,
        0x82,
        0x82,
        0xfe,
        0x00,
        0x00,
    }, /* "]", 61 */
    {
        0x00,
        0x30,
        0x0c,
        0x02,
        0x0c,
        0x30,
        0x00,
        0x00,
    }, /* "^", 62 */
    {
        0x00,
        0x80,
        0x80,
        0x80,
        0x80,
        0x80,
        0x80,
        0x00,
    }, /* "_", 63 */
    {
        0x00,
        0x00,
        0x04,
        0x0c,
        0x18,
        0x00,
        0x00,
        0x00,
    }, /* "`", 64 */
    {
        0x00,
        0x00,
        0x60,
        0x90,
        0x90,
        0xe0,
        0x00,
        0x00,
    }, /* "a", 65 */
    {
        0x00,
        0x00,
        0xf8,
        0xa0,
        0xe0,
        0x00,
        0x00,
        0x00,
    }, /* "b", 66 */
    {
        0x00,
        0x00,
        0x60,
        0x90,
        0x90,
        0x00,
        0x00,
        0x00,
    }, /* "c", 67 */
    {
        0x00,
        0x00,
        0xe0,
        0xa0,
        0xf8,
        0x00,
        0x00,
        0x00,
    }, /* "d", 68 */
    {
        0x00,
        0x00,
        0x70,
        0xa8,
        0xa8,
        0x90,
        0x00,
        0x00,
    }, /* "e", 69 */
    {
        0x00,
        0x00,
        0x10,
        0xf8,
        0x14,
        0x00,
        0x00,
        0x00,
    }, /* "f", 70 */
    {
        0x00,
        0x00,
        0xd8,
        0xa4,
        0x7c,
        0x00,
        0x00,
        0x00,
    }, /* "g", 71 */
    {
        0x00,
        0x00,
        0xf8,
        0x20,
        0xe0,
        0x00,
        0x00,
        0x00,
    }, /* "h", 72 */
    {
        0x00,
        0x00,
        0x00,
        0xe8,
        0x00,
        0x00,
        0x00,
        0x00,
    }, /* "i", 73 */
    {
        0x00,
        0x00,
        0x40,
        0x90,
        0x74,
        0x00,
        0x00,
        0x00,
    }, /* "j", 74 */
    {
        0x00,
        0x00,
        0xf8,
        0x60,
        0x90,
        0x00,
        0x00,
        0x00,
    }, /* "k", 75 */
    {
        0x00,
        0x00,
        0x78,
        0x80,
        0x80,
        0x00,
        0x00,
        0x00,
    }, /* "l", 76 */
    {
        0x00,
        0xe0,
        0x10,
        0xe0,
        0x10,
        0xe0,
        0x00,
        0x00,
    }, /* "m", 77 */
    {
        0x00,
        0x00,
        0xf0,
        0x10,
        0x10,
        0xe0,
        0x00,
        0x00,
    }, /* "n", 78 */
    {
        0x00,
        0x00,
        0x60,
        0x90,
        0x90,
        0x60,
        0x00,
        0x00,
    }, /* "o", 79 */
    {
        0x00,
        0x00,
        0xf0,
        0x48,
        0x48,
        0x30,
        0x00,
        0x00,
    }, /* "p", 80 */
    {
        0x00,
        0x00,
        0x30,
        0x48,
        0x48,
        0xf0,
        0x00,
        0x00,
    }, /* "q", 81 */
    {
        0x00,
        0x00,
        0x00,
        0xf0,
        0x20,
        0x10,
        0x00,
        0x00,
    }, /* "r", 82 */
    {
        0x00,
        0x00,
        0x90,
        0xa8,
        0xa8,
        0x48,
        0x00,
        0x00,
    }, /* "s", 83 */
    {
        0x00,
        0x10,
        0x10,
        0xf8,
        0x90,
        0x90,
        0x00,
        0x00,
    }, /* "t", 84 */
    {
        0x00,
        0x00,
        0x78,
        0x80,
        0x80,
        0xf8,
        0x00,
        0x00,
    }, /* "u", 85 */
    {
        0x00,
        0x18,
        0x60,
        0x80,
        0x60,
        0x18,
        0x00,
        0x00,
    }, /* "v", 86 */
    {
        0x00,
        0x38,
        0xc0,
        0x38,
        0xc0,
        0x38,
        0x00,
        0x00,
    }, /* "w", 87 */
    {
        0x00,
        0x88,
        0x50,
        0x20,
        0x50,
        0x88,
        0x00,
        0x00,
    }, /* "x", 88 */
    {
        0x00,
        0x8c,
        0x50,
        0x20,
        0x10,
        0x0c,
        0x00,
        0x00,
    }, /* "y", 89 */
    {
        0x00,
        0x88,
        0xc8,
        0xa8,
        0x98,
        0x88,
        0x00,
        0x00,
    }, /* "z", 90 */
    {
        0x00,
        0x00,
        0x10,
        0x7c,
        0x82,
        0x00,
        0x00,
        0x00,
    }, /* "{", 91 */
    {
        0x00,
        0x00,
        0x00,
        0xfe,
        0x00,
        0x00,
        0x00,
        0x00,
    }, /* "|", 92 */
    {
        0x00,
        0x00,
        0x00,
        0x82,
        0x7c,
        0x10,
        0x00,
        0x00,
    }, /* "}", 93 */
    {
        0x00,
        0x08,
        0x04,
        0x04,
        0x08,
        0x10,
        0x10,
        0x08,
    }, /* "~", 94 */
};

static const uint8_t gram_logo[8][128] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06,
    0x0E, 0X1E, 0X3E, 0X7E, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE,
    0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE,
    0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFE,
    0XFE, 0XFE, 0XFE, 0XFE, 0XFE, 0XFC, 0XFC, 0XF8, 0XF8, 0XF0, 0XE0, 0XC0,
    0X80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x03, 0x07, 0x0F, 0X1F, 0X3F, 0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XF7, 0XE7, 0XC7, 0X87, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0F, 0X1F,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFE, 0XF0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0XC0,
    0XFC, 0XFC, 0XFC, 0XFC, 0XFC, 0XFD, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFE, 0XFC, 0XFC, 0XFC, 0XFC,
    0XFC, 0XFC, 0XFC, 0XFC, 0XFC, 0XFC, 0XFE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0X7F, 0X3F, 0x0F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0X80, 0XF0, 0XFE, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X1F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0X1F, 0X3F, 0X7F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XEF, 0XCF, 0X87, 0x07, 0x03, 0x03, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0XE0, 0XFC,
    0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X3F, 0x07,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0X40, 0X40, 0X40, 0XC0, 0XC0, 0XC0,
    0XC0, 0XC1, 0XC3, 0XC7, 0XCF, 0XDF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFE, 0XFC, 0XF8, 0XF0, 0XE0, 0XC0, 0X80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x0C, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x07,
    0x07, 0x0F, 0x0F, 0X1F, 0X1F, 0X3F, 0X3F, 0X7F, 0X7F, 0XFF, 0XF3, 0XE3,
    0XC3, 0X83, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x06,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x60, 0x78, 0x7B, 0x1B, 0x1B, 0x1B, 0x7B, 0x7F,
    0x6F, 0x4E, 0x00, 0x3E, 0x7F, 0x63, 0x63, 0x63, 0x7F, 0x3E, 0x00, 0x78,
    0x7B, 0x6B, 0x6B, 0x6B, 0x6B, 0x7B, 0x7F, 0x36, 0x00, 0x3E, 0x7F, 0x63,
    0x63, 0x63, 0x7F, 0x3E, 0x40, 0x60, 0x78, 0x3D, 0x07, 0x1F, 0x7C, 0x70,
    0x1D, 0x07, 0x7F, 0x78, 0x40, 0x00, 0x60, 0x70, 0x38, 0x5D, 0x6F, 0x67,
    0x6F, 0x7C, 0x70, 0x40, 0x00, 0x40, 0x66, 0x6F, 0x6B, 0x6B, 0x6B, 0x6B,
    0x7B, 0x31, 0x02, 0x03, 0x03, 0x7F, 0x7F, 0x03, 0x03, 0x61, 0x68, 0x6B,
    0x6B, 0x6B, 0x6B, 0x6B, 0x0B, 0x03, 0x60, 0x78, 0x7B, 0x1B, 0x1B, 0x1B,
    0x7B, 0x7B, 0x6F, 0x4E, 0x00, 0x46, 0x6F, 0x6F, 0x6B, 0x6B, 0x6B, 0x7B,
    0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static OLED_t *goled;
static bool inited = false;

/* Private function ---------------------------------------------- */
static void OLED_WriteByte(uint8_t data, OLED_Write_t type) {
#ifdef OLED_USE_SPI
  switch (type) {
    case OLED_WriteCMD:
      OLED_CMD_Clr();
      break;

    case OLED_WriteData:
      OLED_CMD_Set();
      break;
  }
  BSP_SPI_Transmit(BSP_SPI_OLED, &data, 1);

#elif defined OLED_USE_I2C
  uint8_t cmd_data[2];

  switch (type) {
    case OLED_WriteCMD:
      cmd_data[0] = 0x00;
      break;

    case OLED_WriteData:
      cmd_data[0] = 0x40;
      break;
  }
  cmd_data[1] = data;
  HAL_I2C_Master_Transmit_DMA(OLED_I2C, OLED_I2C_ADDRESS, cmd_data, 2);
#endif

  BSP_Delay(1);
}

static void OLED_WriteSequenceData(uint8_t *data, uint16_t len) {
#ifdef OLED_USE_SPI
  OLED_CMD_Set();
  HAL_SPI_Transmit_DMA(&hspi1, data, len);

#elif defined OLED_USE_I2C
  /* TODO */
  uint8_t cmd_data[2];
  cmd_data[0] = 0x40;
  cmd_data[1] = data;
  HAL_I2C_Master_Transmit_DMA(OLED_I2C, OLED_I2C_ADDRESS, cmd_data, 2);
#endif
}

/* Exported functions ------------------------------------------------------- */
int8_t OLED_Init(OLED_t *oled) {
  ASSERT(oled);

  if (inited) return -1;

  oled->cursor.column = 0;
  oled->cursor.page = 0;
  oled->modified = true;

  goled = oled;

  OLED_RST_Clr();
  BSP_Delay(1);
  OLED_RST_Set();
  BSP_Delay(5);

  OLED_WriteByte(0xae, OLED_WriteCMD); /* Dot martix display off. */
  OLED_WriteByte(0x00, OLED_WriteCMD); /* Set lower column address:(00H~0FH). */
  OLED_WriteByte(0x10,
                 OLED_WriteCMD); /* Set higher column address:(10H~1FH). */
  OLED_WriteByte(0x20, OLED_WriteCMD); /* Set Memory Addressing Mode */
  OLED_WriteByte(
      0x10,
      OLED_WriteCMD); /* 00,Horizontal Addressing Mode;01,Vertical Addressing
                         Mode;10,Page Addressing Mode (RESET);11,Invalid */
  OLED_WriteByte(0x32, OLED_WriteCMD); /* Set pump voltage 8v. */
  OLED_WriteByte(0x40, OLED_WriteCMD); /* Set display start line:(40H~7FH). */
  OLED_WriteByte(0x81, OLED_WriteCMD); /* Contarst control. */
  OLED_WriteByte(0x6f, OLED_WriteCMD); /* 		Contarst: 0x00~0xff. */
  OLED_WriteByte(0xa1, OLED_WriteCMD); /* Set segment re-map. 0xa0 or 0xa1. */
  OLED_WriteByte(0xa4, OLED_WriteCMD); /* Entire display off. */
  OLED_WriteByte(0xa6, OLED_WriteCMD); /* Set normal display. */
  OLED_WriteByte(0xa8, OLED_WriteCMD); /* Set multiplex ratio. */
  OLED_WriteByte(0x3f, OLED_WriteCMD);
  OLED_WriteByte(0xad, OLED_WriteCMD); /* Set DC/DC booster. */
  OLED_WriteByte(0x8b, OLED_WriteCMD);
  OLED_WriteByte(0xb0, OLED_WriteCMD); /* Set page address. */
  OLED_WriteByte(0xc8, OLED_WriteCMD); /* Com scan COM0->COM63. */
  OLED_WriteByte(0xd3, OLED_WriteCMD); /* Set display offset. */
  OLED_WriteByte(0x00, OLED_WriteCMD);
  OLED_WriteByte(0xd5, OLED_WriteCMD); /* Set frame frequency. */
  OLED_WriteByte(0xf0, OLED_WriteCMD);
  OLED_WriteByte(0xd9, OLED_WriteCMD); /* Set pre_charge period. */
  OLED_WriteByte(0x22, OLED_WriteCMD);
  OLED_WriteByte(0xda, OLED_WriteCMD); /* Com pin configuration. */
  OLED_WriteByte(0x12, OLED_WriteCMD);
  OLED_WriteByte(0xdb, OLED_WriteCMD); /* Set vcom deselect level. */
  OLED_WriteByte(0x20, OLED_WriteCMD);
  OLED_WriteByte(0x8d, OLED_WriteCMD); /* Set DC-DC enable */
  OLED_WriteByte(0x14, OLED_WriteCMD);
  OLED_WriteByte(0xaf, OLED_WriteCMD); /* Display on. */

  memcmp(oled->gram, gram_logo, sizeof(oled->gram));

  OLED_SetAllRam(oled, OLED_PEN_WRITE); /* Clear memory. */

  OLED_Refresh(oled); /* Display initial picture. */
  return 0;
}

OLED_t *OLED_GetDevice(void) {
  if (inited) {
    return goled;
  }
  return NULL;
}

int8_t OLED_PrintRam(OLED_t *oled, const char *str) {
  ASSERT(oled);
  ASSERT(str);

  oled->modified = true;

  for (uint16_t i = 0; str[i] != '\0'; i++) {
    uint8_t c = str[i];
    if (c < 32) {
      switch (c) {
        case '\n':
          oled->cursor.page++;
          oled->cursor.column = 0;
          break;

        case '\t':
          oled->cursor.column += 16;
          break;
      }
    } else if (c < 126) {
      if (oled->cursor.column == 128) {
        oled->cursor.page++;
        oled->cursor.column = 0;
      }
      if (oled->cursor.page == 8) {
        OLED_SetAllRam(oled, OLED_PEN_CLEAR);
        oled->cursor.page = 0;
      }
      memcpy(&oled->gram[oled->cursor.page][oled->cursor.column],
             oled_font[c - 32], sizeof(oled_font[0]));
      oled->cursor.column += 8;
    }
  }
  return 0;
}

uint8_t OLED_RewindRam(OLED_t *oled) {
  oled->modified = true;

  OLED_SetAllRam(oled, OLED_PEN_CLEAR);

  oled->cursor.page = 0;
  oled->cursor.column = 0;
  return 0;
}

uint8_t OLED_SetAllRam(OLED_t *oled, OLED_Pen_t pen) {
  switch (pen) {
    case OLED_PEN_WRITE:
      memset(oled->gram, -1, sizeof(oled->gram));
      break;

    case OLED_PEN_CLEAR:
      memset(oled->gram, 0, sizeof(oled->gram));
      break;

    case OLED_PEN_INVERSION:
      for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t n = 0; n < 128; n++) {
          oled->gram[i][n] = ~oled->gram[i][n];
        }
      }
      break;
  }
  return 0;
}

uint8_t OLED_DisplayOn(OLED_t *oled) {
  OLED_WriteByte(0x8d, OLED_WriteCMD);
  OLED_WriteByte(0x14, OLED_WriteCMD);
  OLED_WriteByte(0xaf, OLED_WriteCMD);
  return 0;
}

uint8_t OLED_DisplayOff(OLED_t *oled) {
  OLED_WriteByte(0x8d, OLED_WriteCMD);
  OLED_WriteByte(0x10, OLED_WriteCMD);
  OLED_WriteByte(0xae, OLED_WriteCMD);
  return 0;
}

void OLED_SetCursor(uint8_t x, uint8_t y) {
  OLED_WriteByte((0xb0 + y), OLED_WriteCMD); /*set page address y */
  OLED_WriteByte(((x & 0xf0) >> 4) | 0x10,
                 OLED_WriteCMD);             /*set column high address */
  OLED_WriteByte((x & 0x0f), OLED_WriteCMD); /*set column low address */
}

uint8_t OLED_Refresh(OLED_t *oled) {
  for (uint8_t i = 0; i < 8; i++) {
    OLED_SetCursor(0, i);
    for (uint8_t n = 0; n < 8; n++) {
      OLED_WriteByte(oled->gram[i][n], OLED_WriteData);
    }
  }
  oled->modified = false;

  return 0;
}
