/*
 *  Project:      BeatBassBox
 *  File:         delayline.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Delays all commands to have all synchron
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
#include "delayline.h"
#include "hammer.h"
#include "tmc5160.h"
#include "console.h"

/* Variables -----------------------------------------------------------------*/
uint16_t delayline_u16Time;
DELAYLINE_MoveTo_s delayline_asFifoMoveTo[8];
DELAYLINE_DrumCorrected_s delayline_asFifoDrumCorrected[8];
int delayline_iFifoMoveToWr;
int delayline_iFifoDrumCorrectedWr;
int delayline_iFifoMoveToRd;
int delayline_iFifoDrumCorrectedRd;
int delayLine_iMaxDelay;

/* Local function prototypes -------------------------------------------------*/


/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 */
void DELAYLINE_Init()
{
	delayline_u16Time = 0;
	delayline_iFifoMoveToWr = 0;
	delayline_iFifoDrumCorrectedWr = 0;
	delayline_iFifoMoveToRd = 0;
	delayline_iFifoDrumCorrectedRd = 0;
	delayLine_iMaxDelay = TMC5160_GetMaxDelay();
	CONSOLE_PrintfPrompt("Delay for max distance: %dms", delayLine_iMaxDelay);

}

/**
 * Call this function every 1ms from main
 *
 */
void DELAYLINE_Task1ms()
{

	// Is there an entry in the Fifo?
	if (delayline_iFifoMoveToWr != delayline_iFifoMoveToRd)
	{
		// Time reached?
		if (delayline_asFifoMoveTo[delayline_iFifoMoveToRd].u16TimeStamp == delayline_u16Time)
		{
			// Move it delayed
			TMC5160_MoveTo(delayline_asFifoMoveTo[delayline_iFifoMoveToRd].u16Position);

			// Increment read pointer
			delayline_iFifoMoveToRd++;
			delayline_iFifoMoveToRd &= (8-1);
		}
	}

	// Is there an entry in the Fifo?
	if (delayline_iFifoDrumCorrectedWr != delayline_iFifoDrumCorrectedRd)
	{
		// Time reached?
		if (delayline_asFifoDrumCorrected[delayline_iFifoDrumCorrectedRd].u16TimeStamp == delayline_u16Time)
		{
			// Drum delayed
			HAMMER_DrumCorrected(delayline_asFifoDrumCorrected[delayline_iFifoDrumCorrectedRd].u16Articulation);

			// Increment read pointer
			delayline_iFifoDrumCorrectedRd++;
			delayline_iFifoDrumCorrectedRd &= (8-1);
		}
	}

	delayline_u16Time++;
}

/**
 * Delay the DELAYLINE_TMC5160_MoveTo command
 *
 */
void DELAYLINE_TMC5160_MoveTo(int iDelay_ms, int32_t s32Position)
{
	int iDelay;
	iDelay = DELAYLINE_MAXDELAY-iDelay_ms;
	if (iDelay<0)
	{
		iDelay = 0;
	}

	delayline_asFifoMoveTo[delayline_iFifoMoveToWr].u16TimeStamp = delayline_u16Time + iDelay;
	delayline_asFifoMoveTo[delayline_iFifoMoveToWr].u16Position = (uint16_t)s32Position;
	delayline_iFifoMoveToWr++;
	delayline_iFifoMoveToWr &= (8-1);
}

/**
 * Delay the HAMMER_DrumCorrected command
 *
 */
void DELAYLINE_HAMMER_DrumCorrected(int iDelay_ms, int iArticulation)
{
	int iDelay;
	iDelay = DELAYLINE_MAXDELAY-iDelay_ms;
	if (iDelay<0)
	{
		iDelay = 0;
	}
	delayline_asFifoDrumCorrected[delayline_iFifoDrumCorrectedWr].u16TimeStamp = delayline_u16Time + iDelay;
	delayline_asFifoDrumCorrected[delayline_iFifoDrumCorrectedWr].u16Articulation = (uint16_t)iArticulation;
	delayline_iFifoDrumCorrectedWr++;
	delayline_iFifoDrumCorrectedWr &= (8-1);

}
