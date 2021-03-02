/*
 *  Project:      BeatBassBox
 *  File:         tmc5160.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Stepper driver for TMC5160 SilentStepStick
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


/* Includes ------------------------------------------------------------------*/
#include "tmc5160.h"
#include "spi.h"
#include "printf.h"
#include "console.h"
#include "errorhandler.h"

/* Variables -----------------------------------------------------------------*/
TMC5160_SPI_TX_s tmc_sSpiDMATxBuff;
TMC5160_SPI_RX_s tmc_sSpiDMARxBuff;
int tmc_bTransmitting;
uint8_t tmc_u8LastReadAllAddr;

/* Prototypes of static function ---------------------------------------------*/
static void TMC5160_WriteData(uint8_t u8Addr, uint32_t u32Data);
static uint32_t TMC5160_ReadData(uint8_t u8Addr);
static void TMC5160_ReadAllNext(uint8_t u8Addr);


/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 *
 */
void TMC5160_Init()
{
	volatile uint8_t u8Version;

	tmc_bTransmitting = 0;

	// Read the version
	TMC5160_ReadData(TMC5160_IOIN);
	// Get the data with a 2nd SPI access
	u8Version = TMC5160_ReadData(TMC5160_IOIN)>>24;
	if (u8Version != 0x30)
	{
		ERRORHANDLER_SetError(ERROR_TMC5160_VERSION);
		PRINTF_printf("TMC5160 not found. Version value: 0x%02x. Is it powered?", u8Version);
		CONSOLE_Prompt();
	}
}


/**
 * Call this function every 1ms from main
 *
 *
 */
void TMC5160_Task1ms()
{
}

/**
 * Writes data to the TMC5160
 *
 * \param u8Addr TMC5160 address of the register
 * \param u32Data Data to write into the register
 *
 *
 */
static void TMC5160_WriteData(uint8_t u8Addr, uint32_t u32Data)
{
	// Wait until last transmission is completed
	while (tmc_bTransmitting);

	// Fill data
	tmc_sSpiDMATxBuff.u1W = 1;
	tmc_sSpiDMATxBuff.u7Address = u8Addr;
	tmc_sSpiDMATxBuff.u32Data = __REV(u32Data);

	// Transmit it
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
	tmc_bTransmitting = 1;
	HAL_SPI_TransmitReceive_DMA(
			&hspi2,
			(uint8_t*)&tmc_sSpiDMATxBuff,
			(uint8_t*)&tmc_sSpiDMARxBuff,
			5);
}

/**
 * reads data from the TMC5160
 *
 * \param u8Addr TMC5160 address of the register
 *
 * \return u32Data Data read from the last SPI operation
 *
 *
 */
static uint32_t TMC5160_ReadData(uint8_t u8Addr)
{
	// Wait until last transmission is completed
	while (tmc_bTransmitting);

	// Fill data
	tmc_sSpiDMATxBuff.u1W = 0;
	tmc_sSpiDMATxBuff.u7Address = u8Addr;
	tmc_sSpiDMATxBuff.u32Data = 0;

	// Transmit it
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
	tmc_bTransmitting = 1;
	HAL_SPI_TransmitReceive_DMA(
			&hspi2,
			(uint8_t*)&tmc_sSpiDMATxBuff,
			(uint8_t*)&tmc_sSpiDMARxBuff,
			5);

	// For read we wailt also until last transmission is completed
	while (tmc_bTransmitting);

	return __REV(tmc_sSpiDMARxBuff.u32Data);
}

/**
 * Writes data to the TMC5160
 *
 * \param u8Addr TMC5160 address of the register
 * \param u32Data Data to write into the register
 *
 *
 */
void TMC5160_Write(uint8_t u8Addr, uint32_t u32Data)
{
	TMC5160_WriteData(u8Addr, u32Data);
}

/**
 * reads data from the TMC5160 and prints the result on the console
 *
 * \param u8Addr TMC5160 address of the register
 *
 *
 */
void TMC5160_Read(uint8_t u8Addr)
{
	TMC5160_ReadData(u8Addr);
	tmc_u8LastReadAllAddr = u8Addr;
	// Get the data with a 2nd SPI access
	TMC5160_ReadAllNext(u8Addr);
}

/**
 * reads data from the TMC5160 and prints the result on the console
 *
 * \param u8Addr TMC5160 address of the register
 *
 *
 */
static void TMC5160_ReadAllNext(uint8_t u8Addr)
{
	PRINTF_printf("0x%02x: 0x%8x",tmc_u8LastReadAllAddr, TMC5160_ReadData(u8Addr));
	CONSOLE_NewLine();
	tmc_u8LastReadAllAddr = u8Addr;
}
/**
 * reads data from the TMC5160 and prints the result on the console
 *
 */
void TMC5160_ReadAll()
{
	// Read all Registers

	TMC5160_ReadData(0x00);
	tmc_u8LastReadAllAddr = 0x00;

	for (uint8_t i=0x01;i<=0x0C; i++)
		TMC5160_ReadAllNext(i);
	for (uint8_t i=0x10;i<=0x15; i++)
		TMC5160_ReadAllNext(i);
	for (uint8_t i=0x20;i<=0x2D; i++)
		TMC5160_ReadAllNext(i);
	for (uint8_t i=0x33;i<=0x36; i++)
		TMC5160_ReadAllNext(i);
	for (uint8_t i=0x38;i<=0x3D; i++)
		TMC5160_ReadAllNext(i);
	for (uint8_t i=0x60;i<=0x73; i++)
		TMC5160_ReadAllNext(i);

	TMC5160_ReadAllNext(0x00);
}

/**
 * Call this function every 1ms from main
 *
 *
 */
void TMC5160_Task100ms()
{
//	TMC5160_WriteData(3, 0x80000055);
}
/**
  * @brief  Tx and Rx Transfer completed callback.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);
  tmc_bTransmitting = 0;
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);


  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_TxRxCpltCallback should be implemented in the user file
   */
}
