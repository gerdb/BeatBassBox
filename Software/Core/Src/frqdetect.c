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

/* Variables -----------------------------------------------------------------*/
FRQDETECT_1632_u frqd_u1632ADCValues[16] = {0};

/* Prototypes of static function ---------------------------------------------*/


/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 * Starts the ADC DMA
 *
 */
void FRQDETECT_Init()
{
	// Workaround. See ERRATA. It is necessary to switch on the DAC clock
	__HAL_RCC_DAC_CLK_ENABLE();
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) frqd_u1632ADCValues, 32);
	HAL_TIM_Base_Start(&htim5);
}

/**
 * Call this function every 1ms from main
 *
 * It starts the creation of the action tabled triggered from the ISR
 *
 */
void FRQDETECT_Task1ms()
{
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
  //int i;
  frqd_u1632ADCValues[0].u16_0++;
//  for (i=0; i< 160; i++)
//	  MEASUREMENT_NewSample((int)ADCConvertedValue[i]);
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
  UNUSED(hadc);
  frqd_u1632ADCValues[0].u16_0++;
//  for (i=160; i< 320; i++)
//	  MEASUREMENT_NewSample((int)ADCConvertedValue[i]);

}

