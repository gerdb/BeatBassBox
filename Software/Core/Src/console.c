/*
 *  Project:      BeatBassBox
 *  File:         console.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  A terminal console
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
#include "com.h"
#include "printf.h"
#include "console.h"
#include "frqdetect.h"
#include "tmc5160.h"
#include "bass.h"
#include "hammer.h"

/* Variables -----------------------------------------------------------------*/
char console_acLine[CONSOLE_LINE_LENGTH];
int console_iCmdLen;
int console_iLinePtr;
int32_t console_as32Pars[CONSOLE_PARAMETERS];
int32_t console_as32Signs[CONSOLE_PARAMETERS];
int console_iPars;

/* Local function prototypes -------------------------------------------------*/
static void CONSOLE_ProcessLine(void);
static int CONSOLE_ProcessCmd(void);
static int CONSOLE_IsCmd(char *cmd);
static void CONSOLE_RepeatLast(void);

/* Functions -----------------------------------------------------------------*/
/**
 * Initialize this module
 *
 */
void CONSOLE_Init(void)
{
	PRINTF_printf("\r\n\r\nBeatBassBox v0.1");
	CONSOLE_Prompt();
}

/**
 * Writes an (error) text
 *
 */
void CONSOLE_Text(char* text)
{
	PRINTF_printf(text);
	CONSOLE_Prompt();
}

/**
 * Check whether it is a specified command
 *
 */
static int CONSOLE_IsCmd(char *cmd)
{
	for (int i = 0; i < console_iCmdLen; i++)
	{
		if (console_acLine[i] != cmd[i] || cmd[i] == 0)
		{
			return 0;
		}
	}
	return 1;
}
/**
 * Process the command
 *
 */
static int CONSOLE_ProcessCmd(void)
{
	if (CONSOLE_IsCmd("SERVO.SET"))
	{
		if (console_iPars == 2)
		{
			//SERVO_Set(console_as32Pars[0],console_as32Pars[1]);
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("HAMMER.DRUM"))
	{
		if (console_iPars == 0)
		{
			HAMMER_Drum();
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("HAMMER.PAR"))
	{
		if (console_iPars == 0)
		{
			CONSOLE_NewLine();
			HAMMER_ParGet();
		}
		else if (console_iPars == 2)
		{
			HAMMER_ParSet(console_as32Pars[0], console_as32Pars[1]);
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("TMC.READ"))
	{
		if (console_iPars == 0)
		{
			CONSOLE_NewLine();
			TMC5160_ReadAll();
		}
		else if (console_iPars == 1)
		{
			CONSOLE_NewLine();
			TMC5160_Read(console_as32Pars[0]);
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("TMC.WRITE"))
	{
		if (console_iPars == 2)
		{
			TMC5160_Write(
					console_as32Pars[0],
					console_as32Pars[1]);
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("TMC.MOVE"))
	{
		if (console_iPars == 1)
		{
			TMC5160_MoveTo(console_as32Pars[0]);
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("TMC.REF"))
	{
		if (console_iPars == 0)
		{
			TMC5160_Ref();
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}


	else if (CONSOLE_IsCmd("FRQD.DEBUG"))
	{
		if (console_iPars < 2)
		{
			FRQDETECT_SetDebug(console_as32Pars[0]);
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("FRQD.FILTER"))
	{
		if (console_iPars == 0)
		{
			CONSOLE_NewLine();
			FRQDETECT_PrintFilter();
		}
		else if (console_iPars == 3)
		{
			FRQDETECT_SetFilter(
					console_as32Pars[0],
					console_as32Pars[1],
					console_as32Pars[2]);
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("FRQD.DETECTION"))
	{
		if (console_iPars == 0)
		{
			CONSOLE_NewLine();
			FRQDETECT_PrintDetection();
		}
		else if (console_iPars == 3)
		{
			FRQDETECT_SetDetection(
					console_as32Pars[0],
					console_as32Pars[1],
					console_as32Pars[2]);
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("FRQD.MAXFRQ"))
	{
		if (console_iPars == 0)
		{
			CONSOLE_NewLine();
			FRQDETECT_PrintMaxFrq();
		}
		else if (console_iPars == 1)
		{
			FRQDETECT_SetMaxFrq(
					console_as32Pars[0]);
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("FRQD.START"))
	{
		if (console_iPars == 0)
		{
			FRQDETECT_Start();
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("BASS.CALIB"))
	{
		if (console_iPars == 0)
		{
			BASS_StartCalib();
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else
	{
		return CONSOLE_ERROR_UNKNOWN_CMD;
	}
	return CONSOLE_NO_ERROR;
}

/**
 * Process the stored line
 *
 */
static void CONSOLE_ProcessLine(void)
{
	int iBase = 10;
	int bIsCmd = 1;
	int bSepFound = 0;
	int iError = CONSOLE_NO_ERROR;
	char c;
	int iParIndex = -1;

	console_iCmdLen = 0;
	console_iPars = 0;

	for (int i = 0; i < console_iLinePtr && (iError == CONSOLE_NO_ERROR); i++)
	{
		c = console_acLine[i];
		if (bIsCmd)
		{
			if (c == ' ')
			{
				bIsCmd = 0;
				bSepFound = 1;
				iBase = 10;
			}
			else
			{
				if ((c >= 'A' && c <= 'Z') || c == '_' || c == '.')
				{
					console_iCmdLen++;
				}
				else
				{
					iError = CONSOLE_ERROR_CMD;
				}
			}
		}
		else
		{

			if (c == '\r' || c == '\n')
			{
				// ignore it;
			}
			else if (c == ' ' || c == ',')
			{
				bSepFound = 1;
			}
			else
			{
				if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')
						|| c == '-' || c == '+'|| c == 'X')
				{
					if (bSepFound)
					{
						bSepFound = 0;
						if (iParIndex < (CONSOLE_PARAMETERS - 1))
						{
							iParIndex++;
							console_iPars = iParIndex + 1;
							console_as32Pars[iParIndex] = 0;
							console_as32Signs[iParIndex] = 1;
							iBase = 10;
						}
						else
						{
							iError = CONSOLE_ERROR_TO_MANY_PARS;
						}

					}
					if (iParIndex >= 0)
					{
						if (c == 'X')
						{
							iBase = 16;
						}
						else if (c >= '0' && c <= '9')
						{
							console_as32Pars[iParIndex] *= iBase;
							console_as32Pars[iParIndex] += c - '0';
						}
						else if (c >= 'A' && c <= 'F')
						{
							console_as32Pars[iParIndex] *= iBase;
							console_as32Pars[iParIndex] += c - 'A' + 10;
						}
						else if (c == '-')
						{
							console_as32Signs[iParIndex] = -1;
						}
					}
					else
					{
						iError = CONSOLE_ERROR_PAR;
					}
				}
				else
				{
					iError = CONSOLE_ERROR_PAR;
				}
			}
		}
	}


	for (int i = 0; i < CONSOLE_PARAMETERS; i++)
	{
		console_as32Pars[i] *= console_as32Signs[i];
	}

	if (console_iCmdLen < 1)
	{
		iError = CONSOLE_ERROR_CMD;
	}

	PRINTF_printf(" ");
	if (iError == CONSOLE_NO_ERROR)
	{
		iError = CONSOLE_ProcessCmd();
		if (iError == CONSOLE_NO_ERROR)
		{
			// do nothing
		}
		else if (iError == CONSOLE_ERROR_PAR_COUNT)
		{
			PRINTF_printf("Wrong amount of parameters");
		}
		else
		{
			PRINTF_printf("Unknown command");
		}

	}
	else if (iError == CONSOLE_ERROR_CMD)
	{
		PRINTF_printf("Error in command");
	}
	else if (iError == CONSOLE_ERROR_TO_MANY_PARS)
	{
		PRINTF_printf("Too many parameters");
	}
	else if (iError == CONSOLE_ERROR_PAR)
	{
		PRINTF_printf("Error in parameter");
	}
	CONSOLE_Prompt();

}
/**
 * Prints a new new line
 *
 */
void CONSOLE_NewLine(void)
{
	COM_PutByte('\r');
	COM_PutByte('\n');
}


/**
 * Prepares a new new line
 *
 */
void CONSOLE_Prompt(void)
{

	for (int i = 0; i < CONSOLE_PARAMETERS; i++)
	{
		console_as32Pars[i] = 0;
	}
	console_iLinePtr = 0;
	CONSOLE_NewLine();
	COM_PutByte('>');
}


/**
 * Repeat the last command
 *
 */
static void CONSOLE_RepeatLast(void)
{
	console_iLinePtr = console_iCmdLen;

	if (console_acLine[console_iCmdLen] == ' ')
	{
		console_iLinePtr++;
	}

	for (int i = 0; i < console_iLinePtr; i++)
	{
		COM_PutByte(console_acLine[i]);
	}

}

/**
 * Prepares a new new line
 *
 */
void CONSOLE_NewChar(char c)
{
	static int iEsc = 0;
	static char cEsc1;
	static int bModified = 0;

	if (iEsc > 0)
	{
		if (iEsc == 2)
		{
			cEsc1 = c;
		}
		else if (iEsc == 1)
		{
			if (cEsc1 == '[')
			{
				if (c == 'A')
				{
					if (bModified == 0)
					{
						CONSOLE_RepeatLast();
					}
				}
			}
		}
		iEsc--;
	}
	else if (c == 27)
	{
		iEsc = 2;
	}
	else if (c == '\n')
	{
		COM_PutByte(c);
	}
	else if (c == '\r')
	{
		if (console_iLinePtr == 0)
		{
			CONSOLE_RepeatLast();
		}
		bModified = 0;
		CONSOLE_ProcessLine();
	}
	else if (c == 127 && console_iLinePtr > 0)
	{
		bModified = 1;
		console_iLinePtr--;
		COM_PutByte(c);
	}
	else if (c >= ' ' && c <= 'z' && console_iLinePtr < CONSOLE_LINE_LENGTH)
	{
		bModified = 1;

		if (c >= 'a')
		{
			c -= 'a' - 'A';
		}
		console_acLine[console_iLinePtr] = c;
		console_iLinePtr++;
		COM_PutByte(c);
	}
}

