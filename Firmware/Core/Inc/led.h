/*
 *  Project:      BeatBassBox
 *  File:         led.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for led.c
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

#ifndef __LED_H__
#define __LED_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Constants  ----------------------------------------------------------------*/

/* Types ---------------------------------------------------------------------*/
typedef enum
{
	LED_S_REF, LED_S_SONG_LOADED, LED_S_SONG_NOT_LOADED
} LED_StateType_e;

typedef enum
{
	LED_C_OFF, LED_C_GREEN, LED_C_RED, LED_C_BLUE
} LED_Colorype_e;

/* Function prototypes  ------------------------------------------------------*/
void LED_Init();
void LED_Task1ms();
void LED_State(LED_StateType_e eState);

#endif /* __LED_H__ */
