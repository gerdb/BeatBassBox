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
#include "hammer.h"
#include "tmc5160.h"
#include "frqdetect.h"
#include "approximation.h"
#include "bass.h"
#include "errorhandler.h"
#include <math.h>

/* Variables -----------------------------------------------------------------*/
BASS_Calib_e bass_eCalib;
int bass_iCalibCnt;
int bass_iCalibPos;
float bass_fFrq;
float bass_fNoteFrqs[64];

/* Local function prototypes -------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 */
void BASS_Init()
{
	bass_eCalib = CALIB_NO;

	int index;
	// Fill the notes with its frequency
	float fFrq = 0.0f;
	for (int i=0; i <128; i++)
	{
		// is it an A?
		if (((i-21) % 12) == 0)
		{
			int iOctave = (i-21) / 12 - 4;
			fFrq = 440.0f * powf(2.f,(float)iOctave);
		}
		else
		{
			// frequency of chromatic scale
			fFrq *= powf(2.f,1.f/12.f);
		}
		// Offset of -20 see also SONG_DecodeLine(..)
		index = i-20;
		if (index >= 0 && index <64)
		{
			bass_fNoteFrqs[index] = fFrq;
		}
	}
}

/**
 * Call this function every 1ms from main
 *
 */
void BASS_Task1ms()
{
	switch (bass_eCalib)
	{
	case CALIB_NO:
		break;
	case CALIB_START:
		TMC5160_Ref();
		bass_eCalib = CALIB_WAIT_REF;
		break;
	case CALIB_WAIT_REF:
		if (!TMC5160_IsReferencing())
		{
			APPROX_Init();
			bass_iCalibPos = 18000;
			TMC5160_MoveTo(bass_iCalibPos);
			bass_iCalibCnt = 0;
			bass_eCalib = CALIB_START_WAIT_REACHED;
		}
		break;
	case CALIB_START_WAIT_REACHED:
		bass_iCalibCnt ++;
		if (bass_iCalibCnt > 1000)
		{
			bass_iCalibCnt = 0;
			FRQDETECT_SetFilter(500, 50, 0);
			FRQDETECT_SetMaxFrq(500);
			FRQDETECT_Start();
			HAMMER_Drum();
			bass_eCalib = CALIB_WAIT_FIRST;
		}
		break;
	case CALIB_WAIT_FIRST:
		bass_iCalibCnt ++;
		if (bass_iCalibCnt > 1000)
		{
			bass_fFrq = FRQDETECT_GetMeanFrequency();
			if (bass_fFrq > 16.0f)
			{
				PRINTF_printf("Frequency: %dHz\r\n",
							(int)(bass_fFrq)
							);
				bass_eCalib = CALIB_SINGLE;
			}
			else
			{
				PRINTF_printf("No propper signal\r\n");
				bass_eCalib = CALIB_NO;
			}
		}
		break;
	case CALIB_SINGLE:
		bass_iCalibCnt = 0;
		if (bass_iCalibPos > 8000)
		{
			TMC5160_MoveTo(bass_iCalibPos);

			bass_eCalib = CALIB_SINGLE_WAIT_REACHED;
		}
		else
		{
			bass_eCalib = CALIB_FINISH;
		}
		break;
	case CALIB_SINGLE_WAIT_REACHED:
		bass_iCalibCnt ++;
		if (bass_iCalibCnt > 250)
		{
			bass_iCalibCnt = 0;
			FRQDETECT_SetFilter((int)bass_fFrq, 50, 1);
			FRQDETECT_SetMaxFrq((int)(bass_fFrq*1.5f));
			FRQDETECT_Start();
			HAMMER_Drum();
			bass_eCalib = CALIB_SINGLE_WAIT;
		}
		break;
	case CALIB_SINGLE_WAIT:
		bass_iCalibCnt ++;
		if (bass_iCalibCnt > 500)
		{
			bass_fFrq = FRQDETECT_GetMeanFrequency();
			if (bass_fFrq > 16.0f)
			{
				APPROX_Point(bass_iCalibPos, bass_fFrq);
				PRINTF_printf("%d,%d\r\n",
							bass_iCalibPos,
							(int)(bass_fFrq)
							);
				bass_iCalibPos -= 200;
				bass_eCalib = CALIB_SINGLE;
			}
			else
			{
				PRINTF_printf("No propper signal\r\n");
				bass_eCalib = CALIB_NO;
			}
		}
		break;
	case CALIB_FINISH:
		APPROX_Approximate();
		APPROX_Calc(100.0f);
		FRQDETECT_SetFilter(500, 50, 0);
		FRQDETECT_SetMaxFrq(500);
		TMC5160_MoveTo(6000);
		bass_eCalib = CALIB_NO;
		break;
	default:
		bass_eCalib = CALIB_NO;
	}
}

/**
 * Start a calibration
 *
 */
void BASS_StartCalib()
{
	bass_eCalib = CALIB_START;
}

/**
 * Play one bass note
 *
 */
void BASS_Play(int iNote, int bIsArticulated)
{
	if (iNote > 0)
	{
		int iPos = APPROX_Calc(bass_fNoteFrqs[iNote]);
		if (iPos > 6000 &&  iPos < 20000)
		{
			PRINTF_printf("%d ", iNote);
			TMC5160_MoveTo(iPos);
			HAMMER_Drum();
		}
		else
		{
			PRINTF_printf("Position out of range");
		}
	}
}
