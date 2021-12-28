/*
 *  Project:      BeatBassBox
 *  File:         errorhandler.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Errorhandler
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
#include "errorhandler.h"
#include "console.h"

/* Variables -----------------------------------------------------------------*/
uint32_t u32ErrorCode;

/* Prototypes of static function ---------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 *
 */
void ERRORHANDLER_Init()
{
	u32ErrorCode = ERROR_NO_ERROR;
}


/**
 * Sets an error with error text and id
 *
 */
void ERRORHANDLER_SetErrorText(char* text, int iErrId)
{
	ERRORHANDLER_SetError(iErrId);
	CONSOLE_PrintfPrompt(text);
}
/**
 * Sets an error with error text and id
 *
 */
void ERRORHANDLER_SetError(int iErrId)
{
	u32ErrorCode |= (1<<(iErrId-1));
}

/**
 * Resets an error with error text and id
 *
 */
void ERRORHANDLER_ResetError(int iErrId)
{
	u32ErrorCode &= ~(1<<(iErrId-1));
}

/**
 * Get the error code with the highest priority (lowest ErrId)
 *
 */
int ERRORHANDLER_GetError(void)
{
	uint32_t u32ErrorCodeTmp;

	// No Error
	if (u32ErrorCode == 0)
		return 0;

	u32ErrorCodeTmp = u32ErrorCode;

	// Return the error ID
	for (int i=1; i<=ERROR_MAX_ERROR_ID; i++)
	{
		if (u32ErrorCodeTmp & 1)
		{
			return i;
		}
		u32ErrorCodeTmp>>=1;
	}
	return 0;
}


