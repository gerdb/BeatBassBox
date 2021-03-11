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
int song_iTokenCnt;

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

	song_iTokenCnt = 0;
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


	// Get Token number
	iTokenNr = SONG_GetNumber(sLine, 0, 3);
	// Check syntax
	if (iTokenNr != song_iTokenCnt)
	{
		PRINTF_printf("Invalid token number");
		CONSOLE_Prompt();
		return -1;
	}

	// Check syntax
	if (sLine[3]!= ':')
	{
		PRINTF_printf("There must be a ':' at position 3");
		CONSOLE_Prompt();
		return -1;
	}

	// Decode the command
	if (sLine[5]== 'B' && sLine[6]== 'B' && sLine[7]== ':')
	{
		// it was a BB command
		iDuration = SONG_GetNumber(sLine, 8, 2);
		// Check range
		if (iDuration<0 || iDuration>31)
		{
			PRINTF_printf("Duration must be between 0 and 31");
			CONSOLE_Prompt();
			return -1;
		}
		// Check syntax
		if (sLine[10]!= ':')
		{
			PRINTF_printf("There must be a ':' at position 10");
			CONSOLE_Prompt();
			return -1;
		}

		// Get the bass note
		iBass = SONG_GetNumber(sLine, 11, 2);
		if (iBass != 0)
		{
			// Offset 20
			iBass-= 20;

			// Check range
			if (iBass<1 || iBass>63)
			{
				PRINTF_printf("Bass note out of range");
				CONSOLE_Prompt();
				return -1;
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
			PRINTF_printf("Invalid articulation sign");
			CONSOLE_Prompt();
			return -1;
		}

		// Check syntax
		if (sLine[14]!= ':')
		{
			PRINTF_printf("There must be a ':' at position 14");
			CONSOLE_Prompt();
			return -1;
		}

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
				PRINTF_printf("End of line");
				CONSOLE_Prompt();
				return -1;
			}
			if (c != ' ')
			{
				iBeat |= 1;
			}
		}

		// It's a bass-beat chord an not a jump
		song_stTokens[song_iTokenCnt].stBassBeat.u1_isJump = 0;
		song_stTokens[song_iTokenCnt].stBassBeat.u5_Duration = iDuration;
		song_stTokens[song_iTokenCnt].stBassBeat.u6_Bass = iBass;
		song_stTokens[song_iTokenCnt].stBassBeat.u1_Articulated = bIsArticulated;
		song_stTokens[song_iTokenCnt].stBassBeat.u10_Beat = iBeat;
	}
	else if (sLine[5]== ']' && sLine[7]== ':')
	{
		// it was a single repeat
	}
	else if (sLine[5]== 'F' && sLine[6]== 'I')
	{
		// it was a fine
	}
	else if (sLine[5]== 'V' && sLine[6]== '1')
	{
		// it was a volta
	}
	else if (sLine[5]== 'J' && sLine[8]== ':')
	{
		// it was a jump
	}
	else
	{
		PRINTF_printf("Unknown command");
		CONSOLE_Prompt();
		return -1;
	}

	song_iTokenCnt++;
	// Check syntax
	if (song_iTokenCnt >= 1024)
	{
		PRINTF_printf("To many tokens");
		CONSOLE_Prompt();
		return -1;
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
		PRINTF_printf("Only songs 0.BBB .. 9.BBB allowed", iSong);
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
