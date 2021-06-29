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
TMC5160_Ref_e tmc_eRefState;
int tmc_iRefCnt;
int tmc_iPosition=0;

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
	tmc_eRefState = REF_NO;

	uint8_t u8Version;
	TMC5160_REG_GCONF unGconf = {0};
	TMC5160_REG_SHORT_CONF unShortConf = {0};
	TMC5160_REG_DRV_CONF unDrvConf = {0};
	TMC5160_REG_IHOLD_IRUN unIHoldIRun = {0};
	TMC5160_REG_TPOWERDOWN unTPowerDown = {0};
	TMC5160_REG_SW_MODE unSWMode = {0};
	TMC5160_REG_ENC_CONST enEncConst = {0};
	TMC5160_REG_CHOPCONF enChopConf = {0};
	TMC5160_REG_PWMCONF enPWMConf = {0};

	tmc_bTransmitting = 0;

	// Read the version
	TMC5160_ReadData(TMC5160_IOIN);
	// Get the data with a 2nd SPI access
	u8Version = TMC5160_ReadData(TMC5160_IOIN)>>24;
	if (u8Version == 0x30)
	{
		PRINTF_printf("TMC5160 found with version: 0x%02x.", u8Version);
		CONSOLE_Prompt();
	}
	else
	{
		ERRORHANDLER_SetError(ERROR_TMC5160_VERSION);
		PRINTF_printf("TMC5160 not found. Version value: 0x%02x. Is it powered?", u8Version);
		CONSOLE_Prompt();
	}

	unGconf.multistep_filt = 1;

	unShortConf.S2VS_LEVEL = 6;
	unShortConf.S2G_LEVEL = 6;
	unShortConf.SHORTFILTER = 1;

	unDrvConf.BBMCLKS = 4;
	unDrvConf.DRVSTRENGTH = 2;

	unIHoldIRun.IHOLD =15; // 0..31 IHold
	unIHoldIRun.IRUN = 31; // 0..31 IRun
	unIHoldIRun.IHOLDDELAY = 7;

	unTPowerDown.TPOWERDOWN = 10;

	unSWMode.STOP_L_ENABLE = 1;
	unSWMode.LATCH_L_ACTIVE = 1;

	enEncConst.ENC_CONST = 0x00010000;

	enChopConf.TOFF = 3;
	enChopConf.HRST = 5;
	enChopConf.HEND13 = 1;
	enChopConf.TBL0 = 0;
	enChopConf.TBL1 = 1;
	enChopConf.TPFD = 4;

	enPWMConf.PWM_OFS = 30;
	enPWMConf.PWM_GRAD = 0;
	enPWMConf.PWM_AUTOGRAD = 1;
	enPWMConf.PWM_AUTOSCALE = 1;
	enPWMConf.PWM_REG = 4;
	enPWMConf.PWM_LIM = 12;

	TMC5160_WriteData(TMC5160_GCONF, unGconf.u32);			// GCONF
	TMC5160_WriteData(TMC5160_SHORTCONF, unShortConf.u32);	// SHORTCONF
	TMC5160_WriteData(TMC5160_DRVCONF, unDrvConf.u32);		// DRVCONF
	TMC5160_WriteData(TMC5160_IHOLD_IRUN, unIHoldIRun.u32);	// IHOLD_IRUN
	TMC5160_WriteData(TMC5160_TPOWERDOWN, unTPowerDown.u32);// TPOWERDOWN
	TMC5160_WriteData(TMC5160_SW_MODE	, unSWMode.u32);	// SW_MODE
	TMC5160_WriteData(TMC5160_ENC_CONST, enEncConst.u32);	// ENC_CONST
	TMC5160_WriteData(TMC5160_CHOPCONF, enChopConf.u32);	// CHOPCONF
	TMC5160_WriteData(TMC5160_PWMCONF, enPWMConf.u32);		// PWMCONF

	TMC5160_WriteData(TMC5160_A1, 1000);		// A1
	TMC5160_WriteData(TMC5160_V1, 0);			// V1
	TMC5160_WriteData(TMC5160_AMAX, 40000);		// AMAX
	TMC5160_WriteData(TMC5160_VMAX, TMC_VMAX);	// VMAX
	TMC5160_WriteData(TMC5160_DMAX, 40000);		// DMAX
	TMC5160_WriteData(TMC5160_D1, 1400);		// D1
	TMC5160_WriteData(TMC5160_VSTART, 10);		// VSTART
	TMC5160_WriteData(TMC5160_VSTOP, 10);		// VSTOP
	TMC5160_WriteData(TMC5160_RAMPMODE, 0);		// Target position move

	// Reset the position
	TMC5160_WriteData(TMC5160_XACTUAL, 0);
	TMC5160_WriteData(TMC5160_XTARGET, 0);

	// Enable driver
	HAL_GPIO_WritePin(DRV_ENN_GPIO_Port, DRV_ENN_Pin, GPIO_PIN_RESET);
}


/**
 * Call this function every 1ms from main
 *
 *
 */
void TMC5160_Task1ms()
{
	switch (tmc_eRefState)
	{
	case REF_NO:
		break;
	case REF_START:
		TMC5160_WriteData(TMC5160_VMAX, 4000 );
		TMC5160_ReadData(TMC5160_RAMP_STAT);
		tmc_iRefCnt = 0;
		if (((TMC5160_REG_RAMP_STAT)TMC5160_ReadData(TMC5160_RAMP_STAT)).STATUS_STOP_L)
		{
			// We are in stop position, so move right a little bit right
			TMC5160_WriteData(TMC5160_XTARGET, 1000);
		}
		else
		{
			tmc_iRefCnt = 1000;
		}

		tmc_eRefState = REF_MOVE_R;
		break;
	case REF_MOVE_R:
		if (tmc_iRefCnt > 1000)
		{
			TMC5160_WriteData(TMC5160_XTARGET, -30000);
			TMC5160_ReadData(TMC5160_RAMP_STAT);
			tmc_iRefCnt = 0;
			tmc_eRefState = REF_MOVE_REF;
		}
		break;
	case REF_MOVE_REF:
		if (((TMC5160_REG_RAMP_STAT)TMC5160_ReadData(TMC5160_RAMP_STAT)).VZERO)
		{
			tmc_iRefCnt = 0;
			TMC5160_WriteData(TMC5160_RAMPMODE, 3);		// Hold mode
			tmc_eRefState = REF_FINISH;
		}
		break;
	case REF_FINISH:
		if (tmc_iRefCnt > 1000)
		{
			TMC5160_ReadData(TMC5160_XACTUAL);
			int iXactual = TMC5160_ReadData(TMC5160_XACTUAL);
			TMC5160_ReadData(TMC5160_XLATCH);
			int iXlatch = TMC5160_ReadData(TMC5160_XLATCH);
			TMC5160_WriteData(TMC5160_XACTUAL, iXactual- iXlatch);

			TMC5160_WriteData(TMC5160_VMAX, TMC_VMAX );
			TMC5160_WriteData(TMC5160_RAMPMODE, 0);		// Position mode
			TMC5160_WriteData(TMC5160_XTARGET, 6800);
			tmc_eRefState = REF_NO;
		}

		break;
	default:
		tmc_eRefState = REF_NO;
		break;
	}
	tmc_iRefCnt++;
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

	// For read we wait also until last transmission is completed
	while (tmc_bTransmitting);

	return __REV(tmc_sSpiDMARxBuff.u32Data);
}

/**
 * Move to position
 *
 * \param s32Position Destination position
 *
 *
 */
void TMC5160_MoveTo(int32_t s32Position)
{
	if (tmc_eRefState != REF_NO)
		return;

	tmc_iPosition = s32Position;
	TMC5160_WriteData(TMC5160_XTARGET, s32Position);
}

/**
 * Gets the current position
 *
 * \return the position
 *
 *
 */
int TMC5160_GetPos()
{
	return tmc_iPosition;
}

/**
 * Starts a reference move
 *
 * \param s32Position Destination position
 *
 *
 */
void TMC5160_Ref()
{
	tmc_eRefState = REF_START;
}

/**
 * Gets the reference state
 *
 * \return true, if referencing
 *
 *
 */
int TMC5160_IsReferencing()
{
	return (tmc_eRefState != REF_NO);
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
