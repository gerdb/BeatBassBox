/*
 *  Project:      BeatBassBox
 *  File:         com.h
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  header file for com.c
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
#ifndef __COM_H__
#define __COM_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Defines -------------------------------------------------------------------*/
#define HCOM huart3

// Transmit buffer with read and write pointer
#define COM_TX_SIZE 2048
#define COM_TX_MASK (COM_TX_SIZE-1)

// Receive buffer with read and write pointer
#define COM_RX_SIZE 256
#define COM_RX_MASK (COM_RX_SIZE-1)

/* Function prototypes -------------------------------------------------------*/
void COM_Init(void);
void COM_RxBufferTask(void);
int COM_RxBufferNotEmpty(void);
void COM_PutByte(uint8_t b);
void COM_ISR(void);

#endif /* __COM_H__ */
