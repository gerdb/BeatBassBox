/*
 *  Project:      BeatBassBox
 *  File:         usbstick.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for usbstick.c
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
#ifndef __USBSTICK_H__
#define __USBSTICK_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/
extern int ustick_bMounted;

/* Function prototypes -------------------------------------------------------*/
void USBSTICK_Connected(void);
void USBSTICK_Disconnected(void);
int USBSTICK_PluggedInEvent(void);
int USBSTICK_IsAvailable(void);

#endif /* __USB_STICK_H__ */
