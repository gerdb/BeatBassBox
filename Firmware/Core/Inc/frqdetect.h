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

#define FRQD_MAXPER 1000 // 16Hz
#define FRQD_SAMPLE_FRQ 16000.0f
#define FRQD_FOVER2PI (FRQD_SAMPLE_FRQ / 6.283185f)
#define FRQD_2PIOVERF (6.283185f / FRQD_SAMPLE_FRQ)
#define FRQD_MAXPERCNT 99999
#define FRQD_THRESH_PULSE 200.0

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
void FRQDETECT_Task100ms();
void FRQDETECT_Start();
float FRQDETECT_GetFrequency();
float FRQDETECT_GetMeanFrequency();
int FRQDETECT_IsValid();
void FRQDETECT_SetDebug(int iDebug);
void FRQDETECT_SetFilter(int iFrq, int iDamp, int isBandpass);
void FRQDETECT_SetDetection(int iMinAmpl, int iEnvThresh, int iEnvDecay);
void FRQDETECT_SetMaxFrq(int iFrq);
void FRQDETECT_PrintFilter();
void FRQDETECT_PrintDetection();
void FRQDETECT_PrintMaxFrq();
int FRQDETECT_PulseDetected();



#endif /* __FRQDETECT_H__ */
