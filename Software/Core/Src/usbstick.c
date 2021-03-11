/*
 *  Project:      BeatBassBox
 *  File:         usbstick.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  USB stick with music files
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
#include "usb_host.h"
#include "usbstick.h"
#include "printf.h"
#include "console.h"
#include "errorhandler.h"

/* Variables -----------------------------------------------------------------*/
int ustick_bMounted = 0;
int ustick_iBBBFiles = 0;
/* Local function prototypes -------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/






/**
 * Returns true, if an USB stick is connected (only the edge)
 */
int USBSTICK_PluggedInEvent(void)
{
	static int bMountedOld = 0;
	int retval;

	retval =  ustick_bMounted && !bMountedOld;
	bMountedOld = ustick_bMounted;

	return retval;
}

/**
 * Callback function: An USB stick was connected
 */
void USBSTICK_Connected(void)
{
	// Mount the USB stick and read the files
	if (f_mount(&USBHFatFS, (TCHAR const*) USBHPath, 0) == FR_OK)
	{
		ustick_bMounted = 1;
		PRINTF_printf("USB stick connected");
		CONSOLE_Prompt();
	}
}

/**
 * Callback function: An USB stick was disconnected
 */
void USBSTICK_Disconnected(void)
{
	ustick_bMounted = 0;
	PRINTF_printf("USB stick removed");
	CONSOLE_Prompt();
}
