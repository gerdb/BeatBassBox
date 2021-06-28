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

/* Variables -----------------------------------------------------------------*/
BASS_Calib_e bass_eCalib;
int bass_iCalibCnt;
int bass_iCalibPos;
float bass_fFrq;

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
		APPROX_Init();
		bass_iCalibPos = 18000;
		TMC5160_MoveTo(bass_iCalibPos);
		bass_iCalibCnt = 0;
		bass_eCalib = CALIB_START_WAIT_REACHED;
		break;
	case CALIB_START_WAIT_REACHED:
		bass_iCalibCnt ++;
		if (bass_iCalibCnt > 1000)
		{
			bass_iCalibCnt = 0;
			FRQDETECT_SetFilter(500, 50, 0);
			FRQDETECT_SetMaxFrq(500);
			HAMMER_ParSet(1,70);
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
			HAMMER_ParSet(1,80-(bass_iCalibPos-8000)/1000);
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
		PRINTF_printf("%d ", iNote);
		TMC5160_MoveTo(iNote * 200);
	}
}
