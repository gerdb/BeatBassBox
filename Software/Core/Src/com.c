/*
 *  Project:      BeatBassBox
 *  File:         com.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Serial communication
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
#include "usart.h"
#include "console.h"
#include "com.h"

/* Variables -----------------------------------------------------------------*/

// Transmit buffer with read and write pointer
uint8_t com_au8TxBuffer[COM_TX_SIZE];
int com_bTxEn = 0;
int com_iTxWrPtr = 0;
int com_iTxTdPtr = 0;

// Receive buffer with read and write pointer
uint8_t com_au8RxBuffer[COM_RX_SIZE];
int com_iRxWrPtr = 0;
int com_iRxRdPtr = 0;
/* Static function prototypes--------------------------------------------------*/
static void COM_Receive(void);
static void COM_Transmit(void);

/* Functions ------------------------------------------------------------------*/
/**
 * Initialize this module
 *
 */
void COM_Init(void)
{
	SET_BIT(HCOM.Instance->CR1, USART_CR1_RXNEIE);
}

/**
 * Check, if there is something in the receive buffer to decode
 *
 */
void COM_RxBufferTask(void)
{
	//COM_PutByte('X');

	//Increment the read pointer of the RX buffer
	if (com_iRxWrPtr != com_iRxRdPtr)
	{
		com_iRxRdPtr++;
		com_iRxRdPtr &= COM_RX_MASK;

		//Decode the received byte
		CONSOLE_NewChar(com_au8RxBuffer[com_iRxRdPtr]);
	}
}

/**
 * Returns, whether the receive buffer is empty or not
 *
 * \return =0: there was no byte in the receive buffer
 *         >0: there was at least one byte in the receive buffer.
 */
int COM_RxBufferNotEmpty(void)
{

	return com_iRxWrPtr != com_iRxRdPtr;
}

/**
 * Write a new character into the transmit buffer
 * This function is called by the printf function
 *
 * \param
 *              ch New character to send
 */
void COM_PutByte(uint8_t b)
{

	__disable_irq();

	// Send the byte
	if (!com_bTxEn)
	{
		// Write the first byte directly into the USART
		com_bTxEn = 1;
		HCOM.Instance->TDR = (uint16_t) b;
		SET_BIT(HCOM.Instance->CR1, USART_CR1_TXEIE);

	}
	else
	{
		// Write the next character into the buffer
		com_iTxWrPtr++;
		com_iTxWrPtr &= COM_TX_MASK;
		if (com_iTxWrPtr == com_iTxTdPtr)
		{
			com_iTxWrPtr--;
			com_iTxWrPtr &= COM_TX_MASK;
		}

		com_au8TxBuffer[com_iTxWrPtr] = b;
	}
	__enable_irq();

}

/**
 * Transmit a character
 */
static void COM_Transmit(void)
{
	//Increment the read pointer of the RX buffer
	if (com_iTxWrPtr != com_iTxTdPtr)
	{
		com_iTxTdPtr++;
		com_iTxTdPtr &= COM_TX_MASK;
		//send the next byte
		HCOM.Instance->TDR = (uint16_t) com_au8TxBuffer[com_iTxTdPtr];
	}
	else
	{
		// This was the last byte to send, disable the transmission.
		CLEAR_BIT(HCOM.Instance->CR1, USART_CR1_TXEIE);
		com_bTxEn = 0;
	}
}

/**
 * Receive a character
 */
static void COM_Receive(void)
{
	// Increment the buffer pointer, if it's possible
	com_iRxWrPtr++;
	com_iRxWrPtr &= COM_RX_MASK;
	if (com_iRxWrPtr == com_iRxRdPtr)
	{
		com_iRxWrPtr--;
		com_iRxWrPtr &= COM_RX_MASK;
	}

	// Write the received byte and the time difference into the buffer
	com_au8RxBuffer[com_iRxWrPtr] = (uint8_t) (HCOM.Instance->RDR);
}

/**
 * USART Interrupt handler
 */
__INLINE void COM_ISR(void)
{
	uint32_t tmp1 = 0, tmp2 = 0;

	tmp1 = __HAL_UART_GET_FLAG(&HCOM, UART_FLAG_PE);
	tmp2 = __HAL_UART_GET_IT_SOURCE(&HCOM, UART_IT_PE);
	/* UART parity error interrupt occurred ------------------------------------*/
	if ((tmp1 != RESET) && (tmp2 != RESET))
	{
		__HAL_UART_CLEAR_FLAG(&HCOM, UART_FLAG_PE);

		HCOM.ErrorCode |= HAL_UART_ERROR_PE;
	}

	tmp1 = __HAL_UART_GET_FLAG(&HCOM, UART_FLAG_FE);
	tmp2 = __HAL_UART_GET_IT_SOURCE(&HCOM, UART_IT_ERR);
	/* UART frame error interrupt occurred -------------------------------------*/
	if ((tmp1 != RESET) && (tmp2 != RESET))
	{
		__HAL_UART_CLEAR_FLAG(&HCOM, UART_FLAG_FE);

		HCOM.ErrorCode |= HAL_UART_ERROR_FE;
	}

	tmp1 = __HAL_UART_GET_FLAG(&HCOM, UART_FLAG_NE);
	tmp2 = __HAL_UART_GET_IT_SOURCE(&HCOM, UART_IT_ERR);
	/* UART noise error interrupt occurred -------------------------------------*/
	if ((tmp1 != RESET) && (tmp2 != RESET))
	{
		__HAL_UART_CLEAR_FLAG(&HCOM, UART_FLAG_NE);

		HCOM.ErrorCode |= HAL_UART_ERROR_NE;
	}

	tmp1 = __HAL_UART_GET_FLAG(&HCOM, UART_FLAG_ORE);
	tmp2 = __HAL_UART_GET_IT_SOURCE(&HCOM, UART_IT_ERR);
	/* UART Over-Run interrupt occurred ----------------------------------------*/
	if (tmp1 != RESET)
	{
		__HAL_UART_CLEAR_FLAG(&HCOM, UART_FLAG_ORE);

		HCOM.ErrorCode |= HAL_UART_ERROR_ORE;
	}

	tmp1 = __HAL_UART_GET_FLAG(&HCOM, UART_FLAG_RXNE);
	tmp2 = __HAL_UART_GET_IT_SOURCE(&HCOM, UART_IT_RXNE);
	/* UART in mode Receiver ---------------------------------------------------*/
	if ((tmp1 != RESET) && (tmp2 != RESET))
	{
		COM_Receive();
		__HAL_UART_CLEAR_FLAG(&HCOM, UART_FLAG_RXNE);
	}

	tmp1 = __HAL_UART_GET_FLAG(&HCOM, UART_FLAG_TXE);
	tmp2 = __HAL_UART_GET_IT_SOURCE(&HCOM, UART_IT_TXE);
	/* UART in mode Transmitter ------------------------------------------------*/
	if ((tmp1 != RESET) && (tmp2 != RESET))
	{
		COM_Transmit();
		__HAL_UART_CLEAR_FLAG(&HCOM, UART_FLAG_TXE);
	}

	if (HCOM.ErrorCode != HAL_UART_ERROR_NONE)
	{
		/* Set the UART state ready to be able to start again the process */
	}

}
