/*
 *  Project:      BeatBassBox
 *  File:         led.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  RGB LED driver
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
#include "led.h"
#include "bass.h"
#include "song.h"

/* Variables -----------------------------------------------------------------*/
int led_iWobbleCntPrescaler = 0;
int led_iWobbleCnt = 0;
int led_bWobbleCntUp = 1;
int led_iPwmCnt = 0;
int led_iErrorCode = 0;
int led_iErrorCntPrescaler = 0;
int led_iErrorCnt = 0;


/* Prototypes of static function ---------------------------------------------*/
static void LED_Color(LED_Colorype_e eColor);
static void LED_ColorM(LED_Colorype_e eColor, int condition);

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 *
 */
void LED_Init()
{
	// All LEDs off
	LED_Color(LED_C_OFF);
}

/**
 * Switch on the LED unconditionally
 *
 */
static void LED_Color(LED_Colorype_e eColor)
{
	LED_ColorM(eColor, 1);
}

/**
 * Switch on the LED and modulate it depending on the condition
 *
 */
static void LED_ColorM(LED_Colorype_e eColor, int condition)
{
	// LD1: green
	// LD2: blue
	// LD3: red

	// Is the condition true?
	if (!condition)
	{
		eColor = LED_C_OFF;
	}

	switch (eColor)
	{
	case LED_C_OFF:
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
		break;
	case LED_C_RED:
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
		break;
	case LED_C_GREEN:
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
		break;
	case LED_C_BLUE:
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
		break;
	}
}

/**
 * Call this function every 1ms from main
 *
 */
void LED_Task1ms()
{

	int bWobble=0;
	static int bError=0;
	LED_Colorype_e eColor;

    // Count up and down between 0 and 100
	led_iWobbleCntPrescaler ++;
	if (led_iWobbleCntPrescaler > 5)
	{
		led_iWobbleCntPrescaler = 0;
		if (led_bWobbleCntUp)
		{
			led_iWobbleCnt++;
			if (led_iWobbleCnt >= 100)
			{
				led_bWobbleCntUp = 0;
			}
		}
		else
		{
			led_iWobbleCnt--;
			if (led_iWobbleCnt <= 0)
			{
				led_bWobbleCntUp = 1;
			}
		}
	}

	// Generate the Wobble PWM
	led_iPwmCnt += led_iWobbleCnt;
	if (led_iPwmCnt >= 100)
	{
		led_iPwmCnt -=100;
		bWobble = 1;
	}

	// Error blinking
	if (led_iErrorCode)
	{
		led_iErrorCntPrescaler ++;
		if (led_iErrorCntPrescaler > 300)
		{
			led_iErrorCntPrescaler = 0;
			led_iErrorCnt++;
			if (led_iErrorCnt > 10)
			{
				led_iErrorCnt = 0;
			}
		}
		bError = (led_iErrorCnt < led_iErrorCode) && (led_iErrorCntPrescaler < 100);
	}
	else
	{
		led_iErrorCntPrescaler = 0;
		led_iErrorCnt = 0;
		bError = 0;
	}

	// is there any error? then this is the most importand state to show
	if (led_iErrorCode)
	{
		LED_ColorM(LED_C_RED, bError);
	}
	else
	{
		// green: song is loaded, blue: no valid song
		if (SONG_Loaded())
		{
			eColor = LED_C_GREEN;
		}
		else
		{
			eColor = LED_C_BLUE;
		}

		// Waiting for reference run
		if (!BASS_IsCalibrated())
		{
			LED_ColorM(eColor, bWobble);
		}
		else
		{
			// default is permanent
			LED_Color(eColor);
		}

	}

}

/**
 * Sets the LEDs
 *
 *
 */
void LED_State(LED_StateType_e eState)
{
	// LD1: green
	// LD2: blue
	// LD3: red

	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
}

