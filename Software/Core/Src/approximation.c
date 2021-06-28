/*
 *  Project:      BeatBassBox
 *  File:         approximation.c
 *  Author:       Gerd Bartelt - www.sebulli.com
 *
 *  Description:  Approximation of non linear rubber equation
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
#include "printf.h"
#include "console.h"
#include "approximation.h"
#include "errorhandler.h"
#include <math.h>

/* Variables -----------------------------------------------------------------*/

// size of data points
int approx_iN;
#define APPROX_N_MAX 50
// polynomial degree
#define APPROX_DEGREE 3

// input data
float approx_afx[APPROX_N_MAX];
float approx_afy[APPROX_N_MAX];

// matrix
float approx_afX[2 * APPROX_DEGREE + 1];
float approx_afY[APPROX_DEGREE + 1];
float approx_afB[APPROX_DEGREE + 1][APPROX_DEGREE + 2];

// coefficents
float approx_afA[APPROX_DEGREE + 1];


/* Local function prototypes -------------------------------------------------*/
static void APPROX_Prepare();

/* Functions -----------------------------------------------------------------*/

/**
 * Initialize this module
 *
 *
 */
void APPROX_Init()
{
	approx_iN = 0;
	for (int i = 0; i <= APPROX_DEGREE; i++)
	{
		approx_afA[i] = 0.0f;;
	}
}

/**
 * Add one point
 *
 * \param iPos position
 * \param fFrq frequency
 *
 */
void APPROX_Point(int iPos, float fFrq)
{
	if (approx_iN < APPROX_N_MAX)
	{
		approx_afx[approx_iN] = fFrq;
		approx_afy[approx_iN] = (float)iPos;
		approx_iN ++;
	}
}


/**
 * Calculate a value
 *
 * \param fFrq input frequency
 *
 * \return the target position of the arm
 */
int APPROX_Calc(float fFrq)
{
	float f = approx_afA[APPROX_DEGREE];

	for (int i= APPROX_DEGREE-1 ; i>=0; i--)
	{
		f *= fFrq;
		f += approx_afA[i];
	}
	return (int) f;
}
/**
 * Prepare the matrix
 *
 */
static void APPROX_Prepare()
{
	int i, j;

	for (i = 0; i <= 2 * APPROX_DEGREE; i++)
	{
		approx_afX[i] = 0;
		for (j = 0; j < approx_iN; j++)
		{
			approx_afX[i] = approx_afX[i] + pow(approx_afx[j], i);
		}
	}
	for (i = 0; i <= APPROX_DEGREE; i++)
	{
		approx_afY[i] = 0;
		for (j = 0; j < approx_iN; j++)
		{
			approx_afY[i] = approx_afY[i] + pow(approx_afx[j], i) * approx_afy[j];
		}
	}
	for (i = 0; i <= APPROX_DEGREE; i++)
	{
		for (j = 0; j <= APPROX_DEGREE; j++)
		{
			approx_afB[i][j] = approx_afX[i + j];
		}
	}
	for (i = 0; i <= APPROX_DEGREE; i++)
	{
		approx_afB[i][APPROX_DEGREE + 1] = approx_afY[i];
	}
}

/**
 * Do the approximation and calculate the coefficients
 *
 */
void APPROX_Approximate()
{
	APPROX_Prepare();

	int i, j, k;
	for (i = 0; i < APPROX_DEGREE; i++)
	{
		// Partial pivoting
		for (k = i + 1; k < (APPROX_DEGREE + 1); k++)
		{
			// If diagonal element(absolute value) is smaller than any of the terms below it
			if (fabsf(approx_afB[i][i]) < fabsf(approx_afB[k][i]))
			{
				// Swap the rows
				for (j = 0; j < (APPROX_DEGREE + 2); j++)
				{
					float temp;
					temp = approx_afB[i][j];
					approx_afB[i][j] = approx_afB[k][j];
					approx_afB[k][j] = temp;
				}
			}
		}
		// Begin Gauss elimination
		for (k = i + 1; k < (APPROX_DEGREE + 1); k++)
		{
			float term = approx_afB[k][i] / approx_afB[i][i];
			for (j = 0; j < (APPROX_DEGREE + 2); j++)
			{
				approx_afB[k][j] = approx_afB[k][j] - term * approx_afB[i][j];
			}
		}
	}

	//Begin back substitution
	for (i = APPROX_DEGREE; i >= 0; i--)
	{
		approx_afA[i] = approx_afB[i][APPROX_DEGREE + 1];
		for (j = i + 1; j < (APPROX_DEGREE + 1); j++)
		{
			approx_afA[i] = approx_afA[i] - approx_afB[i][j] * approx_afA[j];
		}
		approx_afA[i] = approx_afA[i] / approx_afB[i][i];
	}
}
