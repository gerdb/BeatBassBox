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

/* Functions ------------------------------------------------------------------*/
/**
 * Initialize this module
 *
 */
void COM_Init(void)
{
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
		//USARTL2_Decode(COM_rx_buffer[COM_rx_rd_pointer]);
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
		//USART_SendData(USART2, b);
		//USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
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
 * USART2 Interrupt handler
 */
__INLINE void COM_ISR(void)
{

	// Receive interrupt
	if ((READ_REG(HCOM.Instance->ISR) & USART_ISR_RXNE) != 0)
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

	// Transmit interrupt
	if ((READ_REG(HCOM.Instance->ISR) & USART_ISR_TXE) != 0)
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
}
