/**
 * @file PCD8544.h
 * @brief This file contains functions declarations for PCD8544 display.
 * @author Sk. Mohammadul Haque, Andre Wussow, Limor Fried (originally)
 * @version 1.0.0.0
 * @copyright
 * Copyright (c) 2010 Limor Fried, Adafruit Industries
 * Copyright (c) 2012 Andre Wussow (Raspberry Pi version)
 * Copyright (c) 2016 Sk. Mohammadul Haque (modified version)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define BLACK 1
#define WHITE 0

#define LCDWIDTH 84
#define LCDHEIGHT 48

#define PCD8544_POWERDOWN 0x04
#define PCD8544_ENTRYMODE 0x02
#define PCD8544_EXTENDEDINSTRUCTION 0x01

#define PCD8544_DISPLAYBLANK 0x0
#define PCD8544_DISPLAYNORMAL 0x4
#define PCD8544_DISPLAYALLON 0x1
#define PCD8544_DISPLAYINVERTED 0x5

#define PCD8544_FUNCTIONSET 0x20
#define PCD8544_DISPLAYCONTROL 0x08
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80

#define PCD8544_SETTEMP 0x04
#define PCD8544_SETBIAS 0x10
#define PCD8544_SETVOP 0x80

#define LCD_BIAS 0x03	// Range: 0-7 (0x00-0x07)
#define LCD_TEMP 0x02	// Range: 0-3 (0x00-0x03)
#define LCD_CONTRAST 0x28	// Range: 0-127 (0x00-0x7F)

#define LCD_POS 0
#define LCD_NEG 1

#define LCD_OFF 0
#define LCD_ON 1

#define CLKCONST 400

#define LSBFIRST 0
#define MSBFIRST 1

extern uint8_t pcd8544_buffer[LCDWIDTH*LCDHEIGHT/8];

void LCDInit(uint8_t SCLK, uint8_t DIN, uint8_t DC, uint8_t CS, uint8_t RST, uint8_t contrast, uint8_t spi_enabled);
void LCDsetPower(uint8_t mode);
void LCDshowLogo();
void LCDdrawbitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color);
void LCDdrawbitframe(const uint8_t *bitframe, uint8_t type);
void LCDdrawstring(uint8_t x, uint8_t line, char *c);
void LCDdrawchar(uint8_t x, uint8_t line, char c);
void LCDwrite(uint8_t c);
void LCDsetDisplayMode(uint8_t mode);
void LCDsetContrast(uint8_t val);
void LCDsetCursor(uint8_t x, uint8_t y);
void LCDsetPosition(uint8_t x, uint8_t y);
void LCDsetTextSize(uint8_t s);
void LCDsetTextColor(uint8_t c);
void LCDsetPixel(uint8_t x, uint8_t y, uint8_t color);
uint8_t LCDgetPixel(uint8_t x, uint8_t y);
void LCDdrawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);
void LCDdrawrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color);
void LCDfillrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t color);
void LCDdrawcircle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t color);
void LCDfillcircle(uint8_t x0, uint8_t y0, uint8_t r,uint8_t color);
void LCDspiwrite(uint8_t c);
void LCDspiwriteArray(uint8_t *c, uint16_t n);
void LCDcommand(uint8_t c);
void LCDcommandArray(uint8_t *c, uint16_t n);
void LCDdata(uint8_t c);
void LCDdataArray(uint8_t *c, uint16_t n);
void LCDzero();
void LCDdisplay();
void LCDupdate();
void LCDclear();
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
void delay(uint32_t msecs);
#ifdef __cplusplus
}
#endif
