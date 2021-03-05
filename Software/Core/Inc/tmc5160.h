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
#define TMC5160_GCONF		0x00
#define TMC5160_IOIN		0x04
#define TMC5160_SHORTCONF	0x09
#define TMC5160_DRVCONF		0x0A
#define TMC5160_IHOLD_IRUN	0x10
#define TMC5160_TPOWERDOWN	0x11
#define TMC5160_RAMPMODE	0x20
#define TMC5160_VSTART		0x23
#define TMC5160_A1			0x24
#define TMC5160_V1			0x25
#define TMC5160_AMAX		0x26
#define TMC5160_VMAX		0x27
#define TMC5160_DMAX		0x28
#define TMC5160_D1			0x2A
#define TMC5160_VSTOP		0x2B
#define TMC5160_XTARGET		0x2D
#define TMC5160_ENC_CONST	0x3A
#define TMC5160_CHOPCONF	0x6C
#define TMC5160_PWMCONF		0x70

/* Types ---------------------------------------------------------------------*/

// TMC5160 registers
typedef union
{
	struct __packed
	{
		uint8_t		recalibrate:1;
		uint8_t		faststandstill:1;
		uint8_t		en_pwm_mode:1;
		uint8_t		multistep_filt:1;
		uint8_t		shaft:1;
		uint8_t		diag0_error:1;
		uint8_t		diag0_otpw:1;
		uint8_t		diag0_stall:1;
		uint8_t		diag1_stall:1;
		uint8_t		diag1_index:1;
		uint8_t		diag1_onstate:1;
		uint8_t		diag1_steps_skipped:1;
		uint8_t		diag0_int_pushpull:1;
		uint8_t		diag1_poscomp_pushpull:1;
		uint8_t		small_hysteresis:1;
		uint8_t		stop_enable:1;
		uint8_t		direct_mode:1;
		uint8_t		test_mode:1;
		uint32_t	:14;
	};
	uint32_t u32;
} TMC5160_REG_GCONF;

typedef union
{
	struct __packed
	{
		uint8_t		S2VS_LEVEL:4;
		uint8_t		:4;
		uint8_t		S2G_LEVEL:4;
		uint8_t		:4;
		uint8_t		SHORTFILTER:2;
		uint8_t		SHORTDELAY:1;
		uint32_t	:13;
	};
	uint32_t u32;
} TMC5160_REG_SHORT_CONF;

typedef union
{
	struct __packed
	{
		uint8_t		BBMTIME:5;
		uint8_t		:3;
		uint8_t		BBMCLKS:4;
		uint8_t		:4;
		uint8_t		OTSELECT:2;
		uint8_t		DRVSTRENGTH:2;
		uint8_t		FILT_ISENSE:2;
		uint32_t	:10;
	};
	uint32_t u32;
} TMC5160_REG_DRV_CONF;

typedef union
{
	struct __packed
	{
		uint8_t		IHOLD:5;
		uint8_t		:3;
		uint8_t		IRUN:5;
		uint8_t		:3;
		uint8_t		IHOLDDELAY:4;
		uint32_t	:12;
	};
	uint32_t u32;
} TMC5160_REG_IHOLD_IRUN;

typedef union
{
	struct __packed
	{
		uint8_t		TPOWERDOWN:8;
		uint32_t	:24;
	};
	uint32_t u32;
} TMC5160_REG_TPOWERDOWN;

typedef union
{
	struct __packed
	{
		uint32_t	VSTOP:18;
		uint32_t	:14;
	};
	uint32_t u32;
} TMC5160_REG_VSTOP;

typedef union
{
	struct __packed
	{
		uint16_t	TZEROWAIT;
		uint16_t	:16;
	};
	uint32_t u32;
} TMC5160_REG_TZEROWAIT;

typedef union
{
	int32_t ENC_CONST;
	uint32_t u32;
} TMC5160_REG_ENC_CONST;

typedef union
{
	struct __packed
	{
		uint8_t		TOFF:4;
		uint8_t		HRST:3;
		uint8_t		HEND0:1;
		uint8_t		HEND13:3;
		uint8_t		FD3:1;
		uint8_t		DISFDCC:1;
		uint8_t		:1;
		uint8_t		CHM:1;
		uint8_t		TBL0:1;
		uint8_t		TBL1:1;
		uint8_t		:1;
		uint8_t		VHIGHFS:1;
		uint8_t		VHIGHCHM:1;
		uint8_t		TPFD:4;
		uint8_t		MRES:4;
		uint8_t		INTPOL:1;
		uint8_t		DEDGE:1;
		uint8_t		DISS2G:1;
		uint8_t		DISS2VS:1;
	};
	uint32_t u32;
} TMC5160_REG_CHOPCONF;

typedef union
{
	struct __packed
	{
		uint8_t		PWM_OFS;
		uint8_t		PWM_GRAD;
		uint8_t		PWM_FRQ:2;
		uint8_t		PWM_AUTOSCALE:1;
		uint8_t		PWM_AUTOGRAD:1;
		uint8_t		FREEWHEEL:2;
		uint8_t		:2;
		uint8_t		PWM_REG:4;
		uint8_t		PWM_LIM:4;
	};
	uint32_t u32;
} TMC5160_REG_PWMCONF;

// Structure Write Data
typedef struct __packed
{
  unsigned int	u7Address:7;
  unsigned int	u1W:1;
  uint32_t		u32Data;
} TMC5160_SPI_TX_s;


// Structure Status
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
void TMC5160_Write(uint8_t u8Addr, uint32_t u32Data);
void TMC5160_Read(uint8_t u8Addr);
void TMC5160_ReadAll();
void TMC5160_MoveTo(int32_t s32Position);

#endif /* __TMC5160_H__ */
