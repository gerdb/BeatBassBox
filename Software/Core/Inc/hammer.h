/*
 *  Project:      BeatBassBox
 *  File:         hammer.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for hammer.c
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
#ifndef __HAMMER_H__
#define __HAMMER_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/


/* Function prototypes -------------------------------------------------------*/
void HAMMER_Init();
void HAMMER_Task1ms();
void HAMMER_Drum(void);
void HAMMER_DrumRaw(void);
void HAMMER_ParSet(int iParameter, int iVal);
void HAMMER_ParGet();

#endif /* __HAMMER_H__ */
