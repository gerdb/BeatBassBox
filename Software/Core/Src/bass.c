/*
 *  Project:      BeatBassBox
 *  File:         bass.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Bass controller
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
#include "tmc5160.h"
#include "bass.h"
#include "errorhandler.h"

/* Variables -----------------------------------------------------------------*/

/* Local function prototypes -------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 */
void BASS_Init()
{
}

/**
 * Call this function every 1ms from main
 *
 */
void BASS_Task1ms()
{
}


/**
 * Play one bass note
 *
 */
void BASS_Play(int iNote, int bIsArticulated)
{
	if (iNote > 0)
	{
		PRINTF_printf("%d ", iNote);
		TMC5160_MoveTo(iNote * 200);
	}
}
