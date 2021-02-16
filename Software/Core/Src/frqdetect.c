/*
 *  Project:      BeatBassBox
 *  File:         frqdetect.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Frequency detection of bass rubber string
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
#include "frqdetect.h"
#include "tim.h"
#include "adc.h"
#include "dac.h"
#include <string.h>

/* Variables -----------------------------------------------------------------*/
//FRQDETECT_1632_u frqd_u1632ADCValues[16] = {0};

uint32_t frqd_u32ADCDMABuff[8];

int frqd_iWrPtr;
int frqd_iRdPtr;
FRQD_blockbuf_t frqd_uBlockBuf[4];


/* Prototypes of static function ---------------------------------------------*/
static void FRQDETECT_Process(uint16_t ui16Sample);


/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 * Starts the ADC DMA
 *
 */
void FRQDETECT_Init()
{
	frqd_iWrPtr = 0;
	frqd_iRdPtr = 0;

	// Workaround. See ERRATA. It is necessary to switch on the DAC clock
	__HAL_RCC_DAC_CLK_ENABLE();
	HAL_ADC_Start_DMA(&hadc1, frqd_u32ADCDMABuff, 16);
	HAL_TIM_Base_Start(&htim5);
}

/**
 * Processes on sample
 *
 * \param ui16Sample 16 (12-bit) ADC value from 0..0x0FFF
 *
 */

static void FRQDETECT_Process(uint16_t ui16Sample)
{
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, ui16Sample);
}

/**
 * Call this function every 1ms from main
 *
 * It processes on (or more) blocks
 *
 */
void FRQDETECT_Task1ms()
{
	while (frqd_iRdPtr != frqd_iWrPtr)
	{
		for (int i=0;i<8;i++)
		{
			FRQDETECT_Process(frqd_uBlockBuf[frqd_iRdPtr].u16[i]);
		}
		frqd_iRdPtr++;
		frqd_iRdPtr &= 0x03;
	}
}

/**
  * @brief  Regular conversion half DMA transfer callback in non blocking mode
  * @param  hadc: pointer to a ADC_HandleTypeDef structure that contains
  *         the configuration information for the specified ADC.
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  memcpy(&frqd_uBlockBuf[frqd_iWrPtr],&frqd_u32ADCDMABuff[0], 16);
  frqd_iWrPtr++;
  frqd_iWrPtr &= 0x03;
}

/**
  * @brief  Regular conversion complete callback in non blocking mode
  * @param  hadc: pointer to a ADC_HandleTypeDef structure that contains
  *         the configuration information for the specified ADC.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  memcpy(&frqd_uBlockBuf[frqd_iWrPtr],&frqd_u32ADCDMABuff[4], 16);
  frqd_iWrPtr++;
  frqd_iWrPtr &= 0x03;
}

