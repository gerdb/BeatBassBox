/*
 *  Project:      BeatBassBox
 *  File:         song.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Contains the song data and methos to load it from SUB stick
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
#include <stdio.h>
#include "fatfs.h"
#include "printf.h"
#include "console.h"
#include "usbstick.h"
#include "errorhandler.h"
#include "song.h"

/* Variables -----------------------------------------------------------------*/
int song_iSongSelected;
int song_bSelectedChanged;
int song_iSongLoaded;
int song_bSongLoaded;

int song_iBBBFileVersion = 0;
char song_sTitle[40] = "No Song loaded";
int song_iTempo = 120;

SONG_JumpToDestinations_s song_stJDestinations[256];
SONG_JumpToMemory_s song_stJMemory[256];
SONG_Token_s song_stTokens[1024];
int song_iJDestinationLen;
int song_iJMemoryLen;
int song_iTokenLen;

/* Local function prototypes -------------------------------------------------*/
static void SONG_StatusLED();
static int SONG_Load(int iSong);
static int SONG_DecodeLine(char* sLine);
static int SONG_DecodeVersion(char* sLine);
static int SONG_DecodeTitle(char* sLine);
static int SONG_DecodeTempo(char* sLine);
static int SONG_DecodeStart(char* sLine);
static int SONG_IsEndOfLineChar(char c);
static int SONG_GetNumber(char* s, int pos, int len);
static int SONG_DecodeError(char* sErrorText);
static int SONG_DecodeCheckSign(char* sLine, int pos, char c);

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 */
void SONG_Init()
{
	song_iSongLoaded = -1;
	song_iSongSelected = -1;
	song_bSelectedChanged = 0;
	song_bSongLoaded = 0;
	song_iJDestinationLen = 0;
	song_iJMemoryLen = 0;
	song_iTokenLen = 0;
}


/**
 * Call this function every 1ms from main
 *
 */
void SONG_Task1ms()
{
	int bUSBPluggedIn;

	song_bSongLoaded = (song_iSongLoaded == song_iSongSelected) && song_iSongLoaded >= 0;
	SONG_StatusLED();

	bUSBPluggedIn = USBSTICK_PluggedInEvent();

	// Is the seleced song loaded, we do nothing
	if (!song_bSongLoaded)
	{
		// Someone wants a new song
		if ((song_bSelectedChanged || bUSBPluggedIn)
				&& ustick_bMounted)
		{
			song_bSelectedChanged = 0;
			song_iSongLoaded = -1;
			// Load the song
			if (SONG_Load(song_iSongSelected) == 0)
			{
				// Everything was ok
				song_iSongLoaded = song_iSongSelected;
			}
		}
	}
}

/**
 * Decodes the BBB header with version
 *
 * \param sLine: the line
 * \return 0 if there is no error
 */
static int SONG_DecodeVersion(char* sLine)
{
	if (strncmp(sLine, "BBB:", 4) != 0)
	{
		PRINTF_printf("BBB file must start with \"BBB:\"");
		CONSOLE_Prompt();
		return 1;
	}

	song_iBBBFileVersion = sLine[4] - '0';
	if (song_iBBBFileVersion != 1)
	{
		PRINTF_printf("Wrong BBB file version. Must be 1 is:%d", song_iBBBFileVersion);
		CONSOLE_Prompt();
		return 1;
	}
	return 0;
}

/**
 * Decodes the song title
 *
 * \param sLine: the line
 * \return 0 if there is no error
 */
static int SONG_DecodeTitle(char* sLine)
{
	if (strncmp(sLine, "TITLE:", 6) != 0)
	{
		PRINTF_printf("2. line must contain title");
		CONSOLE_Prompt();
		return 1;
	}

	// Copy the title
	strncpy(song_sTitle, &sLine[6], sizeof(song_sTitle)-1);
	return 0;
}


/**
 * Decodes tempo
 *
 * \param sLine: the line
 * \return 0 if there is no error
 */
static int SONG_DecodeTempo(char* sLine)
{
	if (strncmp(sLine, "TEMPO:", 6) != 0)
	{
		PRINTF_printf("2. line must contain the song tempo");
		CONSOLE_Prompt();
		return 1;
	}

	song_iTempo = 0;
	for (int i=0;i<3;i++)
	{
		song_iTempo*=10;
		song_iTempo+=sLine[6+i]-'0';
	}

	if (song_iTempo < 30 || song_iTempo > 200)
	{
		PRINTF_printf("Song tempo must be in the range of 30..200bpm");
		CONSOLE_Prompt();
		return 1;
	}
	return 0;
}

/**
 * Decodes the start tag
 *
 * \param sLine: the line
 * \return 0 if there is no error
 */
static int SONG_DecodeStart(char* sLine)
{
	if (strncmp(sLine, "START", 5) != 0)
	{
		PRINTF_printf("Line 4 of BBB file must be \"START:\"");
		CONSOLE_Prompt();
		return 1;
	}

	song_iTokenLen = 0;
	return 0;
}

/**
 * return true, if end of line is found
 */
static int SONG_IsEndOfLineChar(char c)
{
	return c== '\0' || c== '\r' || c== '\n';
}

/**
 * rdecodes the number in a string
 */
static int SONG_GetNumber(char* s, int pos, int len)
{
	int val = 0;
	char c;

	for (int i=0;i<len;i++)
	{
		val *= 10;
		c = s[pos + i];
		if (c == ' ')
		{
			// spaces are like 0s
		}
		else if (c >= '0' && c <= '9')
		{
			val += c- '0';
		}
		else
		{
		    // BBB file not found
			PRINTF_printf("Invalid character at position %d of %s", (pos+i), s);
			CONSOLE_Prompt();
		}
	}
	return val;
}

/**
 * Outputs an error text
 *
 * \param sErrorText: the error text
 * \return 1 = error
 */
static int SONG_DecodeError(char* sErrorText)
{
	PRINTF_printf(sErrorText);
	CONSOLE_Prompt();
	return -1;
}

/**
 * Outputs an error text
 *
 * \param sLine: the line
 * \param pos: position of the expected sign
 * \param c: expected sign
 * \return 1 = error
 */
static int SONG_DecodeCheckSign(char* sLine, int pos, char c)
{
	if (sLine[pos] != c)
	{
		PRINTF_printf("There must be a '%c' at position %d",c , pos);
		CONSOLE_Prompt();
		return -1;
	}
	return 0;
}



/**
 * Decodes a BBB line
 *
 * \param sLine: the line
 * \return 0 if there is no error
 */
static int SONG_DecodeLine(char* sLine)
{
	char c;
	int iTokenNr = 0;
	int iDuration;
	int iBass;
	int iBeat;
	int bIsArticulated;
	int bIsSwitch;
	int iJumpTo;
	int iPlayUntil;
	int iContinueAt;
	int bAlFine;
	int bWithRepeat;


	// Get Token number
	iTokenNr = SONG_GetNumber(sLine, 0, 3);
	// Check syntax
	if (iTokenNr != song_iTokenLen)
	{
		return (SONG_DecodeError("Invalid token number"));
	}

	// Check syntax
	if (SONG_DecodeCheckSign(sLine, 3, ':') == -1) return -1;

	// Decode the command
	if (sLine[5]== 'B' && sLine[6]== 'B' && sLine[7]== ':')
	{
		// it was a BB command
		iDuration = SONG_GetNumber(sLine, 8, 2);
		// Check range
		if (iDuration<0 || iDuration>31)
		{
			return (SONG_DecodeError("Duration must be between 0 and 31"));
		}
		// Check syntax
		if (SONG_DecodeCheckSign(sLine, 10, ':') == -1) return -1;

		// Get the bass note
		iBass = SONG_GetNumber(sLine, 11, 2);
		if (iBass != 0)
		{
			// Offset 20
			iBass-= 20;

			// Check range
			if (iBass<1 || iBass>63)
			{
				return (SONG_DecodeError("Bass note out of range"));
			}
		}

		// Decode the articulation sign
		if (sLine[13]== '>')
		{
			bIsArticulated = 1;
		}
		else if (sLine[13]== ' ')
		{
			bIsArticulated = 0;
		}
		else
		{
			return (SONG_DecodeError("Invalid articulation sign"));
		}

		// Check syntax
		if (SONG_DecodeCheckSign(sLine, 14, ':') == -1) return -1;

		// Fill the beat structure digital 0/1
		iBeat = 0;
		// 10 entries. LSB is left, so we count down
		for (int i=9;i>=0;i--)
		{
			iBeat <<= 1;
			c = sLine[15+i];
			// syntax check
			if (SONG_IsEndOfLineChar(c))
			{
				return (SONG_DecodeError("End of line"));
			}
			if (c != ' ')
			{
				iBeat |= 1;
			}
		}

		// It's a bass-beat chord an not a jump
		song_stTokens[song_iTokenLen].stBassBeat.u1_isJump = 0;
		song_stTokens[song_iTokenLen].stBassBeat.u5_Duration = iDuration;
		song_stTokens[song_iTokenLen].stBassBeat.u6_Bass = iBass;
		song_stTokens[song_iTokenLen].stBassBeat.u1_Articulated = bIsArticulated;
		song_stTokens[song_iTokenLen].stBassBeat.u10_Beat = iBeat;
	}
	else if (sLine[5]== ']' && sLine[7]== ':')
	{
		// Decode the switch sign
		if (sLine[6]== 'S')
		{
			bIsSwitch = 1;
		}
		else if (sLine[6]== ' ')
		{
			bIsSwitch = 0;
		}
		else
		{
			return (SONG_DecodeError("Invalid switch sign"));
		}

		// Check syntax
		if (SONG_DecodeCheckSign(sLine, 7, ':') == -1) return -1;

		// Get jump address
		iJumpTo = SONG_GetNumber(sLine, 8, 3);

		// it was a single repeat
		song_stTokens[song_iTokenLen].stJump.u1_isJump = 1;
		song_stTokens[song_iTokenLen].stJump.u3_JumpType = SONG_J_REPEAT;
		song_stTokens[song_iTokenLen].stJump.u1_isSwitch = bIsSwitch;
		song_stTokens[song_iTokenLen].stJump.u1_alFine = 0;
		song_stTokens[song_iTokenLen].stJump.u1_withRepeat = 0;

		song_stJDestinations[song_iJDestinationLen].u10_jumpTo = iJumpTo;
		song_stJDestinations[song_iJDestinationLen].u10_playUntil = SONG_NO_JMP_DEST;
		song_stJDestinations[song_iJDestinationLen].u10_continueAt = SONG_NO_JMP_DEST;
		song_stTokens[song_iTokenLen].stJump.u8_jumpToDestinations = song_iJDestinationLen;
		song_iJDestinationLen++;

		song_stTokens[song_iTokenLen].stJump.u8_jumpToMemory = song_iJMemoryLen;
		song_iJMemoryLen ++;
	}
	else if (sLine[5]== 'F' && sLine[6]== 'I')
	{
		// it was a fine
		song_stTokens[song_iTokenLen].stJump.u1_isJump = 1;
		song_stTokens[song_iTokenLen].stJump.u3_JumpType = SONG_J_FINE;
		song_stTokens[song_iTokenLen].stJump.u1_isSwitch = 0;
		song_stTokens[song_iTokenLen].stJump.u1_alFine = 0;
		song_stTokens[song_iTokenLen].stJump.u1_withRepeat = 0;

		song_stJDestinations[song_iJDestinationLen].u10_jumpTo = SONG_NO_JMP_DEST;
		song_stJDestinations[song_iJDestinationLen].u10_playUntil = SONG_NO_JMP_DEST;
		song_stJDestinations[song_iJDestinationLen].u10_continueAt = SONG_NO_JMP_DEST;
		song_stTokens[song_iTokenLen].stJump.u8_jumpToDestinations = SONG_NO_JMP_REF;
		song_stTokens[song_iTokenLen].stJump.u8_jumpToMemory = SONG_NO_JMP_REF;
	}
	else if (sLine[5]== 'V' && sLine[6]== '1')
	{
		// it was a volta

		// Check syntax
		if (SONG_DecodeCheckSign(sLine, 7, ':') == -1) return -1;

		// Get jump address
		iJumpTo = SONG_GetNumber(sLine, 8, 3);

		// it was a volta
		song_stTokens[song_iTokenLen].stJump.u1_isJump = 1;
		song_stTokens[song_iTokenLen].stJump.u3_JumpType = SONG_J_VOLTA1;
		song_stTokens[song_iTokenLen].stJump.u1_isSwitch = 0;
		song_stTokens[song_iTokenLen].stJump.u1_alFine = 0;
		song_stTokens[song_iTokenLen].stJump.u1_withRepeat = 0;

		song_stJDestinations[song_iJDestinationLen].u10_jumpTo = iJumpTo;
		song_stJDestinations[song_iJDestinationLen].u10_playUntil = SONG_NO_JMP_DEST;
		song_stJDestinations[song_iJDestinationLen].u10_continueAt = SONG_NO_JMP_DEST;
		song_stTokens[song_iTokenLen].stJump.u8_jumpToDestinations = song_iJDestinationLen;
		song_iJDestinationLen++;

		song_stTokens[song_iTokenLen].stJump.u8_jumpToMemory = song_iJMemoryLen;
		song_iJMemoryLen ++;

	}
	else if (sLine[5]== 'J' && sLine[8]== ':')
	{
		// it was a jump
		// Check syntax
		if (SONG_DecodeCheckSign(sLine, 8, ':') == -1) return -1;
		if (SONG_DecodeCheckSign(sLine, 12, '>') == -1) return -1;
		if (SONG_DecodeCheckSign(sLine, 16, '>') == -1) return -1;

		// Decode the jump repeat sign
		if (sLine[6]== 'R')
		{
			bWithRepeat = 1;
		}
		else if (sLine[6]== 'P')
		{
			bWithRepeat = 0;
		}
		else
		{
			return (SONG_DecodeError("Invalid jump repeat sign"));
		}

		// Decode the al fine sign
		if (sLine[7]== 'F')
		{
			bAlFine = 1;
		}
		else if (sLine[7]== ' ')
		{
			bAlFine = 0;
		}
		else
		{
			return (SONG_DecodeError("Invalid al fine sign"));
		}

		// Get jump addresses
		iJumpTo = SONG_GetNumber(sLine, 9, 3);
		iPlayUntil = SONG_GetNumber(sLine, 13, 3);
		iContinueAt = SONG_GetNumber(sLine, 17, 3);

		// it was a jump
		song_stTokens[song_iTokenLen].stJump.u1_isJump = 1;
		song_stTokens[song_iTokenLen].stJump.u3_JumpType = SONG_J_JUMP;
		song_stTokens[song_iTokenLen].stJump.u1_isSwitch = 0;
		song_stTokens[song_iTokenLen].stJump.u1_alFine = bAlFine;
		song_stTokens[song_iTokenLen].stJump.u1_withRepeat = bWithRepeat;

		song_stJDestinations[song_iJDestinationLen].u10_jumpTo = iJumpTo;
		song_stJDestinations[song_iJDestinationLen].u10_playUntil = iPlayUntil;
		song_stJDestinations[song_iJDestinationLen].u10_continueAt = iContinueAt;
		song_stTokens[song_iTokenLen].stJump.u8_jumpToDestinations = song_iJDestinationLen;
		song_iJDestinationLen++;

		song_stTokens[song_iTokenLen].stJump.u8_jumpToMemory = SONG_NO_JMP_REF;
	}
	else
	{
		return (SONG_DecodeError("Unknown command"));
	}

	song_iTokenLen++;
	// Check range
	if (song_iTokenLen >= 1024)
	{
		return (SONG_DecodeError("Too many tokens"));
	}
	// Check range
	if (song_iJDestinationLen >= SONG_NO_JMP_REF)
	{
		return (SONG_DecodeError("Too many destinations"));
	}
	// Check range
	if (song_iJMemoryLen >= SONG_NO_JMP_REF)
	{
		return (SONG_DecodeError("To many jumps with memory"));
	}

	// Puh, everything was ok. At least in this line
	return 0;
}


/**
 * Loads a song from USB stick
 *
 * \param iSong the song number from 0..9 = x.BBB
 * \return 0 if every is ok
 */
static int SONG_Load(int iSong)
{
    char sFilename[] = "*.BBB";
    char sLine[40];;
    int error = 0;
    int lineNr = 0;

    // We accept only 10 songs from 0..9
    if (iSong<0 || iSong>9)
    {
		PRINTF_printf("Only songs 0.BBB .. 9.BBB allowed");
		CONSOLE_Prompt();
    	return 1;
    }

    sFilename[0] = iSong + '0';

	// File on stick?
	if (f_open(&USBHFile, sFilename, FA_READ) == FR_OK)
	{

    	// Read the *.c file line by line
		while (f_gets(sLine, sizeof(sLine)-1, &USBHFile) != 0 && !error)
		{
			lineNr ++;

			if (lineNr == 1)
			{
				error = SONG_DecodeVersion(sLine);
			}
			else if (lineNr == 2)
			{
				error = SONG_DecodeTitle(sLine);
			}
			else if (lineNr == 3)
			{
				error = SONG_DecodeTempo(sLine);
			}
			else if (lineNr == 4)
			{
				error = SONG_DecodeStart(sLine);
			}
			else
			{
				error = SONG_DecodeLine(sLine);
			}

		}
		// Close the file
        f_close(&USBHFile);
	}
	else
	{
	    // BBB file not found
		PRINTF_printf("File %d.BBB not found", iSong);
		CONSOLE_Prompt();
		// File not found
		return 1;
	}

	// Error while reading the file?
	if (error)
	{
		ERRORHANDLER_SetError(ERROR_IN_BBB_FILE);
		PRINTF_printf("Error in *.BBB file in line %d", lineNr);
		CONSOLE_Prompt();
		// File not found
		return 1;
	}

	ERRORHANDLER_ResetError(ERROR_IN_BBB_FILE);
	PRINTF_printf("File %s loaded successfully", sFilename);
	CONSOLE_Prompt();

	// File successfuly loaded
	return 0;
}


/**
 * Turn on/off the blue LED if the selected song was loaded
 *
 */
static void SONG_StatusLED()
{
	// Turn on/off the blue LED
	if (song_bSongLoaded)
	{
		HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
	}
}

/**
 * Select a song from 1 .. 9
 *
 */
void SONG_Select(int iSong)
{
	// Has it changed?
	song_bSelectedChanged = song_iSongSelected != iSong;
	// Update the LED
	if (song_bSelectedChanged)
	{
		SONG_StatusLED();
	}

	// Update it
	song_iSongSelected = iSong;
}

/**
 * Selected song is loaded
 *
 */
int SONG_Loaded()
{
	return song_bSongLoaded;
}
