/*
 *  Project:      BeatBassBox
 *  File:         tmc5160.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for tmc5160.c
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

#ifndef __TMC5160_H__
#define __TMC5160_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Constants  ----------------------------------------------------------------*/


/* Types ---------------------------------------------------------------------*/
// Structure for one servo
typedef struct __packed
{
  unsigned int	u7Address:7;
  unsigned int	u1W:1;
  uint32_t		u32Data;
} TMC5160_SPI_TX_s;

typedef struct __packed
{
	uint8_t		reset_flag:1;
	uint8_t		driver_error:1;
	uint8_t		sg2:1;
	uint8_t		standstill:1;
	uint8_t		velocity_reached:1;
	uint8_t		position_reached:1;
	uint8_t		status_stop_l:1;
	uint8_t		status_stop_r:1;

	uint32_t	u32Data;
} TMC5160_SPI_RX_s;

/* Function prototypes  ------------------------------------------------------*/
void TMC5160_Init();
void TMC5160_Task1ms();
void TMC5160_Task100ms();

#endif /* __TMC5160_H__ */
