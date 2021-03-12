/*
 *  Project:      BeatBassBox
 *  File:         song.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for song.c
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
#ifndef __SONG_H__
#define __SONG_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/
#define SONG_NO_JMP_REF 0xFF
#define SONG_NO_JMP_DEST 0x03FF
/* Types ---------------------------------------------------------------------*/

typedef enum
{
	SONG_J_REPEAT, SONG_J_FINE, SONG_J_VOLTA1, SONG_J_JUMP, SONG_END
} SONG_JumpType_e;

// One entry with jump destinations
typedef __PACKED_STRUCT
{
	uint32_t u10_jumpTo:10;
	uint32_t u10_playUntil:10;
	uint32_t u10_continueAt:10;
	uint32_t :2;
} SONG_JumpToDestinations_s;

// One entry with jump working memory
typedef __PACKED_STRUCT
{
	uint8_t u8Runs;
} SONG_JumpToMemory_s;


// One BBB entry
typedef __PACKED_STRUCT
{
	union
	{
		struct
		{
			uint32_t u1_isJump:1;
			uint32_t u5_Duration:5;
			uint32_t u6_Bass:6;
			uint32_t u1_Articulated:1;
			uint32_t u10_Beat:10;
			uint32_t :9;
		} stBassBeat;

		struct
		{
			uint32_t u1_isJump:1;
			uint32_t u3_JumpType:3;
			uint32_t u1_isSwitch:1;
			uint32_t u1_alFine:1;
			uint32_t u1_withRepeat:1;
			uint32_t u8_jumpToDestinations:8;
			uint32_t u8_jumpToMemory:8;
			uint32_t :9;
		} stJump;

	};
} SONG_Token_s;



/* Global variables ----------------------------------------------------------*/
extern char song_sTitle[];
extern int song_bHasSwitch;
extern int song_bSwitchSet;

/* Function prototypes -------------------------------------------------------*/
void SONG_Init();
void SONG_Task1ms();
void SONG_Select(int iSong);
int SONG_Loaded();
void SONG_Start();
SONG_Token_s SONG_GetNext();




#endif /* __PLAYER_H__ */
