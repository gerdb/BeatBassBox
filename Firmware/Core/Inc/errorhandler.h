/*
 *  Project:      BeatBassBox
 *  File:         errorhandler.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for errorhandler.c
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

#ifndef __ERRORHANDLER_H__
#define __ERRORHANDLER_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Constants  ----------------------------------------------------------------*/
#define ERROR_NO_ERROR			0

#define ERROR_DURING_CALIB		1
#define ERROR_IN_BBB_FILE		2
#define ERROR_TMC5160			3

#define ERROR_MAX_ERROR_ID		3


/* Types ---------------------------------------------------------------------*/


/* Function prototypes  ------------------------------------------------------*/
void ERRORHANDLER_Init();
void ERRORHANDLER_SetError(int iErrId);
void ERRORHANDLER_SetErrorText(char* text, int iErrId);
void ERRORHANDLER_ResetError(int iErrId);
int ERRORHANDLER_GetError(void);

#endif /* __ERRORHANDLER_H__ */
