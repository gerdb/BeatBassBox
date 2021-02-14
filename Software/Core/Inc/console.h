/*
 *  Project:      BeatBassBox
 *  File:         console.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for console.c
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
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/
#define CONSOLE_LINE_LENGTH 80
#define CONSOLE_PARAMETERS 4

#define CONSOLE_NO_ERROR 0
#define CONSOLE_ERROR_CMD 1
#define CONSOLE_ERROR_TO_MANY_PARS 2
#define CONSOLE_ERROR_PAR 3
#define CONSOLE_ERROR_UNKNOWN_CMD 4
#define CONSOLE_ERROR_PAR_COUNT 5


/* Function prototypes -------------------------------------------------------*/
void CONSOLE_Init(void);
void CONSOLE_NewChar(char c);


#endif /* __CONSOLE_H__ */
