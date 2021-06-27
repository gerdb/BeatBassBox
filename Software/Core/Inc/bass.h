/*
 *  Project:      BeatBassBox
 *  File:         bass.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for bass.c
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
#ifndef __BASS_H__
#define __BASS_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/

/* Types -------------------------------------------------------------------*/
typedef enum
{
	CALIB_NO,
	CALIB_START,
	CALIB_START_WAIT_REACHED,
	CALIB_WAIT_FIRST,
	CALIB_SINGLE,
	CALIB_SINGLE_WAIT_REACHED,
	CALIB_SINGLE_WAIT,
	CALIB_FINISH
} BASS_Calib_e;


/* Global variables ----------------------------------------------------------*/


/* Function prototypes -------------------------------------------------------*/
void BASS_Init();
void BASS_Task1ms();
void BASS_Play(int iNote, int bIsArticulated);
void BASS_StartCalib();

#endif /* __BASS_H__ */
