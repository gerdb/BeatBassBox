/*
 *  Project:      BeatBassBox
 *  File:         frqdetect.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for frqdetect.c
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

#ifndef __FRQDETECT_H__
#define __FRQDETECT_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Constants  ----------------------------------------------------------------*/

/* Types ---------------------------------------------------------------------*/

// Union for u16 and u32 access
typedef union
{
	struct
	{
		uint16_t u16_0;
		uint16_t u16_1;
	};
	uint32_t u32;
} FRQDETECT_1632_u;

typedef union
{
	uint16_t u16[8];
	uint32_t u32[4];
} FRQD_blockbuf_t;

/* Function prototypes  ------------------------------------------------------*/
void FRQDETECT_Init();
void FRQDETECT_Task1ms();

#endif /* __FRQDETECT_H__ */
