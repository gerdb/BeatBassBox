/*
 *  Project:      BeatBassBox
 *  File:         delayline.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for delayline.c
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
#ifndef __DELAYLINE_H__
#define __DELAYLINE_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/

/* Types ---------------------------------------------------------------------*/

// One entry of the delayline for the TMC5160_MoveTo command
typedef __PACKED_STRUCT
{
	uint16_t u16TimeStamp;
	uint16_t u16Position;
} DELAYLINE_MoveTo_s;

// One entry of the delayline for the DrumCorrected command
typedef __PACKED_STRUCT
{
	uint16_t u16TimeStamp;
	uint16_t u16Articulation;
} DELAYLINE_DrumCorrected_s;

/* Global variables ----------------------------------------------------------*/


/* Function prototypes -------------------------------------------------------*/
void DELAYLINE_Init();
void DELAYLINE_Task1ms();
void DELAYLINE_TMC5160_MoveTo(int iDelay_ms, int32_t s32Position);
void DELAYLINE_HAMMER_DrumCorrected(int iDelay_ms, int iArticulation);

#endif /* __DELAYLINE_H__ */
