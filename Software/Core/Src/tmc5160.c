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
#include "errorhandler.h"

/* Variables -----------------------------------------------------------------*/
TMC5160_SPI_TX_s tmc_sSpiDMATxBuff;
TMC5160_SPI_RX_s tmc_sSpiDMARxBuff;
int tmc_bTransmitting;

/* Prototypes of static function ---------------------------------------------*/
static void TMC5160_WriteData(int iAddress, uint32_t u32Data);
static uint32_t TMC5160_ReadData(int iAddress);



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
 * \param iAddress TMC5160 address of the register
 * \param u32Data Data to write into the register
 *
 *
 */
static void TMC5160_WriteData(int iAddress, uint32_t u32Data)
{
	// Wait until last transmission is completed
	while (tmc_bTransmitting);

	// Fill data
	tmc_sSpiDMATxBuff.u1W = 1;
	tmc_sSpiDMATxBuff.u7Address = iAddress;
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
 * \param iAddress TMC5160 address of the register
 *
 * \return u32Data Data read from the last SPI operation
 *
 *
 */
static uint32_t TMC5160_ReadData(int iAddress)
{
	// Wait until last transmission is completed
	while (tmc_bTransmitting);

	// Fill data
	tmc_sSpiDMATxBuff.u1W = 0;
	tmc_sSpiDMATxBuff.u7Address = iAddress;
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
