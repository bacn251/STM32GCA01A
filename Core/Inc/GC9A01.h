/*
 * GC9A01.h
 *
 *  Created on: Mar 3, 2026
 *      Author: Bacnk
 */

#ifndef INC_GC9A01_H_
#define INC_GC9A01_H_
#include "gpio.h"
#include "spi.h"
#include <stdint.h>
typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *DC_Port;
    uint16_t DC_Pin;
    GPIO_TypeDef *RST_Port;
    uint16_t RST_Pin;
} GC9A01_ConfigTypeDef;
#define GC9A01_SPI_TIMEOUT   100
#define USE_DMA              0  // Enable/disable DMA
void GC9A01_Init_Config(GC9A01_ConfigTypeDef *GC9A01_Config, SPI_HandleTypeDef *hspi,GPIO_TypeDef *CS_Port, uint16_t CS_Pin ,GPIO_TypeDef *DC_Port, uint16_t DC_Pin, GPIO_TypeDef *RST_Port, uint16_t RST_Pin);
void GC9A01_Init(GC9A01_ConfigTypeDef *GC9A01_Config);
void GC9A01_FlushReady(GC9A01_ConfigTypeDef *GC9A01_Config);
void GC9A01_Reset(GC9A01_ConfigTypeDef *GC9A01_Config);
void GC9A01_WriteCommand(GC9A01_ConfigTypeDef *GC9A01_Config, uint8_t cmd);
void GC9A01_WriteData(GC9A01_ConfigTypeDef *GC9A01_Config, uint8_t data);
void GC9A01_WriteDataBuffer(GC9A01_ConfigTypeDef *GC9A01_Config, uint8_t *data, uint16_t size);
void GC9A01_SetAddressWindow(GC9A01_ConfigTypeDef *GC9A01_Config, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void GC9A01_DrawPixel(GC9A01_ConfigTypeDef *GC9A01_Config, uint16_t x, uint16_t y, uint16_t color);
void GC9A01_FillRect(GC9A01_ConfigTypeDef *GC9A01_Config, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void GC9A01_Flush(GC9A01_ConfigTypeDef *GC9A01_Config,const void *color_map, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
#endif /* INC_GC9A01_H_ */
