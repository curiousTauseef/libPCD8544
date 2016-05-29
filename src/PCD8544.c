/**
 * @file PCD8544.c
 * @brief This file contains functions definitions for PCD8544 display.
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

#include <inttypes.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <bitBang.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "../include/PCD8544.h"

/** \cond HIDDEN_SYMBOLS */
#define abs(a) (((a)<0)?-(a):(a))
#define swap(a, b) {uint8_t t = a; a = b; b = t;}
#define _BV(bit) (0x1<<(bit))

static uint8_t cursor_x, cursor_y, textsize, textcolor;
static int8_t _din, _sclk, _dc, _rst, _cs, _spi_enabled;

/** \endcond */

static unsigned char font[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
    0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
    0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
    0x18, 0x3C, 0x7E, 0x3C, 0x18,
    0x1C, 0x57, 0x7D, 0x57, 0x1C,
    0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
    0x00, 0x18, 0x3C, 0x18, 0x00,
    0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
    0x00, 0x18, 0x24, 0x18, 0x00,
    0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
    0x30, 0x48, 0x3A, 0x06, 0x0E,
    0x26, 0x29, 0x79, 0x29, 0x26,
    0x40, 0x7F, 0x05, 0x05, 0x07,
    0x40, 0x7F, 0x05, 0x25, 0x3F,
    0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
    0x7F, 0x3E, 0x1C, 0x1C, 0x08,
    0x08, 0x1C, 0x1C, 0x3E, 0x7F,
    0x14, 0x22, 0x7F, 0x22, 0x14,
    0x5F, 0x5F, 0x00, 0x5F, 0x5F,
    0x06, 0x09, 0x7F, 0x01, 0x7F,
    0x00, 0x66, 0x89, 0x95, 0x6A,
    0x60, 0x60, 0x60, 0x60, 0x60,
    0x94, 0xA2, 0xFF, 0xA2, 0x94,
    0x08, 0x04, 0x7E, 0x04, 0x08,
    0x10, 0x20, 0x7E, 0x20, 0x10,
    0x08, 0x08, 0x2A, 0x1C, 0x08,
    0x08, 0x1C, 0x2A, 0x08, 0x08,
    0x1E, 0x10, 0x10, 0x10, 0x10,
    0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
    0x30, 0x38, 0x3E, 0x38, 0x30,
    0x06, 0x0E, 0x3E, 0x0E, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x5F, 0x00, 0x00,
    0x00, 0x07, 0x00, 0x07, 0x00,
    0x14, 0x7F, 0x14, 0x7F, 0x14,
    0x24, 0x2A, 0x7F, 0x2A, 0x12,
    0x23, 0x13, 0x08, 0x64, 0x62,
    0x36, 0x49, 0x56, 0x20, 0x50,
    0x00, 0x08, 0x07, 0x03, 0x00,
    0x00, 0x1C, 0x22, 0x41, 0x00,
    0x00, 0x41, 0x22, 0x1C, 0x00,
    0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
    0x08, 0x08, 0x3E, 0x08, 0x08,
    0x00, 0x80, 0x70, 0x30, 0x00,
    0x08, 0x08, 0x08, 0x08, 0x08,
    0x00, 0x00, 0x60, 0x60, 0x00,
    0x20, 0x10, 0x08, 0x04, 0x02,
    0x3E, 0x51, 0x49, 0x45, 0x3E,
    0x00, 0x42, 0x7F, 0x40, 0x00,
    0x72, 0x49, 0x49, 0x49, 0x46,
    0x21, 0x41, 0x49, 0x4D, 0x33,
    0x18, 0x14, 0x12, 0x7F, 0x10,
    0x27, 0x45, 0x45, 0x45, 0x39,
    0x3C, 0x4A, 0x49, 0x49, 0x31,
    0x41, 0x21, 0x11, 0x09, 0x07,
    0x36, 0x49, 0x49, 0x49, 0x36,
    0x46, 0x49, 0x49, 0x29, 0x1E,
    0x00, 0x00, 0x14, 0x00, 0x00,
    0x00, 0x40, 0x34, 0x00, 0x00,
    0x00, 0x08, 0x14, 0x22, 0x41,
    0x14, 0x14, 0x14, 0x14, 0x14,
    0x00, 0x41, 0x22, 0x14, 0x08,
    0x02, 0x01, 0x59, 0x09, 0x06,
    0x3E, 0x41, 0x5D, 0x59, 0x4E,
    0x7C, 0x12, 0x11, 0x12, 0x7C,
    0x7F, 0x49, 0x49, 0x49, 0x36,
    0x3E, 0x41, 0x41, 0x41, 0x22,
    0x7F, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x49, 0x49, 0x49, 0x41,
    0x7F, 0x09, 0x09, 0x09, 0x01,
    0x3E, 0x41, 0x41, 0x51, 0x73,
    0x7F, 0x08, 0x08, 0x08, 0x7F,
    0x00, 0x41, 0x7F, 0x41, 0x00,
    0x20, 0x40, 0x41, 0x3F, 0x01,
    0x7F, 0x08, 0x14, 0x22, 0x41,
    0x7F, 0x40, 0x40, 0x40, 0x40,
    0x7F, 0x02, 0x1C, 0x02, 0x7F,
    0x7F, 0x04, 0x08, 0x10, 0x7F,
    0x3E, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x09, 0x09, 0x09, 0x06,
    0x3E, 0x41, 0x51, 0x21, 0x5E,
    0x7F, 0x09, 0x19, 0x29, 0x46,
    0x26, 0x49, 0x49, 0x49, 0x32,
    0x03, 0x01, 0x7F, 0x01, 0x03,
    0x3F, 0x40, 0x40, 0x40, 0x3F,
    0x1F, 0x20, 0x40, 0x20, 0x1F,
    0x3F, 0x40, 0x38, 0x40, 0x3F,
    0x63, 0x14, 0x08, 0x14, 0x63,
    0x03, 0x04, 0x78, 0x04, 0x03,
    0x61, 0x59, 0x49, 0x4D, 0x43,
    0x00, 0x7F, 0x41, 0x41, 0x41,
    0x02, 0x04, 0x08, 0x10, 0x20,
    0x00, 0x41, 0x41, 0x41, 0x7F,
    0x04, 0x02, 0x01, 0x02, 0x04,
    0x40, 0x40, 0x40, 0x40, 0x40,
    0x00, 0x03, 0x07, 0x08, 0x00,
    0x20, 0x54, 0x54, 0x78, 0x40,
    0x7F, 0x28, 0x44, 0x44, 0x38,
    0x38, 0x44, 0x44, 0x44, 0x28,
    0x38, 0x44, 0x44, 0x28, 0x7F,
    0x38, 0x54, 0x54, 0x54, 0x18,
    0x00, 0x08, 0x7E, 0x09, 0x02,
    0x18, 0xA4, 0xA4, 0x9C, 0x78,
    0x7F, 0x08, 0x04, 0x04, 0x78,
    0x00, 0x44, 0x7D, 0x40, 0x00,
    0x20, 0x40, 0x40, 0x3D, 0x00,
    0x7F, 0x10, 0x28, 0x44, 0x00,
    0x00, 0x41, 0x7F, 0x40, 0x00,
    0x7C, 0x04, 0x78, 0x04, 0x78,
    0x7C, 0x08, 0x04, 0x04, 0x78,
    0x38, 0x44, 0x44, 0x44, 0x38,
    0xFC, 0x18, 0x24, 0x24, 0x18,
    0x18, 0x24, 0x24, 0x18, 0xFC,
    0x7C, 0x08, 0x04, 0x04, 0x08,
    0x48, 0x54, 0x54, 0x54, 0x24,
    0x04, 0x04, 0x3F, 0x44, 0x24,
    0x3C, 0x40, 0x40, 0x20, 0x7C,
    0x1C, 0x20, 0x40, 0x20, 0x1C,
    0x3C, 0x40, 0x30, 0x40, 0x3C,
    0x44, 0x28, 0x10, 0x28, 0x44,
    0x4C, 0x90, 0x90, 0x90, 0x7C,
    0x44, 0x64, 0x54, 0x4C, 0x44,
    0x00, 0x08, 0x36, 0x41, 0x00,
    0x00, 0x00, 0x77, 0x00, 0x00,
    0x00, 0x41, 0x36, 0x08, 0x00,
    0x02, 0x01, 0x02, 0x04, 0x02,
    0x3C, 0x26, 0x23, 0x26, 0x3C,
    0x1E, 0xA1, 0xA1, 0x61, 0x12,
    0x3A, 0x40, 0x40, 0x20, 0x7A,
    0x38, 0x54, 0x54, 0x55, 0x59,
    0x21, 0x55, 0x55, 0x79, 0x41,
    0x21, 0x54, 0x54, 0x78, 0x41,
    0x21, 0x55, 0x54, 0x78, 0x40,
    0x20, 0x54, 0x55, 0x79, 0x40,
    0x0C, 0x1E, 0x52, 0x72, 0x12,
    0x39, 0x55, 0x55, 0x55, 0x59,
    0x39, 0x54, 0x54, 0x54, 0x59,
    0x39, 0x55, 0x54, 0x54, 0x58,
    0x00, 0x00, 0x45, 0x7C, 0x41,
    0x00, 0x02, 0x45, 0x7D, 0x42,
    0x00, 0x01, 0x45, 0x7C, 0x40,
    0xF0, 0x29, 0x24, 0x29, 0xF0,
    0xF0, 0x28, 0x25, 0x28, 0xF0,
    0x7C, 0x54, 0x55, 0x45, 0x00,
    0x20, 0x54, 0x54, 0x7C, 0x54,
    0x7C, 0x0A, 0x09, 0x7F, 0x49,
    0x32, 0x49, 0x49, 0x49, 0x32,
    0x32, 0x48, 0x48, 0x48, 0x32,
    0x32, 0x4A, 0x48, 0x48, 0x30,
    0x3A, 0x41, 0x41, 0x21, 0x7A,
    0x3A, 0x42, 0x40, 0x20, 0x78,
    0x00, 0x9D, 0xA0, 0xA0, 0x7D,
    0x39, 0x44, 0x44, 0x44, 0x39,
    0x3D, 0x40, 0x40, 0x40, 0x3D,
    0x3C, 0x24, 0xFF, 0x24, 0x24,
    0x48, 0x7E, 0x49, 0x43, 0x66,
    0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
    0xFF, 0x09, 0x29, 0xF6, 0x20,
    0xC0, 0x88, 0x7E, 0x09, 0x03,
    0x20, 0x54, 0x54, 0x79, 0x41,
    0x00, 0x00, 0x44, 0x7D, 0x41,
    0x30, 0x48, 0x48, 0x4A, 0x32,
    0x38, 0x40, 0x40, 0x22, 0x7A,
    0x00, 0x7A, 0x0A, 0x0A, 0x72,
    0x7D, 0x0D, 0x19, 0x31, 0x7D,
    0x26, 0x29, 0x29, 0x2F, 0x28,
    0x26, 0x29, 0x29, 0x29, 0x26,
    0x30, 0x48, 0x4D, 0x40, 0x20,
    0x38, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x38,
    0x2F, 0x10, 0xC8, 0xAC, 0xBA,
    0x2F, 0x10, 0x28, 0x34, 0xFA,
    0x00, 0x00, 0x7B, 0x00, 0x00,
    0x08, 0x14, 0x2A, 0x14, 0x22,
    0x22, 0x14, 0x2A, 0x14, 0x08,
    0xAA, 0x00, 0x55, 0x00, 0xAA,
    0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x00, 0x00, 0x00, 0xFF, 0x00,
    0x10, 0x10, 0x10, 0xFF, 0x00,
    0x14, 0x14, 0x14, 0xFF, 0x00,
    0x10, 0x10, 0xFF, 0x00, 0xFF,
    0x10, 0x10, 0xF0, 0x10, 0xF0,
    0x14, 0x14, 0x14, 0xFC, 0x00,
    0x14, 0x14, 0xF7, 0x00, 0xFF,
    0x00, 0x00, 0xFF, 0x00, 0xFF,
    0x14, 0x14, 0xF4, 0x04, 0xFC,
    0x14, 0x14, 0x17, 0x10, 0x1F,
    0x10, 0x10, 0x1F, 0x10, 0x1F,
    0x14, 0x14, 0x14, 0x1F, 0x00,
    0x10, 0x10, 0x10, 0xF0, 0x00,
    0x00, 0x00, 0x00, 0x1F, 0x10,
    0x10, 0x10, 0x10, 0x1F, 0x10,
    0x10, 0x10, 0x10, 0xF0, 0x10,
    0x00, 0x00, 0x00, 0xFF, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0xFF, 0x10,
    0x00, 0x00, 0x00, 0xFF, 0x14,
    0x00, 0x00, 0xFF, 0x00, 0xFF,
    0x00, 0x00, 0x1F, 0x10, 0x17,
    0x00, 0x00, 0xFC, 0x04, 0xF4,
    0x14, 0x14, 0x17, 0x10, 0x17,
    0x14, 0x14, 0xF4, 0x04, 0xF4,
    0x00, 0x00, 0xFF, 0x00, 0xF7,
    0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0xF7, 0x00, 0xF7,
    0x14, 0x14, 0x14, 0x17, 0x14,
    0x10, 0x10, 0x1F, 0x10, 0x1F,
    0x14, 0x14, 0x14, 0xF4, 0x14,
    0x10, 0x10, 0xF0, 0x10, 0xF0,
    0x00, 0x00, 0x1F, 0x10, 0x1F,
    0x00, 0x00, 0x00, 0x1F, 0x14,
    0x00, 0x00, 0x00, 0xFC, 0x14,
    0x00, 0x00, 0xF0, 0x10, 0xF0,
    0x10, 0x10, 0xFF, 0x10, 0xFF,
    0x14, 0x14, 0x14, 0xFF, 0x14,
    0x10, 0x10, 0x10, 0x1F, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x10,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xFF, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFF, 0xFF,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x38, 0x44, 0x44, 0x38, 0x44,
    0x7C, 0x2A, 0x2A, 0x3E, 0x14,
    0x7E, 0x02, 0x02, 0x06, 0x06,
    0x02, 0x7E, 0x02, 0x7E, 0x02,
    0x63, 0x55, 0x49, 0x41, 0x63,
    0x38, 0x44, 0x44, 0x3C, 0x04,
    0x40, 0x7E, 0x20, 0x1E, 0x20,
    0x06, 0x02, 0x7E, 0x02, 0x02,
    0x99, 0xA5, 0xE7, 0xA5, 0x99,
    0x1C, 0x2A, 0x49, 0x2A, 0x1C,
    0x4C, 0x72, 0x01, 0x72, 0x4C,
    0x30, 0x4A, 0x4D, 0x4D, 0x30,
    0x30, 0x48, 0x78, 0x48, 0x30,
    0xBC, 0x62, 0x5A, 0x46, 0x3D,
    0x3E, 0x49, 0x49, 0x49, 0x00,
    0x7E, 0x01, 0x01, 0x01, 0x7E,
    0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
    0x44, 0x44, 0x5F, 0x44, 0x44,
    0x40, 0x51, 0x4A, 0x44, 0x40,
    0x40, 0x44, 0x4A, 0x51, 0x40,
    0x00, 0x00, 0xFF, 0x01, 0x03,
    0xE0, 0x80, 0xFF, 0x00, 0x00,
    0x08, 0x08, 0x6B, 0x6B, 0x08,
    0x36, 0x12, 0x36, 0x24, 0x36,
    0x06, 0x0F, 0x09, 0x0F, 0x06,
    0x00, 0x00, 0x18, 0x18, 0x00,
    0x00, 0x00, 0x10, 0x10, 0x00,
    0x30, 0x40, 0xFF, 0x01, 0x01,
    0x00, 0x1F, 0x01, 0x01, 0x1E,
    0x00, 0x19, 0x1D, 0x17, 0x12,
    0x00, 0x3C, 0x3C, 0x3C, 0x3C,
    0x00, 0x00, 0x00, 0x00, 0x00,
};


uint8_t pcd8544_buffer[LCDWIDTH*LCDHEIGHT/8] = {0,}; /**< PCD8544 drawing buffer */

const uint8_t pi_logo [] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0010 (16) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xF8, 0xFC, 0xAE, 0x0E, 0x0E, 0x06, 0x0E, 0x06,   // 0x0020 (32) pixels
    0xCE, 0x86, 0x8E, 0x0E, 0x0E, 0x1C, 0xB8, 0xF0, 0xF8, 0x78, 0x38, 0x1E, 0x0E, 0x8E, 0x8E, 0xC6,   // 0x0030 (48) pixels
    0x0E, 0x06, 0x0E, 0x06, 0x0E, 0x9E, 0xFE, 0xFC, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0040 (64) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0050 (80) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0060 (96) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0F, 0x0F, 0xFE,   // 0x0070 (112) pixels
    0xF8, 0xF0, 0x60, 0x60, 0xE0, 0xE1, 0xE3, 0xF7, 0x7E, 0x3E, 0x1E, 0x1F, 0x1F, 0x1F, 0x3E, 0x7E,   // 0x0080 (128) pixels
    0xFB, 0xF3, 0xE1, 0xE0, 0x60, 0x70, 0xF0, 0xF8, 0xBE, 0x1F, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x00,   // 0x0090 (144) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x00A0 (160) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x00B0 (176) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0,   // 0x00C0 (192) pixels
    0xE0, 0xFC, 0xFE, 0xFF, 0xF3, 0x38, 0x38, 0x0C, 0x0E, 0x0F, 0x0F, 0x0F, 0x0E, 0x3C, 0x38, 0xF8,   // 0x00D0 (208) pixels
    0xF8, 0x38, 0x3C, 0x0E, 0x0F, 0x0F, 0x0F, 0x0E, 0x0C, 0x38, 0x38, 0xF3, 0xFF, 0xFF, 0xF8, 0xE0,   // 0x00E0 (224) pixels
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x00F0 (240) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0100 (256) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0110 (272) pixels
    0x00, 0x7F, 0xFF, 0xE7, 0xC3, 0xC1, 0xE0, 0xFF, 0xFF, 0x78, 0xE0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE0,   // 0x0120 (288) pixels
    0x60, 0x78, 0x38, 0x3F, 0x3F, 0x38, 0x38, 0x60, 0x60, 0xC0, 0xC0, 0xC0, 0xC0, 0xE0, 0xF8, 0x7F,   // 0x0130 (304) pixels
    0xFF, 0xE0, 0xC1, 0xC3, 0xE7, 0x7F, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0140 (320) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0150 (336) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0160 (352) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0F, 0x7F, 0xFF, 0xF1, 0xE0, 0xC0, 0x80, 0x01,   // 0x0170 (368) pixels
    0x03, 0x9F, 0xFF, 0xF0, 0xE0, 0xE0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE0, 0xE0, 0xF0, 0xFF, 0x9F,   // 0x0180 (384) pixels
    0x03, 0x01, 0x80, 0xC0, 0xE0, 0xF1, 0x7F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0190 (400) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x01A0 (416) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x01B0 (432) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,   // 0x01C0 (448) pixels
    0x03, 0x03, 0x07, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x3B, 0x71, 0x60, 0x60, 0x60, 0x60, 0x60, 0x71,   // 0x01D0 (464) pixels
    0x3B, 0x1F, 0x0F, 0x0F, 0x0F, 0x07, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x01E0 (480) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x01F0 (496) pixels
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/** \cond HIDDEN_SYMBOLS */

static void __setpixel(uint8_t x, uint8_t y, uint8_t color)
{
    if((x>=LCDWIDTH)||(y>=LCDHEIGHT)) return;
    if(color) pcd8544_buffer[x+(y/8)*LCDWIDTH] |= _BV(y%8);
    else pcd8544_buffer[x+(y/8)*LCDWIDTH] &= ~_BV(y%8);
}

static uint8_t xUpdateMin = (LCDWIDTH-1), xUpdateMax = 0, yUpdateMin = (LCDHEIGHT-1), yUpdateMax = 0;

static void updateBoundingBox(uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax)
{
    if(xmin<xUpdateMin) xUpdateMin = xmin;
    if(xmax>xUpdateMax) xUpdateMax = xmax;
    if(ymin<yUpdateMin) yUpdateMin = ymin;
    if(ymax>yUpdateMax) yUpdateMax = ymax;
}

static int idx;

/** \endcond */

/** \brief Initializes the LCD Module
 *
 * \param[in] SCLK uint8_t Clock
 * \param[in] DIN uint8_t Data in
 * \param[in] DC uint8_t Data/Command
 * \param[in] CS uint8_t Chip Select
 * \param[in] RST uint8_t Reset
 * \param[in] contrast uint8_t LCD contrast value (0-127)
 * \param[in] spi_enabled uint8_t Enable SPI (0/1)
 *
 */

void LCDInit(uint8_t SCLK, uint8_t DIN, uint8_t DC, uint8_t CS, uint8_t RST, uint8_t contrast, uint8_t spi_enabled)
{
    _din = DIN;
    _sclk = SCLK;
    _dc = DC;
    _rst = RST;
    _cs = CS;
    cursor_x = cursor_y = 0;
    textsize = 1;
    textcolor = BLACK;
    if(wiringPiSetup()<0) printf("wiringPi Setup failed.\n");
    pinMode(_dc, OUTPUT);
    pinMode(_rst, OUTPUT);
    if(spi_enabled)
    {
        _spi_enabled = spi_enabled;
        if(wiringPiSPISetup(0, 2000000)<0) printf("SPI Setup failed.\n");
    }
    else idx = setupBitBang(_cs, _din, _sclk, 0);
    delay(1);
    digitalWrite(_rst, LOW);
    delay(500);
    digitalWrite(_rst, HIGH);

    LCDcommand(PCD8544_FUNCTIONSET|PCD8544_EXTENDEDINSTRUCTION);
    LCDcommand(PCD8544_SETBIAS|LCD_BIAS);
    if(contrast>0x7f) contrast = 0x7f;
    LCDcommand(PCD8544_SETVOP|contrast);
    //LCDcommand(PCD8544_SETTEMP|LCD_TEMP);
    LCDcommand(PCD8544_SETBIAS|LCD_BIAS);
    LCDcommand(PCD8544_FUNCTIONSET);
    LCDcommand(PCD8544_DISPLAYCONTROL|PCD8544_DISPLAYNORMAL);
    LCDclear();
    updateBoundingBox(0, 0, LCDWIDTH-1, LCDHEIGHT-1);
}

/** \brief Sets LCD power mode
 *
 * \param mode uint8_t LCD power mode (LCD_ON/LCD_OFF)
 *
 */

void LCDsetPower(uint8_t mode)
{
    LCDcommand((mode==LCD_ON)?0x20:0x24);
}

/** \brief Displays the Raspberry Pi logo
 *
 *
 */

void LCDshowLogo()
{
    uint16_t i;
    for(i=0; i<LCDWIDTH*LCDHEIGHT/8; i+=4)
    {
        pcd8544_buffer[i] = pi_logo[i];
        pcd8544_buffer[i+1] = pi_logo[i+1];
        pcd8544_buffer[i+2] = pi_logo[i+2];
        pcd8544_buffer[i+3] = pi_logo[i+3];
    }
    LCDdisplay();
}

/** \brief Draws a bitmap with WHITE/BLACK
 *
 * \param[in] x uint8_t Horizontal position
 * \param[in] y uint8_t Vertical position
 * \param[in] bitmap uint8_t* Raw bitmap
 * \param[in] w uint8_t Bitmap width
 * \param[in] h uint8_t Bitmap height
 * \param[in] color uint8_t WHITE/BLACK
 *
 */

void LCDdrawbitmap(uint8_t x, uint8_t y,const uint8_t *bitmap, uint8_t w, uint8_t h,uint8_t color)
{
    uint8_t j,i;
    for(j=0; j<h; ++j)
    {
        for(i=0; i<w; ++i)
        {
            if(*(bitmap+i+(j/8)*w)&_BV(j%8))
            {
                __setpixel(x+i, y+j, color);
            }
        }
    }
    updateBoundingBox(x, y, x+w, y+h);
}

/** \brief Draws a full bit-frame
 *
 * \param[in] bitframe uint8_t* Raw bit-frame
 * \param[in] type uint8_t Positive/ Negative (LCD_POS/LCD_NEG)
 *
 */

void LCDdrawbitframe(const uint8_t *bitframe, uint8_t type)
{
    uint16_t i;
    if(type==LCD_NEG)
    {
        for(i=0; i<LCDWIDTH*LCDHEIGHT/8; i+=4)
        {
            pcd8544_buffer[i] = ~bitframe[i];
            pcd8544_buffer[i+1] = ~bitframe[i+1];
            pcd8544_buffer[i+2] = ~bitframe[i+2];
            pcd8544_buffer[i+3] = ~bitframe[i+3];
        }
    }
    else
    {
        for(i=0; i<LCDWIDTH*LCDHEIGHT/8; i+=4)
        {
            pcd8544_buffer[i] = bitframe[i];
            pcd8544_buffer[i+1] = bitframe[i+1];
            pcd8544_buffer[i+2] = bitframe[i+2];
            pcd8544_buffer[i+3] = bitframe[i+3];
        }
    }
    updateBoundingBox(0, 0, (LCDWIDTH-1), (LCDHEIGHT-1));
}

/** \brief Prints a string
 *
 * \param[in] x uint8_t Horizontal position
 * \param[in] y uint8_t Vertical position
 * \param[in] c char* String to be printed
 *
 */

void LCDdrawstring(uint8_t x, uint8_t y, char *c)
{
    cursor_x = x;
    cursor_y = y;
    while(*c)
    {
        LCDwrite(*c++);
    }
}

/** \brief  Prints a character
 *
 * \param[in] x uint8_t Horizontal position
 * \param[in] y uint8_t Vertical position
 * \param[in] c char Character to be printed
 *
 */

void LCDdrawchar(uint8_t x, uint8_t y, char c)
{
    if(y>=LCDHEIGHT) return;
    if((x+5)>=LCDWIDTH) return;
    uint8_t i,j;
    for(i=0; i<5; ++i)
    {
        uint8_t d = *(font+(c*5)+i);
        uint8_t j;
        for(j=0; j<8; ++j)
        {
            if(d&_BV(j)) __setpixel(x+i, y+j, textcolor);
            else __setpixel(x+i, y+j, !textcolor);
        }
    }

    for(j=0; j<8; ++j)
    {
        __setpixel(x+5, y+j, !textcolor);
    }
    updateBoundingBox(x, y, x+5, y + 8);
}

/** \brief  Prints a character at current position
 *
 * \param[in] c uint8_t Character to be printed
 *
 */

void LCDwrite(uint8_t c)
{
    if(c=='\n')
    {
        cursor_y += textsize*8;
        cursor_x = 0;
    }
    else if(c!='\r')
    {
        LCDdrawchar(cursor_x, cursor_y, c);
        cursor_x += textsize*6;
        if(cursor_x>=(LCDWIDTH-5))
        {
            cursor_x = 0;
            cursor_y += 8;
        }
        if(cursor_y>=LCDHEIGHT) cursor_y = 0;
    }
}

/** \brief Set LCD display mode
 *
 * \param mode uint8_t Display mode (PCD8544_DISPLAYBLANK/PCD8544_DISPLAYNORMAL/PCD8544_DISPLAYALLON/PCD8544_DISPLAYINVERTED)
 *
 */

void LCDsetDisplayMode(uint8_t mode)
{
    LCDcommand(PCD8544_DISPLAYCONTROL|mode);
}

/** \brief Sets the LCD contrast
 *
 * \param[in] val uint8_t Contrast value (0-127)
 *
 */

void LCDsetContrast(uint8_t val)
{
    if(val>0x7f) val = 0x7f;
    LCDcommand(PCD8544_FUNCTIONSET|PCD8544_EXTENDEDINSTRUCTION);
    LCDcommand(PCD8544_SETVOP|val);
    LCDcommand(PCD8544_FUNCTIONSET);
}

/** \brief Sets cursor position
 *
 * \param[in] x uint8_t Horizontal position
 * \param[in] y uint8_t Vertical position
 *
 */

void LCDsetCursor(uint8_t x, uint8_t y)
{
    cursor_x = x;
    cursor_y = y;
}

/** \brief Sets current position on LCD
 *
 * \param[in] x uint8_t Horizontal position
 * \param[in] y uint8_t Vertical position
 *
 */

void LCDsetPosition(uint8_t x, uint8_t y)
{
    static uint8_t _xy[2];
    _xy[0] = x+0x80;
    _xy[1] = y+0x40;
    LCDcommandArray(_xy, 2);
}

/** \brief Sets text size
 *
 * \param s uint8_t Text size
 *
 */

void LCDsetTextSize(uint8_t s)
{
    textsize = s;
}

/** \brief Sets text color
 *
 * \param c uint8_t Text color (WHITE/BLACK)
 *
 */

void LCDsetTextColor(uint8_t c)
{
    textcolor = c;
}

/** \brief Sets a pixel with color
 *
 * \param[in] x uint8_t horizontal position
 * \param[in] y uint8_t vertical position
 * \param[in] color uint8_t WHITE/BLACK
 *
 */

void LCDsetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if((x>=LCDWIDTH)||(y>=LCDHEIGHT)) return;
    if(color) pcd8544_buffer[x+(y/8)*LCDWIDTH] |= _BV(y%8);
    else pcd8544_buffer[x+(y/8)*LCDWIDTH] &= ~_BV(y%8);
    updateBoundingBox(x, y, x, y);
}

/** \brief Gets a pixel's value
 *
 * \param[in] x uint8_t Horizontal position
 * \param[in] y uint8_t Vertical position
 * \return uint8_t Pixel value
 *
 */

uint8_t LCDgetPixel(uint8_t x, uint8_t y)
{
    if((x>=LCDWIDTH)||(y>=LCDHEIGHT)) return 0;
    return(pcd8544_buffer[x+(y/8)*LCDWIDTH]>>(7-(y%8)))&0x1;
}

/** \brief Draws a line.
 *
 * \param[in] x0 uint8_t Horizontal start position
 * \param[in] y0 uint8_t Vertical start position
 * \param[in] x1 uint8_t Horizontal end position
 * \param[in] y1 uint8_t Vertical end position
 * \param[in] color uint8_t WHITE/BLACK
 *
 */

void LCDdrawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color)
{
    uint8_t steep = abs(y1-y0)>abs(x1-x0);
    if(steep)
    {
        swap(x0, y0);
        swap(x1, y1);
    }
    if(x0>x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }
    updateBoundingBox(x0, y0, x1, y1);
    uint8_t dx, dy;
    dx = x1-x0;
    dy = abs(y1-y0);
    int8_t err = dx/2;
    int8_t ystep;

    if(y0<y1) ystep = 1;
    else ystep = -1;
    for(; x0<=x1; ++x0)
    {
        if(steep) __setpixel(y0, x0, color);
        else __setpixel(x0, y0, color);
        err -= dy;
        if(err<0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

/** \brief Draws a rectangle.
 *
 * \param[in] x uint8_t Horizontal start position
 * \param[in] y uint8_t Vertical start position
 * \param[in] w uint8_t Width
 * \param[in] h uint8_t Height
 * \param[in] color uint8_t WHITE/BLACK
 *
 */

void LCDdrawrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
    uint8_t i;
    for(i=x; i<x+w; ++i)
    {
        __setpixel(i, y, color);
        __setpixel(i, y+h-1, color);
    }
    for(i=y; i<y+h; ++i)
    {
        __setpixel(x, i, color);
        __setpixel(x+w-1, i, color);
    }
    updateBoundingBox(x, y, x+w, y+h);
}

/** \brief Draws a filled rectangle.
 *
 * \param[in] x uint8_t Horizontal start position
 * \param[in] y uint8_t Vertical start position
 * \param[in] w uint8_t Width
 * \param[in] h uint8_t Height
 * \param[in] color uint8_t WHITE/BLACK
 *
 */

void LCDfillrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,  uint8_t color)
{
    uint8_t i,j;
    for(i=x; i<x+w; ++i)
    {
        for(j=y; j<y+h; ++j)
        {
            __setpixel(i, j, color);
        }
    }
    updateBoundingBox(x, y, x+w, y+h);
}

/** \brief Draws a circle.
 *
 * \param[in] x0 uint8_t Horizontal position
 * \param[in] y0 uint8_t Vertical position
 * \param[in] r uint8_t Radius
 * \param[in] color uint8_t WHITE/BLACK
 *
 */

void LCDdrawcircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color)
{
    updateBoundingBox(x0-r, y0-r, x0+r, y0+r);
    int8_t f = 1-r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2*r;
    int8_t x = 0;
    int8_t y = r;
    __setpixel(x0, y0+r, color);
    __setpixel(x0, y0-r, color);
    __setpixel(x0+r, y0, color);
    __setpixel(x0-r, y0, color);
    while(x<y)
    {
        if(f>=0)
        {
            --y;
            ddF_y += 2;
            f += ddF_y;
        }
        ++x;
        ddF_x += 2;
        f += ddF_x;
        __setpixel(x0+x, y0+y, color);
        __setpixel(x0-x, y0+y, color);
        __setpixel(x0+x, y0-y, color);
        __setpixel(x0-x, y0-y, color);
        __setpixel(x0+y, y0+x, color);
        __setpixel(x0-y, y0+x, color);
        __setpixel(x0+y, y0-x, color);
        __setpixel(x0-y, y0-x, color);
    }
}

/** \brief Draws a filled circle.
 *
 * \param[in] x0 uint8_t Horizontal position
 * \param[in] y0 uint8_t Vertical position
 * \param[in] r uint8_t Radius
 * \param[in] color uint8_t WHITE/BLACK
 *
 */

void LCDfillcircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t color)
{
    updateBoundingBox(x0-r, y0-r, x0+r, y0+r);
    int8_t f = 1 - r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2 * r;
    int8_t x = 0;
    int8_t y = r;
    uint8_t i;

    for(i=y0-r; i<=y0+r; ++i)
    {
        __setpixel(x0, i, color);
    }

    while(x<y)
    {
        if(f>=0)
        {
            --y;
            ddF_y += 2;
            f += ddF_y;
        }
        ++x;
        ddF_x += 2;
        f += ddF_x;
        for(i=y0-y; i<=y0+y; ++i)
        {
            __setpixel(x0+x, i, color);
            __setpixel(x0-x, i, color);
        }
        for(i=y0-x; i<=y0+x; ++i)
        {
            __setpixel(x0+y, i, color);
            __setpixel(x0-y, i, color);
        }
    }
}

/** \brief Writes out a byte
 *
 * \param[in] c uint8_t Byte
 *
 */

void LCDspiwrite(uint8_t c)
{
    if(_spi_enabled) wiringPiSPIDataRW(0, &c, 1);
    else digitalWriteSerial(idx, c);
}

/** \brief Writes out an array
 *
 * \param[in] c uint8_t* Array
 * \param[in] n uint16_t Size of array
 *
 */

void LCDspiwriteArray(uint8_t *c, uint16_t n)
{
    if(_spi_enabled) wiringPiSPIDataRW (0, c, n);
    else digitalWriteSerialArray(idx, c, n);
}

/** \brief Writes out a command byte
 *
 * \param[in] c uint8_t Command
 *
 */

void LCDcommand(uint8_t c)
{
    digitalWrite(_dc, LOW);
    LCDspiwrite(c);
}

/** \brief Writes out a command array
 *
 * \param[in] c uint8_t* Command array
 * \param[in] n uint16_t Size of array
 *
 */

void LCDcommandArray(uint8_t *c, uint16_t n)
{
    digitalWrite(_dc, LOW);
    LCDspiwriteArray(c, n);
}

/** \brief Writes out a data byte
 *
 * \param[in] c uint8_t Data
 *
 */

void LCDdata(uint8_t c)
{
    digitalWrite(_dc, HIGH);
    LCDspiwrite(c);
}

/** \brief Writes out a data array
 *
 * \param[in] c uint8_t* Data array
 * \param[in] n uint16_t Size of array
 *
 */

void LCDdataArray(uint8_t *c, uint16_t n)
{
    digitalWrite(_dc, HIGH);
    LCDspiwriteArray(c, n);
}

/** \brief Resets the drawing buffer
 *
 *
 */

void LCDzero(void)
{
    memset(pcd8544_buffer, 0, LCDWIDTH*LCDHEIGHT/8);
    xUpdateMin = 0;
    xUpdateMax = (LCDWIDTH-1);
    yUpdateMin = 0;
    yUpdateMax = (LCDHEIGHT-1);
}

/** \brief Displays the drawing buffer
 *
 *
 */

void LCDdisplay(void)
{
    LCDsetPosition(0,0);
    LCDdataArray(pcd8544_buffer, LCDWIDTH*LCDHEIGHT/8);
    LCDsetPosition(0,0);
    xUpdateMin = (LCDWIDTH-1);
    xUpdateMax = 0;
    yUpdateMin = (LCDHEIGHT-1);
    yUpdateMax = 0;
}

/** \brief Updates the LCD
 *
 *
 */

void LCDupdate(void)
{
    uint8_t p;
    for(p=0; p<48; p+=8)
    {
        if(yUpdateMin>=(p+8)) continue;
        if(yUpdateMax<p) break;
        LCDcommand(PCD8544_SETYADDR|(p>>3));
        LCDcommand(PCD8544_SETXADDR|xUpdateMin);
        LCDdataArray(pcd8544_buffer+(LCDWIDTH*p)+xUpdateMin, (xUpdateMax-xUpdateMin));
    }
    LCDcommand(PCD8544_SETYADDR);
    xUpdateMin = (LCDWIDTH-1);
    xUpdateMax = 0;
    yUpdateMin = (LCDHEIGHT-1);
    yUpdateMax = 0;
}

/** \brief Clears the LCD
 *
 *
 */

void LCDclear(void)
{
    memset(pcd8544_buffer, 0, LCDWIDTH*LCDHEIGHT/8);
    LCDsetPosition(0, 0);
    LCDdataArray(pcd8544_buffer, LCDWIDTH*LCDHEIGHT/8);
    LCDsetPosition(0, 0);
    updateBoundingBox(0, 0, (LCDWIDTH-1), (LCDHEIGHT-1));
}

/** \brief Delays for milliseconds
 *
 * \param[in] msecs uint32_t milliseconds to delay
 *
 */

void delay(uint32_t msecs)
{
    usleep(1000*msecs);
}
