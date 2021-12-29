/*
 *  Project:      BeatBassBox
 *  File:         player.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Song player
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
#include "bass.h"
#include "song.h"
#include "tmc5160.h"
#include "player.h"
#include "adc.h"
#include "errorhandler.h"

/* Variables -----------------------------------------------------------------*/
int player_bButton;
int player_bButtonOld;
int player_bButtonClick;
int player_iPlaying;
int player_iBlinkCnt;
int player_iPeriod;
int player_iTime;
int player_iDelayNext;
int player_iStep;
int player_iSelectedSound = 0;
SONG_Token_s player_stCurrentToken;


/* Local function prototypes -------------------------------------------------*/
//static void PLAYER_StatusLED();
static void PLAYER_Start();
static void PLAYER_Stop();
static void PLAYER_SelectSoundSwitch();
/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 */
void PLAYER_Init()
{
	SONG_Select(player_iSelectedSound);
	player_bButton = 0;
	player_bButtonOld = 0;
	player_bButtonClick = 0;
	player_iPlaying = 0;
	player_iPeriod = 100;
	HAL_ADC_Start(&hadc2);
}

/**
 * Turn on/off the blue LED if the selected song was loaded
 *
 */
/*
static void PLAYER_StatusLED()
{

	player_iBlinkCnt ++;
	// Turn on/off the blue LED
	if (SONG_Loaded() && (!player_bPlaying || (player_iBlinkCnt & 0x0200)))
	{
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	}

	if (player_bPlaying && song_bHasSwitch && !song_bSwitchSet)
	{
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
	}

}
*/



/**
 * Starts a song
 *
 */
void PLAYER_Start()
{
	if (SONG_Loaded())
	{
		CONSOLE_Printf("Start %s", song_sTitle);
		CONSOLE_Prompt();
		BASS_MoveTo(SONG_GetFirstBassNote());
		player_iPeriod = SONG_GetPeriod();
		SONG_Start();
		player_iPlaying = 1000; // Wait 1s
		player_iBlinkCnt = 0;
		player_iTime = 0;
		player_iDelayNext = 0;
		player_iStep = 0;
	}
}

/**
 * Stops a song
 *
 */
void PLAYER_Stop()
{
	player_iPlaying = 0;
	CONSOLE_Printf("Stop");
	CONSOLE_Prompt();
	TMC5160_MoveTo(TMC_POS_HOME);
}




/**
 * Call this function every 1ms from main
 *
 */
void PLAYER_Task1ms()
{
	//PLAYER_StatusLED();


	// Wait a certain time before starting
	if (player_iPlaying > 1)
	{
		player_iPlaying--;
		return;
	}

	// Play the song
	if (player_iPlaying == 1)
	{
		player_iTime++;
		if (player_iTime > player_iPeriod)
		{
			player_iTime = 0;
			player_iStep ++;
			if (player_iStep > player_iDelayNext)
			{
				player_iStep = 0;

				// Get the next token
				player_stCurrentToken = SONG_GetNext();
				if (player_stCurrentToken.stJump.u1_isExtra)
				{
					if (player_stCurrentToken.stJump.u3_ExtraType == EXTRA_JUMP)
					{
						if (player_stCurrentToken.stJump.u3_JumpType == SONG_END)
						{
							PLAYER_Stop();
						}
						player_iDelayNext = 0;
					}

					if (player_stCurrentToken.stTempo.u3_ExtraType == EXTRA_TEMPO)
					{
						player_iPeriod = SONG_GetPeriod();
					}
				}
				else
				{
					BASS_Play(player_stCurrentToken.stBassBeat.u6_Bass,
							player_stCurrentToken.stBassBeat.u1_Articulated);
					player_iDelayNext = player_stCurrentToken.stBassBeat.u6_Duration;
				}
			}
		}
	}
}

/**
 * Read the select sound switch
 *
 */
static void PLAYER_SelectSoundSwitch()
{
	int iSelectedSound;
	static int iLastSelectedSound = 0;
	static int iDebounceCnt = 0;
	iSelectedSound = (HAL_ADC_GetValue(&hadc2) + 409) / 819 + 1;

	// Has it changed?
	if (iSelectedSound != iLastSelectedSound)
	{
		// debounce it 300ms
		if (iDebounceCnt < 3)
		{
			iDebounceCnt ++;
		}
		else
		{
			// Do not switch between sound if playing
			if (!player_iPlaying)
			{
				player_iSelectedSound = iSelectedSound;
				iLastSelectedSound = iSelectedSound;
				iDebounceCnt = 0;
				SONG_Select(player_iSelectedSound);
			}
		}
	}

	// start next
	HAL_ADC_Start(&hadc2);
}

/**
 * Call this function every 100ms from main
 *
 */
void PLAYER_Task100ms()
{
	PLAYER_SelectSoundSwitch();

	player_bButton = HAL_GPIO_ReadPin(USER_Btn_GPIO_Port,USER_Btn_Pin);

	player_bButtonClick = player_bButton && !player_bButtonOld;
	player_bButtonOld = player_bButton;

	// Toggle playing with button
	if (player_bButtonClick)
	{
		if (!BASS_IsCalibrated())
		{
			BASS_StartCalib();
		}
		else if (!player_iPlaying)
		{
			PLAYER_Start();
		}
		else
		{
			if (song_bHasSwitch && !song_bSwitchSet)
			{
				// Set the switch flag if button was pressed while playing
				song_bSwitchSet = 1;
			}
			else
			{
				PLAYER_Stop();
			}
		}

		player_bButtonClick = 0;
	}
}

