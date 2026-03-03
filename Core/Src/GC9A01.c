/*
 * GC9A01.c
 *
 *  Created on: Mar 3, 2026
 *      Author: Bacnk
 */
#include "GC9A01.h"

void GC9A01_Init_Config(GC9A01_ConfigTypeDef *GC9A01_Config, SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_Port, uint16_t CS_Pin, GPIO_TypeDef *DC_Port, uint16_t DC_Pin, GPIO_TypeDef *RST_Port, uint16_t RST_Pin)
{
    GC9A01_Config->hspi = hspi;
    GC9A01_Config->CS_Port = CS_Port;
    GC9A01_Config->CS_Pin = CS_Pin;
    GC9A01_Config->DC_Port = DC_Port;
    GC9A01_Config->DC_Pin = DC_Pin;
    GC9A01_Config->RST_Port = RST_Port;
    GC9A01_Config->RST_Pin = RST_Pin;
}

static void GC9A01_Select(GC9A01_ConfigTypeDef *GC9A01_Config)
{
    HAL_GPIO_WritePin(GC9A01_Config->CS_Port, GC9A01_Config->CS_Pin, GPIO_PIN_RESET);
}
static void GC9A01_Unselect(GC9A01_ConfigTypeDef *GC9A01_Config)
{
    HAL_GPIO_WritePin(GC9A01_Config->CS_Port, GC9A01_Config->CS_Pin, GPIO_PIN_SET);
}
static void GC9A01_DC_Command(GC9A01_ConfigTypeDef *GC9A01_Config)
{
    HAL_GPIO_WritePin(GC9A01_Config->DC_Port, GC9A01_Config->DC_Pin, GPIO_PIN_RESET);
}
static void GC9A01_DC_Data(GC9A01_ConfigTypeDef *GC9A01_Config)
{
    HAL_GPIO_WritePin(GC9A01_Config->DC_Port, GC9A01_Config->DC_Pin, GPIO_PIN_SET);
}
void GC9A01_Reset(GC9A01_ConfigTypeDef *GC9A01_Config)
{
    HAL_GPIO_WritePin(GC9A01_Config->RST_Port, GC9A01_Config->RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(GC9A01_Config->RST_Port, GC9A01_Config->RST_Pin, GPIO_PIN_SET);
    HAL_Delay(120);
}
void GC9A01_WriteCommand(GC9A01_ConfigTypeDef *GC9A01_Config, uint8_t cmd)
{
    GC9A01_Select(GC9A01_Config);
    GC9A01_DC_Command(GC9A01_Config);
    HAL_SPI_Transmit(GC9A01_Config->hspi, &cmd, 1, GC9A01_SPI_TIMEOUT);
    GC9A01_Unselect(GC9A01_Config);
}
void GC9A01_WriteData(GC9A01_ConfigTypeDef *GC9A01_Config, uint8_t data)
{
    GC9A01_Select(GC9A01_Config);
    GC9A01_DC_Data(GC9A01_Config);
    HAL_SPI_Transmit(GC9A01_Config->hspi, &data, 1, GC9A01_SPI_TIMEOUT);
    GC9A01_Unselect(GC9A01_Config);
}
void GC9A01_WriteDataBuffer(GC9A01_ConfigTypeDef *GC9A01_Config, uint8_t *data, uint16_t size)
{
    GC9A01_Select(GC9A01_Config);
    GC9A01_DC_Data(GC9A01_Config);
    HAL_SPI_Transmit(GC9A01_Config->hspi, data, size, GC9A01_SPI_TIMEOUT);
    GC9A01_Unselect(GC9A01_Config);
}
void GC9A01_SetAddressWindow(GC9A01_ConfigTypeDef *GC9A01_Config, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    GC9A01_WriteCommand(GC9A01_Config, 0x2A); // Column Address Set
    uint8_t data_col[] = {x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF};
    GC9A01_WriteDataBuffer(GC9A01_Config, data_col, sizeof(data_col));

    GC9A01_WriteCommand(GC9A01_Config, 0x2B); // Row Address Set
    uint8_t data_row[] = {y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF};
    GC9A01_WriteDataBuffer(GC9A01_Config, data_row, sizeof(data_row));

    GC9A01_WriteCommand(GC9A01_Config, 0x2C); // Memory Write
}
void GC9A01_DrawPixel(GC9A01_ConfigTypeDef *GC9A01_Config, uint16_t x, uint16_t y, uint16_t color)
{
    GC9A01_SetAddressWindow(GC9A01_Config, x, y, x, y);
    uint8_t data[] = {color >> 8, color & 0xFF};
    GC9A01_WriteDataBuffer(GC9A01_Config, data, sizeof(data));
}
void GC9A01_FillRect(GC9A01_ConfigTypeDef *GC9A01_Config, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint32_t size = w * h;
    GC9A01_SetAddressWindow(GC9A01_Config, x, y, x + w - 1, y + h - 1);
    uint8_t buf[512];
    for (int i = 0; i < sizeof(buf); i += 2)
    {
        buf[i] = color >> 8;
        buf[i + 1] = color & 0xFF;
    }
    while (size > 0)
    {
        uint16_t chunk = (size > 256) ? 256 : size;
        GC9A01_WriteDataBuffer(GC9A01_Config, buf, chunk * 2);
        size -= chunk;
    }
}
void GC9A01_Flush(GC9A01_ConfigTypeDef *GC9A01_Config, const void *color_map, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;

    GC9A01_SetAddressWindow(GC9A01_Config, x1, y1, x2, y2);
    GC9A01_Select(GC9A01_Config);
    GC9A01_DC_Data(GC9A01_Config);
    HAL_SPI_Transmit(GC9A01_Config->hspi, (uint8_t *)color_map, size, GC9A01_SPI_TIMEOUT);
    GC9A01_Unselect(GC9A01_Config);
    GC9A01_FlushReady(GC9A01_Config);
}
__weak void GC9A01_FlushReady(void)
{
	// Implement in user code:
}
void GC9A01_Init(GC9A01_ConfigTypeDef *GC9A01_Config)
{
    GC9A01_Reset(GC9A01_Config);

	GC9A01_WriteCommand(GC9A01_Config, 0xEF);
	GC9A01_WriteCommand(GC9A01_Config, 0xEB);
	GC9A01_WriteData(GC9A01_Config, 0x14);

	GC9A01_WriteCommand(GC9A01_Config, 0xFE);
	GC9A01_WriteCommand(GC9A01_Config, 0xEF);

	GC9A01_WriteCommand(GC9A01_Config, 0xEB);
	GC9A01_WriteData(GC9A01_Config, 0x14);

	GC9A01_WriteCommand(GC9A01_Config, 0x84);
	GC9A01_WriteData(GC9A01_Config, 0x40);

	GC9A01_WriteCommand(GC9A01_Config, 0x85);
	GC9A01_WriteData(GC9A01_Config, 0xFF);

	GC9A01_WriteCommand(GC9A01_Config, 0x86);
	GC9A01_WriteData(GC9A01_Config, 0xFF);

	GC9A01_WriteCommand(GC9A01_Config, 0x87);
	GC9A01_WriteData(GC9A01_Config, 0xFF);

	GC9A01_WriteCommand(GC9A01_Config, 0x88);
	GC9A01_WriteData(GC9A01_Config, 0x0A);

	GC9A01_WriteCommand(GC9A01_Config, 0x89);
	GC9A01_WriteData(GC9A01_Config, 0x21);

	GC9A01_WriteCommand(GC9A01_Config, 0x8A);
	GC9A01_WriteData(GC9A01_Config, 0x00);

	GC9A01_WriteCommand(GC9A01_Config, 0x8B);
	GC9A01_WriteData(GC9A01_Config, 0x80);

	GC9A01_WriteCommand(GC9A01_Config, 0x8C);
	GC9A01_WriteData(GC9A01_Config, 0x01);

	GC9A01_WriteCommand(GC9A01_Config, 0x8D);
	GC9A01_WriteData(GC9A01_Config, 0x01);

	GC9A01_WriteCommand(GC9A01_Config, 0x8E);
	GC9A01_WriteData(GC9A01_Config, 0xFF);

	GC9A01_WriteCommand(GC9A01_Config, 0x8F);
	GC9A01_WriteData(GC9A01_Config, 0xFF);


	GC9A01_WriteCommand(GC9A01_Config, 0xB6);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x20);

	GC9A01_WriteCommand(GC9A01_Config, 0x36);
	GC9A01_WriteData(GC9A01_Config, 0x08);//Set as vertical screen

	GC9A01_WriteCommand(GC9A01_Config, 0x3A);
	GC9A01_WriteData(GC9A01_Config, 0x05);


	GC9A01_WriteCommand(GC9A01_Config, 0x90);
	GC9A01_WriteData(GC9A01_Config, 0x08);
	GC9A01_WriteData(GC9A01_Config, 0x08);
	GC9A01_WriteData(GC9A01_Config, 0x08);
	GC9A01_WriteData(GC9A01_Config, 0x08);

	GC9A01_WriteCommand(GC9A01_Config, 0xBD);
	GC9A01_WriteData(GC9A01_Config, 0x06);

	GC9A01_WriteCommand(GC9A01_Config, 0xBC);
	GC9A01_WriteData(GC9A01_Config, 0x00);

	GC9A01_WriteCommand(GC9A01_Config, 0xFF);
	GC9A01_WriteData(GC9A01_Config, 0x60);
	GC9A01_WriteData(GC9A01_Config, 0x01);
	GC9A01_WriteData(GC9A01_Config, 0x04);

	GC9A01_WriteCommand(GC9A01_Config, 0xC3);
	GC9A01_WriteData(GC9A01_Config, 0x13);
	GC9A01_WriteCommand(GC9A01_Config, 0xC4);
	GC9A01_WriteData(GC9A01_Config, 0x13);

	GC9A01_WriteCommand(GC9A01_Config, 0xC9);
	GC9A01_WriteData(GC9A01_Config, 0x22);

	GC9A01_WriteCommand(GC9A01_Config, 0xBE);
	GC9A01_WriteData(GC9A01_Config, 0x11);

	GC9A01_WriteCommand(GC9A01_Config, 0xE1);
	GC9A01_WriteData(GC9A01_Config, 0x10);
	GC9A01_WriteData(GC9A01_Config, 0x0E);

	GC9A01_WriteCommand(GC9A01_Config, 0xDF);
	GC9A01_WriteData(GC9A01_Config, 0x21);
	GC9A01_WriteData(GC9A01_Config, 0x0c);
	GC9A01_WriteData(GC9A01_Config, 0x02);

	GC9A01_WriteCommand(GC9A01_Config, 0xF0);
	GC9A01_WriteData(GC9A01_Config, 0x45);
	GC9A01_WriteData(GC9A01_Config, 0x09);
	GC9A01_WriteData(GC9A01_Config, 0x08);
	GC9A01_WriteData(GC9A01_Config, 0x08);
	GC9A01_WriteData(GC9A01_Config, 0x26);
	GC9A01_WriteData(GC9A01_Config, 0x2A);

	GC9A01_WriteCommand(GC9A01_Config, 0xF1);
	GC9A01_WriteData(GC9A01_Config, 0x43);
	GC9A01_WriteData(GC9A01_Config, 0x70);
	GC9A01_WriteData(GC9A01_Config, 0x72);
	GC9A01_WriteData(GC9A01_Config, 0x36);
	GC9A01_WriteData(GC9A01_Config, 0x37);
	GC9A01_WriteData(GC9A01_Config, 0x6F);


	GC9A01_WriteCommand(GC9A01_Config, 0xF2);
	GC9A01_WriteData(GC9A01_Config, 0x45);
	GC9A01_WriteData(GC9A01_Config, 0x09);
	GC9A01_WriteData(GC9A01_Config, 0x08);
	GC9A01_WriteData(GC9A01_Config, 0x08);
	GC9A01_WriteData(GC9A01_Config, 0x26);
	GC9A01_WriteData(GC9A01_Config, 0x2A);

	GC9A01_WriteCommand(GC9A01_Config, 0xF3);
	GC9A01_WriteData(GC9A01_Config, 0x43);
	GC9A01_WriteData(GC9A01_Config, 0x70);
	GC9A01_WriteData(GC9A01_Config, 0x72);
	GC9A01_WriteData(GC9A01_Config, 0x36);
	GC9A01_WriteData(GC9A01_Config, 0x37);
	GC9A01_WriteData(GC9A01_Config, 0x6F);

	GC9A01_WriteCommand(GC9A01_Config, 0xED);
	GC9A01_WriteData(GC9A01_Config, 0x1B);
	GC9A01_WriteData(GC9A01_Config, 0x0B);

	GC9A01_WriteCommand(GC9A01_Config, 0xAE);
	GC9A01_WriteData(GC9A01_Config, 0x77);

	GC9A01_WriteCommand(GC9A01_Config, 0xCD);
	GC9A01_WriteData(GC9A01_Config, 0x63);


	GC9A01_WriteCommand(GC9A01_Config, 0x70);
	GC9A01_WriteData(GC9A01_Config, 0x07);
	GC9A01_WriteData(GC9A01_Config, 0x07);
	GC9A01_WriteData(GC9A01_Config, 0x04);
	GC9A01_WriteData(GC9A01_Config, 0x0E);
	GC9A01_WriteData(GC9A01_Config, 0x0F);
	GC9A01_WriteData(GC9A01_Config, 0x09);
	GC9A01_WriteData(GC9A01_Config, 0x07);
	GC9A01_WriteData(GC9A01_Config, 0x08);
	GC9A01_WriteData(GC9A01_Config, 0x03);

	GC9A01_WriteCommand(GC9A01_Config, 0xE8);
	GC9A01_WriteData(GC9A01_Config, 0x34);

	GC9A01_WriteCommand(GC9A01_Config, 0x62);
	GC9A01_WriteData(GC9A01_Config, 0x18);
	GC9A01_WriteData(GC9A01_Config, 0x0D);
	GC9A01_WriteData(GC9A01_Config, 0x71);
	GC9A01_WriteData(GC9A01_Config, 0xED);
	GC9A01_WriteData(GC9A01_Config, 0x70);
	GC9A01_WriteData(GC9A01_Config, 0x70);
	GC9A01_WriteData(GC9A01_Config, 0x18);
	GC9A01_WriteData(GC9A01_Config, 0x0F);
	GC9A01_WriteData(GC9A01_Config, 0x71);
	GC9A01_WriteData(GC9A01_Config, 0xEF);
	GC9A01_WriteData(GC9A01_Config, 0x70);
	GC9A01_WriteData(GC9A01_Config, 0x70);

	GC9A01_WriteCommand(GC9A01_Config, 0x63);
	GC9A01_WriteData(GC9A01_Config, 0x18);
	GC9A01_WriteData(GC9A01_Config, 0x11);
	GC9A01_WriteData(GC9A01_Config, 0x71);
	GC9A01_WriteData(GC9A01_Config, 0xF1);
	GC9A01_WriteData(GC9A01_Config, 0x70);
	GC9A01_WriteData(GC9A01_Config, 0x70);
	GC9A01_WriteData(GC9A01_Config, 0x18);
	GC9A01_WriteData(GC9A01_Config, 0x13);
	GC9A01_WriteData(GC9A01_Config, 0x71);
	GC9A01_WriteData(GC9A01_Config, 0xF3);
	GC9A01_WriteData(GC9A01_Config, 0x70);
	GC9A01_WriteData(GC9A01_Config, 0x70);

	GC9A01_WriteCommand(GC9A01_Config, 0x64);
	GC9A01_WriteData(GC9A01_Config, 0x28);
	GC9A01_WriteData(GC9A01_Config, 0x29);
	GC9A01_WriteData(GC9A01_Config, 0xF1);
	GC9A01_WriteData(GC9A01_Config, 0x01);
	GC9A01_WriteData(GC9A01_Config, 0xF1);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x07);

	GC9A01_WriteCommand(GC9A01_Config, 0x66);
	GC9A01_WriteData(GC9A01_Config, 0x3C);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0xCD);
	GC9A01_WriteData(GC9A01_Config, 0x67);
	GC9A01_WriteData(GC9A01_Config, 0x45);
	GC9A01_WriteData(GC9A01_Config, 0x45);
	GC9A01_WriteData(GC9A01_Config, 0x10);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x00);

	GC9A01_WriteCommand(GC9A01_Config, 0x67);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x3C);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x01);
	GC9A01_WriteData(GC9A01_Config, 0x54);
	GC9A01_WriteData(GC9A01_Config, 0x10);
	GC9A01_WriteData(GC9A01_Config, 0x32);
	GC9A01_WriteData(GC9A01_Config, 0x98);

	GC9A01_WriteCommand(GC9A01_Config, 0x74);
	GC9A01_WriteData(GC9A01_Config, 0x10);
	GC9A01_WriteData(GC9A01_Config, 0x85);
	GC9A01_WriteData(GC9A01_Config, 0x80);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x00);
	GC9A01_WriteData(GC9A01_Config, 0x4E);
	GC9A01_WriteData(GC9A01_Config, 0x00);

	GC9A01_WriteCommand(GC9A01_Config, 0x98);
	GC9A01_WriteData(GC9A01_Config, 0x3e);
	GC9A01_WriteData(GC9A01_Config, 0x07);

	GC9A01_WriteCommand(GC9A01_Config, 0x35);
	GC9A01_WriteCommand(GC9A01_Config, 0x21);

	GC9A01_WriteCommand(GC9A01_Config, 0x11);
	HAL_Delay(120);
	GC9A01_WriteCommand(GC9A01_Config, 0x29);
	HAL_Delay(20);
}