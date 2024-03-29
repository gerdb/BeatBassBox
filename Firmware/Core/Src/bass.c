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
#include "console.h"
#include "tmc5160.h"
#include "hammer.h"
#include "tmc5160.h"
#include "frqdetect.h"
#include "approximation.h"
#include "bass.h"
#include "errorhandler.h"
#include <math.h>
#include "delayline.h"

/* Variables -----------------------------------------------------------------*/
BASS_Calib_e bass_eCalib;
int bass_iCalibCnt;
int bass_iCalibPos;
int bass_iCalibRetries;
float bass_fFrq;
float bass_fFrqLastValid;
float bass_fNoteFrqs[64];
int bass_bIsCalibrated;
int bass_iMeasFrqCnt;
int bass_iMeasDelay;
int bass_iPos;

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
	bass_bIsCalibrated = 0;
	bass_iMeasFrqCnt = 0;
	bass_iPos = 0;

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
	if (bass_iMeasFrqCnt > 0)
	{
		bass_iMeasFrqCnt--;
		if (bass_iMeasFrqCnt == 0)
		{
			float fFrq = FRQDETECT_GetMeanFrequency();
			CONSOLE_PrintfLn("frq: %d.%03dHz", (int)fFrq,(int)((fFrq-(int)fFrq)*1000) );
			CONSOLE_PrintfLn("diff: %dcent", (int)(100.0f*((fFrq/bass_fFrq-1.0f)/0.05946309f)));
		}
	}

	switch (bass_eCalib)
	{
	case CALIB_NO:
		break;
	case CALIB_START:
		ERRORHANDLER_ResetError(ERROR_DURING_CALIB);
		TMC5160_Ref();
		bass_eCalib = CALIB_WAIT_REF;
		break;
	case CALIB_WAIT_REF:
		if (!TMC5160_IsReferencing())
		{
			APPROX_Init();
			bass_iCalibPos = TMC_POS_CALIBMAX;
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
			HAMMER_DrumCorrected(0);
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
				bass_fFrqLastValid = bass_fFrq;
				CONSOLE_PrintfLn("Frequency: %dHz",
							(int)(bass_fFrq)
							);
				bass_iCalibRetries = 0;
				bass_eCalib = CALIB_SINGLE;
			}
			else
			{
				ERRORHANDLER_SetErrorText("No proper signal", ERROR_DURING_CALIB);
				bass_eCalib = CALIB_NO;
			}
		}
		break;
	case CALIB_SINGLE:
		bass_iCalibCnt = 0;

		if (bass_iCalibPos >= TMC_POS_CALIBMIN)
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
			FRQDETECT_SetFilter((int)bass_fFrqLastValid, 50, 1);
			FRQDETECT_SetMaxFrq((int)(bass_fFrqLastValid*1.5f));
			FRQDETECT_Start();
			bass_iMeasDelay = 0;
			HAMMER_DrumCorrected(0);
			bass_eCalib = CALIB_SINGLE_WAIT;
		}
		break;
	case CALIB_SINGLE_WAIT:
		bass_iCalibCnt ++;
		if (bass_iMeasDelay == 0 && FRQDETECT_PulseDetected())
		{
			bass_iMeasDelay = bass_iCalibCnt;
		}
		if (bass_iCalibCnt > 500)
		{
			bass_fFrq = FRQDETECT_GetMeanFrequency();
			if (bass_fFrq > 16.0f)
			{
				bass_fFrqLastValid = bass_fFrq;
				APPROX_Point(bass_iCalibPos, bass_fFrq);
				CONSOLE_PrintfLn("%d,%d,%d",
							bass_iCalibPos,
							(int)(bass_fFrq),
							bass_iMeasDelay
							);
				bass_iCalibPos -= 200;
				bass_iCalibRetries = 0;
				bass_eCalib = CALIB_SINGLE;
			}
			else
			{
				bass_iCalibRetries ++;
				if (bass_iCalibRetries>5)
				{
					ERRORHANDLER_SetErrorText("No proper signal", ERROR_DURING_CALIB);
					bass_eCalib = CALIB_NO;
				}
				else
				{
					bass_eCalib = CALIB_SINGLE;
				}
			}
		}
		break;
	case CALIB_FINISH:
		APPROX_Approximate();
		APPROX_Calc(100.0f);
		FRQDETECT_SetFilter(500, 50, 0);
		FRQDETECT_SetMaxFrq(500);
		TMC5160_MoveTo(TMC_POS_HOME);
		CONSOLE_PrintfPrompt("Calibration successful");
		bass_bIsCalibrated = 1;
		bass_eCalib = CALIB_NO;
		break;
	default:
		bass_eCalib = CALIB_NO;
	}
}

/**
 * Is the bass calibrated?
 *
 */
int BASS_IsCalibrated()
{
	return bass_bIsCalibrated;
}

/**
 * Start a calibration
 *
 */
void BASS_StartCalib()
{
	bass_eCalib = CALIB_START;
	bass_bIsCalibrated = 0;
}

/**
 * Move only the arm
 *
 */
int BASS_MoveTo(int iNote)
{
	if (iNote > 0)
	{
		int iPos = APPROX_Calc(bass_fNoteFrqs[iNote]);
		if (iPos > TMC_POS_MIN &&  iPos < TMC_POS_MAX)
		{
			//TMC5160_MoveTo(iPos);
			// Distance to move
			DELAYLINE_TMC5160_MoveTo(TMC5160_CalcDelay(iPos - bass_iPos),iPos);
			bass_iPos = iPos;
			return 1;
		}
		else
		{
			CONSOLE_Printf("Position out of range");
		}
	}
	return 0;
}


/**
 * Play one bass note
 *
 */
void BASS_Play(int iNote, int bIsArticulated)
{
	CONSOLE_Printf("%d ", iNote);
	if (BASS_MoveTo(iNote))
	{
		//HAMMER_DrumCorrected();
		// Use a fix delay of 25ms for hammer
		DELAYLINE_HAMMER_DrumCorrected(25, bIsArticulated);
	}
}

/**
 * Play one bass note and measure the frequency
 *
 */
void BASS_Test(int iNote)
{
	bass_fFrq = bass_fNoteFrqs[iNote];
	CONSOLE_PrintfLn("frq: %d.%03dHz", (int)bass_fFrq,(int)((bass_fFrq-(int)bass_fFrq)*1000) );
	int iPos = APPROX_Calc(bass_fFrq);
	CONSOLE_PrintfLn("pos: %d", iPos );
	FRQDETECT_SetFilter((int)bass_fFrq, 50, 1);
	FRQDETECT_SetMaxFrq((int)(bass_fFrq*1.5f));
	FRQDETECT_Start();
	bass_iMeasFrqCnt = 500;

	if (BASS_MoveTo(iNote))
	{
		HAMMER_DrumCorrected(0);
	}
}
