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
static void USBSTICK_CheckBBBFiles(void);

/* Functions -----------------------------------------------------------------*/



/**
 * @brief Reads the Configuration file
 */
static void USBSTICK_CheckBBBFiles(void)
{
    FILINFO fno;
    char sFilename[] = "*.bbb";

    // Count the BBB files
    for (int i=0;i<=9; i++)
    {
    	sFilename[0] = i + '0';
    	if (f_stat(sFilename, &fno) == FR_OK)
    	{
    		ustick_iBBBFiles ++;
    	}
    }

    // How many BBB files found?
	PRINTF_printf("%d BBB files found", ustick_iBBBFiles);
	CONSOLE_Prompt();

	// There must be at least one BBB file
	if (ustick_iBBBFiles > 0)
    {
		ERRORHANDLER_ResetError(ERROR_NO_BBB_FILE);
    }
    else
    {
    	ERRORHANDLER_SetError(ERROR_NO_BBB_FILE);
    }
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
		USBSTICK_CheckBBBFiles();
	}
}

/**
 * Callback function: An USB stick was disconnected
 */
void USBSTICK_Disconnected(void)
{
	ustick_bMounted = 0;
	PRINTF_printf("USB stick removed");
	ERRORHANDLER_SetError(ERROR_NO_BBB_FILE);
	CONSOLE_Prompt();
}
