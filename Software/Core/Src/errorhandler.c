/*
 *  Project:      BeatBassBox
 *  File:         errorhandler.c
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
#include "errorhandler.h"

/* Variables -----------------------------------------------------------------*/
uint32_t u32ErrorCode;

/* Prototypes of static function ---------------------------------------------*/
static void ERRORHANDLER_Leds();


/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 *
 */
void ERRORHANDLER_Init()
{
	u32ErrorCode = ERROR_NO_ERROR;
	ERRORHANDLER_Leds();
}

/**
 * Sets the LEDs
 *
 *
 */
static void ERRORHANDLER_Leds()
{
	// LD1: green
	// LD3: red
	/*
	if (u32ErrorCode == ERROR_NO_ERROR)
	{
		  HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
	}
	else
	{
		  HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_RESET);
	}
	*/
}


/**
 * Call this function every 1ms from main
 *
 *
 */
void ERRORHANDLER_SetError(uint32_t u32ErrCode)
{
	u32ErrorCode |= u32ErrCode;
	ERRORHANDLER_Leds();
}

/**
 * Call this function every 1ms from main
 *
 *
 */
void ERRORHANDLER_ResetError(uint32_t u32ErrCode)
{
	u32ErrorCode &= ~u32ErrCode;
	ERRORHANDLER_Leds();
}

