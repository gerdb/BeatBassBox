/*
 *  Project:      BeatBassBox
 *  File:         frqdetect.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Frequency detection of bass rubber string
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
#include "frqdetect.h"
#include "printf.h"
#include "tim.h"
#include "adc.h"
#include "dac.h"
#include <string.h>

/* Variables -----------------------------------------------------------------*/
uint32_t frqd_u32ADCDMABuff[8];

int frqd_iWrPtr;
int frqd_iRdPtr;
FRQD_blockbuf_t frqd_uBlockBuf[4];
float frqd_fFiltF;
float frqd_fFiltD;
float frqd_fFiltZ1;
float frqd_fFiltZ2;
float frqd_fFiltHPLP;
float frqd_fFiltHP;
float frqd_fEnvPos;
float frqd_fEnvNeg;
float frqd_fEnvThresh;
float frqd_fEnvDecay;
int frqd_iPulse;
int frqd_iPerCnt;
int frqd_iPer;
int frqd_iDebug;
int frqd_iAmpl;
int frqd_iAmplMax;
int frqd_iMinAmpl;

/* Prototypes of static function ---------------------------------------------*/
__STATIC_INLINE void FRQDETECT_Process(uint16_t ui16Sample);


/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 * Starts the ADC DMA
 *
 */
void FRQDETECT_Init()
{
	frqd_iWrPtr = 0;
	frqd_iRdPtr = 0;

	frqd_fFiltF = 500.0f * FRQD_2PIOVERF;
	frqd_fFiltD = 0.5f;

	// Minimal amplitude to measure the frequency
	frqd_iMinAmpl = 100.0f;
	// Envelop threshold
	frqd_fEnvThresh = 0.8f;
	// Envelop decay
	frqd_fEnvDecay = 0.005f;


	// Workaround. See ERRATA. It is necessary to switch on the DAC clock
	__HAL_RCC_DAC_CLK_ENABLE();
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

	HAL_ADC_Start_DMA(&hadc1, frqd_u32ADCDMABuff, 16);
	HAL_TIM_Base_Start(&htim5);
}

/**
 * Returns true if frequency is valid
 *
 * \return true if frequency is valid
 *
 */
int FRQDETECT_IsValid()
{
	return frqd_iPer != 0;
}


/**
 * Returns the current frequency or 0.0f if no frequency is available
 *
 * \return frequency in Hz
 *
 */
float FRQDETECT_GetFrequency()
{
	if (frqd_iPer == 0)
	{
		return 0.0f;
	}

	return FRQD_SAMPLE_FRQ / frqd_iPer;
}

/**
 * Returns the current frequency or 0.0f if no frequency is available
 *
 * \param iDebug 1 to switch on debug mode
 *
 */
void FRQDETECT_SetDebug(int iDebug)
{
	frqd_iDebug = iDebug;
}

/**
 * Sets the frequency and damping of the 2nd order lowpass
 *
 * \param iFrq the frequency in Hz
 * \param iDamp the damping in %
 *
 */
void FRQDETECT_SetFilter(int iFrq, int iDamp)
{
	frqd_fFiltF = ((float)iFrq) * FRQD_2PIOVERF;
	frqd_fFiltD = ((float)iDamp)*0.01f;
}

/**
 * Sets the detection parameters
 *
 * \param iMinAmpl the minimal amplitude to use the detected period
 * \param iEnvThresh the envelope threshold in %
 * \param iEnvDecay the envelope decay in ms
 *
 */
void FRQDETECT_SetDetection(int iMinAmpl, int iEnvThresh, int iEnvDecay)
{
	frqd_iMinAmpl = ((float)iMinAmpl) *4096.0f / 3000.0f;
	frqd_fEnvThresh = ((float)iEnvThresh) / 100.0f;
	if (iEnvDecay != 0)
	{
		frqd_fEnvDecay = 1.0f / (((float)iEnvDecay) / 1000.0f * FRQD_SAMPLE_FRQ);
	}
	else
	{
		frqd_fEnvDecay = 0.0f;
	}
}

/**
 * Print the filter settings
 *
 */
void FRQDETECT_PrintFilter()
{
	PRINTF_printf("%d(Hz), %d(%%)\r\n",
			(int)(frqd_fFiltF * FRQD_FOVER2PI),
			(int)(frqd_fFiltD * 100.0f)
			);
}
/**
 * Print the detection settings
 *
 */
void FRQDETECT_PrintDetection()
{
	PRINTF_printf("%d(mV), %d(%%), %d(ms)\r\n",
			(int)(frqd_iMinAmpl * 3000.0f / 4096.0f),
			(int)(frqd_fEnvThresh * 100.0f),
			(int)(1000.0f / FRQD_SAMPLE_FRQ / frqd_fEnvDecay)
			);
}



/**
 * Processes on sample
 *
 * \param ui16Sample 16 (12-bit) ADC value from 0..0x0FFF
 *
 */

__STATIC_INLINE void FRQDETECT_Process(uint16_t ui16Sample)
{
	// High pass to remove DC
	frqd_fFiltHP = ui16Sample - frqd_fFiltHPLP;
	frqd_fFiltHPLP += frqd_fFiltHP * 0.0002f;

	// Lowpass (bandpass)
	frqd_fFiltZ2 += frqd_fFiltF * frqd_fFiltZ1;
	frqd_fFiltZ1 += frqd_fFiltF * (frqd_fFiltHP - frqd_fFiltZ1 * frqd_fFiltD - frqd_fFiltZ2);


	// Positive envelope
	if (frqd_fFiltZ2 > frqd_fEnvPos)
	{
		frqd_fEnvPos = frqd_fFiltZ2;
		frqd_iAmpl = frqd_fEnvPos - frqd_fEnvNeg;
	}
	else
	{
		frqd_fEnvPos -= frqd_fEnvPos * frqd_fEnvDecay;
	}

	// Negative envelope
	if (frqd_fFiltZ2 < frqd_fEnvNeg)
	{
		frqd_fEnvNeg = frqd_fFiltZ2;
		frqd_iAmpl = frqd_fEnvPos - frqd_fEnvNeg;
	}
	else
	{
		frqd_fEnvNeg -= frqd_fEnvNeg * frqd_fEnvDecay;
	}

	// Schmitttrigger
	if (frqd_fFiltZ2 > frqd_fEnvPos * frqd_fEnvThresh)
	{
		// Rising edge
		if (frqd_iPulse == 0)
		{
			// Count the period
			if (frqd_iPerCnt < FRQD_MAXPER)
			{
				// Amplitude large enough?
				if (frqd_iAmpl > frqd_iMinAmpl)
				{
					frqd_iPer = frqd_iPerCnt;
				}
				else
				{
					frqd_iPer = 0;
				}
			}
			frqd_iPerCnt = 0;
		}
		frqd_iPulse = 1;
	}
	if (frqd_fFiltZ2 < frqd_fEnvNeg * frqd_fEnvThresh)
	{
		frqd_iPulse = 0;
	}

	// Limit to 16Hz
	if (frqd_iPerCnt < FRQD_MAXPER)
	{
		frqd_iPerCnt++;
	}
	else
	{
		frqd_iPer = 0;
	}


	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, frqd_iPer * 16);
}

/**
 * Call this function every 1ms from main
 *
 * It processes on (or more) blocks
 *
 */
void FRQDETECT_Task1ms()
{
	while (frqd_iRdPtr != frqd_iWrPtr)
	{
		for (int i=0;i<8;i++)
		{
			FRQDETECT_Process(frqd_uBlockBuf[frqd_iRdPtr].u16[i]);
		}
		frqd_iRdPtr++;
		frqd_iRdPtr &= 0x03;
	}

	if (frqd_iDebug)
	{
		// Get the peak value of the amplitude
		if (frqd_iAmpl > frqd_iAmplMax)
		{
			frqd_iAmplMax = frqd_iAmpl;
		}
	}

}

/**
 * Call this function every 1ms from main
 *
 * It processes on (or more) blocks
 *
 */
void FRQDETECT_Task100ms()
{
	if (frqd_iDebug)
	{
		if (FRQDETECT_IsValid())
		{
			PRINTF_printf("%dHz %d\r\n",(int)FRQDETECT_GetFrequency(),(100 * frqd_iAmplMax)/4096);
			frqd_iAmplMax = 0;
		}
	}
}

/**
  * @brief  Regular conversion half DMA transfer callback in non blocking mode
  * @param  hadc: pointer to a ADC_HandleTypeDef structure that contains
  *         the configuration information for the specified ADC.
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  memcpy(&frqd_uBlockBuf[frqd_iWrPtr],&frqd_u32ADCDMABuff[0], 16);
  frqd_iWrPtr++;
  frqd_iWrPtr &= 0x03;
}

/**
  * @brief  Regular conversion complete callback in non blocking mode
  * @param  hadc: pointer to a ADC_HandleTypeDef structure that contains
  *         the configuration information for the specified ADC.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  memcpy(&frqd_uBlockBuf[frqd_iWrPtr],&frqd_u32ADCDMABuff[4], 16);
  frqd_iWrPtr++;
  frqd_iWrPtr &= 0x03;
}

