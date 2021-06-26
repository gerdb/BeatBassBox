/*
 *  Project:      BeatBassBox
 *  File:         hammer.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Hammer controller
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
#include "printf.h"
#include "console.h"
#include "hammer.h"
#include "errorhandler.h"

/* Variables -----------------------------------------------------------------*/
int ham_iTimer;
int ham_iDuration1;
int ham_iTime2;
int ham_iDuration2;

/* Local function prototypes -------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 */
void HAMMER_Init()
{
	ham_iTimer = 10000;
	ham_iDuration1 = 40;
	ham_iTime2 = 100;
	ham_iDuration2 = 20;
}

/**
 * Call this function every 1ms from main
 *
 */
void HAMMER_Task1ms()
{
	// count 10sec  max.
	if (ham_iTimer< 10000)
	{
		ham_iTimer++;
	}

	// Control the GPIO Pin
	HAL_GPIO_WritePin(HAMMER_BASS_GPIO_Port, HAMMER_BASS_Pin,
			ham_iTimer < ham_iDuration1 ||
			(ham_iTimer > ham_iTime2 && ham_iTimer < (ham_iTime2 + ham_iDuration2))
			);
}


/**
 * One hammer beat
 *
 */
void HAMMER_Drum(void)
{
	ham_iTimer = 0;
	//PRINTF_printf("boom!");
}

/**
 * Get Drum parameters
 *
 */
void HAMMER_ParGet()
{
	PRINTF_printf("0 (ham_iDuration1): %d", ham_iDuration1);
	PRINTF_printf("\r\n1 (ham_iTime2):     %d", ham_iTime2);
	PRINTF_printf("\r\n2 (ham_iDuration2): %d", ham_iDuration2);
}
/**
 * Set Drum parameters
 *
 */
void HAMMER_ParSet(int iParameter, int iVal)
{
	if (iParameter == 0)
	{
		ham_iDuration1 = iVal;
	}
	if (iParameter == 1)
	{
		ham_iTime2 = iVal;
	}
	if (iParameter == 2)
	{
		ham_iDuration2 = iVal;
	}
}
