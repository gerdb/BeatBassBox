/*
 *  Project:      BeatBassBox
 *  File:         printf.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for printf.c
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PRINTF_H__
#define __PRINTF_H__


/* Includes ------------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
int PRINTF_printf(const char *format, ...);
int PRINTF_sprintf(char *out, const char *format, ...);


#endif /* __PRINTF_H__ */
