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
	CONSOLE_NewLine();
	CONSOLE_NewLine();
	CONSOLE_NewLine();
	CONSOLE_NewLine();
	CONSOLE_PrintfLn("BeatBassBox v0.1");
	CONSOLE_PrintfLn("Type HELP for help");
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
	if (cmd[console_iCmdLen] != 0)
	{
		return 0;
	}
	return 1;
}
/**
 * Process the command
 *
 */
static int CONSOLE_ProcessCmd(void)
{
	if (CONSOLE_IsCmd("HELP"))
	{
		CONSOLE_NewLine();
		CONSOLE_PrintfLn("SERVO.SET [servo_nr] [value]   Set a servo to a value");
		CONSOLE_PrintfLn("HAMMER.DRUM                    Drum with actual parameters");
		CONSOLE_PrintfLn("HAMMER.DRUMCORR                Set positions dependent parameter and drums");
		CONSOLE_PrintfLn("HAMMER.PAR                     Show the hammer parameters");
		CONSOLE_PrintfLn("HAMMER.PAR [par] [value]       Set a parameter to a value");
		CONSOLE_PrintfLn("HAMMER.PARD ...                Same, but also drums");
		CONSOLE_PrintfLn("TMC.READ                       Read all parameters of the TMC controller");
		CONSOLE_PrintfLn("TMC.READ [par]                 Read one parameters of the TMC controller");
		CONSOLE_PrintfLn("TMC.WRITE [par] [value]        Write one parameters to the TMC controller");
		CONSOLE_PrintfLn("TMC.POS                        Reads the position");
		CONSOLE_PrintfLn("TMC.POS [pos]                  Move to a position");
		CONSOLE_PrintfLn("TMC.REF                        Start a reference move");
		CONSOLE_PrintfLn("FRQD.DEBUG [0/1]               Switch on/off debug mode");
		CONSOLE_PrintfLn("FRQD.FILTER                    Read filter parameters");
		CONSOLE_PrintfLn("FRQD.FILTER [p0] [p1] [p2]     Set 3 filter parameters");
		CONSOLE_PrintfLn("FRQD.DETECTION                 Show detection parameters");
		CONSOLE_PrintfLn("FRQD.DETECTION [p0] [p1] [p2]  Set 3 detection parameters");
		CONSOLE_PrintfLn("FRQD.MAXFRQ                    Show the max frequency");
		CONSOLE_PrintfLn("FRQD.MAXFRQ [frq]              Set the max frequency");
		CONSOLE_PrintfLn("FRQD.START                     Starts a frequency detection run");
		CONSOLE_PrintfLn("BASS.CALIB                     Starts calibration run");
		CONSOLE_PrintfLn("BASS.TEST [value]              Plays one bass note get the freq.");
	}
	else if (CONSOLE_IsCmd("SERVO.SET"))
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
			HAMMER_DrumRaw();
		}
		else
		{
			return CONSOLE_ERROR_PAR_COUNT;
		}
	}
	else if (CONSOLE_IsCmd("HAMMER.DRUMCORR"))
	{
		if (console_iPars == 0)
		{
			HAMMER_DrumCorrected(0);
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
	else if (CONSOLE_IsCmd("HAMMER.PARD"))
	{
		if (console_iPars == 0)
		{
			HAMMER_DrumRaw();
		}
		else if (console_iPars == 2)
		{
			HAMMER_ParSet(console_as32Pars[0], console_as32Pars[1]);
			HAMMER_DrumRaw();
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
	else if (CONSOLE_IsCmd("TMC.POS"))
	{
		if (console_iPars == 0)
		{
			TMC5160_PrintPos();
		}
		else if (console_iPars == 1)
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
		if (console_iPars == 1)
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
	else if (CONSOLE_IsCmd("BASS.TEST"))
	{
		if (console_iPars == 1)
		{
			BASS_Test(
					console_as32Pars[0]);
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

	CONSOLE_Printf(" ");
	if (iError == CONSOLE_NO_ERROR)
	{
		iError = CONSOLE_ProcessCmd();
		if (iError == CONSOLE_NO_ERROR)
		{
			// do nothing
		}
		else if (iError == CONSOLE_ERROR_PAR_COUNT)
		{
			CONSOLE_Printf("Wrong amount of parameters");
		}
		else
		{
			CONSOLE_Printf("Unknown command");
		}

	}
	else if (iError == CONSOLE_ERROR_CMD)
	{
		CONSOLE_Printf("Error in command");
	}
	else if (iError == CONSOLE_ERROR_TO_MANY_PARS)
	{
		CONSOLE_Printf("Too many parameters");
	}
	else if (iError == CONSOLE_ERROR_PAR)
	{
		CONSOLE_Printf("Error in parameter");
	}
	CONSOLE_Prompt();

}

/**
 * printf on console
 *
 */
int CONSOLE_Printf(const char *format, ...)
{
	register int *varg = (int*) (&format);
	return PRINTF_printf(format, varg);
}

/**
 * printf on console
 *
 */
int CONSOLE_PrintfPrompt(const char *format, ...)
{
	int r;
	register int *varg = (int*) (&format);
	r = PRINTF_printf(format, varg);
	CONSOLE_Prompt();
	return r;
}
/**
 * printf on console
 *
 */
int CONSOLE_PrintfLn(const char *format, ...)
{
	int r;
	register int *varg = (int*) (&format);
	r = PRINTF_printf(format, varg);
	CONSOLE_NewLine();
	return r;
}


/**
 * Prints a new line
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

