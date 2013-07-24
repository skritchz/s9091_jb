/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#else
#include <debug.h>
#include <string.h>
#endif

#define LCM_DEBUG 1
#ifdef LCM_DEBUG
#if defined BUILD_UBOOT
  #include <common.h>
  #if defined(MT6575)
    #include <asm/arch/mt6577_gpio.h>
  #elif defined(CONFIG_ARCH_MT6573)
    #include <asm/arch/mt6573_gpio.h>
  #endif
  #include "cust_gpio_usage.h"
#elif defined(BUILD_LK)
#else
  #include <linux/kernel.h>
  #include <asm/errno.h>
  #if defined(MT6575)
    #include <mach/mt6575_gpio.h>
  #elif defined(CONFIG_ARCH_MT6573)
    #include <mach/mt6573_gpio.h>
  #endif
  #include <cust_gpio_usage.h>
 // #include <linux/delay.h>	
#endif
#endif

#include "lcm_drv.h"


#define LCM_ID1 0x02
#define LCM_ID2 0x04

#define LCM_ID3 0x94
#define LCM_ID4 0x81
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (320)
#define FRAME_HEIGHT (480)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

static unsigned int lcm_compare_id();

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

static __inline unsigned int HIGH_BYTE(unsigned int val)
{
    return (val >> 8) & 0xFF;
}

static __inline unsigned int LOW_BYTE(unsigned int val)
{
    return (val & 0xFF);
}

static __inline void send_ctrl_cmd(unsigned int cmd)
{
    lcm_util.send_cmd(cmd);
}

static __inline void send_data_cmd(unsigned int data)
{
    lcm_util.send_data(data);
}

static __inline unsigned int read_data_cmd()
{
    return lcm_util.read_data();
}

static void lcm_update(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
static void init_lcm_registers(void);
#if 0//TCL
static unsigned char lcm_initialization_setting[LCM_INIT_TABLE_SIZE_MAX] = {
/*	cmd,		count,	params*/
	0xC0,	2,		0x13, 0x13,
	0xC1, 	1,		0x41,
	0xC2, 	1,		0x33,
	0xC5,	2,		0x00, 0x29,
	0xB1,	2,		0xA0, 0x11,/*62Hz*/
	//0xB1,	2,		0xC0, 0x11,/*81Hz*/
	0xB4, 	1,		0x02,
	0xB7, 	1,		0x86,
	0xE0,	15,		0x0F, 0x24, 0x1F, 0x0C, 0x10, 0x09, 0x4c, 0x97, 0x3B, 0x0C, 0x1a, 0x0A, 0x12, 0x0F, 0x00,
	0XE1,	15,		0x1F, 0x30, 0x2D, 0x05, 0x1F, 0x03, 0x44, 0x86, 0x33, 0x06, 0x0F, 0x03, 0x20, 0x17, 0x00,
 	0XF2,	9,		0x18, 0xA3, 0x12, 0x02, 0xB2, 0x12, 0xFF, 0x10, 0x00,
	0XF1,	6,		0x36, 0x04, 0x00, 0x3C, 0x0F, 0x8F, 

	0xF8,	2,		0x21, 0x04,
	0x35,	1,		0x00,/*LCM_DBI_TE_MODE_VSYNC_ONLY*/
	0x44,	2,		0x00, 0xFF,/*TE start line*/
	0x36, 	1,		0x48,
	0x3A, 	1,		0x66,

	0xF9,	2,		0x00, 0x08,
	0x11,	0,
	REGFLAG_DELAY, 120,
	0x29,	0,
	0x2C,	0,
	REGFLAG_END_OF_TABLE
};
#else//The same as TINNO LCM except TE scan line.
static unsigned char lcm_initialization_setting[LCM_INIT_TABLE_SIZE_MAX] = {
	0xF1,	6,		0x36,0x04,0x00,0x3C,0x0F,0x8F, 
	0xF2,	9,		0x18, 0xA3, 0x12, 0x02, 0xB2, 0x12, 0xFF, 0x10, 0x00, 
	0xF7,	5,		0xA9, 0x91, 0x2D, 0x0A, 0x4F, 
	0xF8,	2,		0x21, 0x04, 
	0xF9,	2,		0x00,0x08,    				//add new
	0x36,	1,		0x48,					//set address mode
	0x3A,	1,		0x66, 					//pixel format, 0x55-16bits,0x66-18bits
	
	//gamma setting  2.2
	0xE0,	15,		0x00,0x00,0x09,0x08,0x13,0x08,0x32,0x9A,0x44,0x07,0x0B,0x06,0x19,0x1C,0x0F,
	//Set Gamma N
	0xE1,	15,		0x00,0x23,0x26,0x09,0x14,0x07,0x3B,0x56,0x4D,0x07,0x0F,0x07,0x36,0x3F,0x0F,
	
	0xB4,	1,		0x02,   					//Set 2Dot Invertion
	0xB1,	2,		0xB0,0x11,				//Set Frame Rate
	0x20,	0,								//Display Inversion OFF
	0xC5,	2,		0x00, 0x3E,
	0xC0,	2,		0x08, 0x0C, 
	0xC1,	1,		0x41,  
	0x35,	1,		0x00,					//Set TE register
	0x44,	2,		0x00,0x5F,  				// Set TE signal delay scanline
	0x2A,	4,		0x00,0x00,0x01,0x3F, 		//display area column setting
	0x2B,	4,		0x00,0x00,0x01,0xDF, 		//display area page setting

	0x11,	0,
	REGFLAG_DELAY, 150,
	0x29,	0,
	REGFLAG_DELAY, 20,
	0x2C,	0,
	REGFLAG_END_OF_TABLE
};

#endif
#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
static int _get_initialization_settings(unsigned char table[])
{
	memcpy(table, lcm_initialization_setting, sizeof(lcm_initialization_setting));
	return sizeof(lcm_initialization_setting);
}

static int _set_initialization_settings(const unsigned char table[], const int count)
{
	if ( count > LCM_INIT_TABLE_SIZE_MAX ){
		return -EIO;
	}
	memset(lcm_initialization_setting, REGFLAG_END_OF_TABLE, sizeof(lcm_initialization_setting));
	memcpy(lcm_initialization_setting, table, count);

	init_lcm_registers();

	return count;
}
#endif

static int push_table(unsigned char table[])
{
	unsigned int i, bExit = 0;
	unsigned char *p = (unsigned char *)table;
	LCM_SETTING_ITEM *pSetting_item;

	while(!bExit) {
		pSetting_item = (LCM_SETTING_ITEM *)p;

#if defined(BUILD_UBOOT) || defined (BUILD_LK)
//		printf("\n push_table : cmd=0x%02x, count=%d, params=", pSetting_item->cmd, pSetting_item->count);
#endif

		switch (pSetting_item->cmd) {
			
			case REGFLAG_DELAY :
				MDELAY(pSetting_item->count);
				p += 2;
			break;

			case REGFLAG_END_OF_TABLE :
#if defined(BUILD_UBOOT) || defined (BUILD_LK)
//				printf("END\n");
#endif
				p += 2;
				bExit = 1;
			break;

			default:
				send_ctrl_cmd(pSetting_item->cmd);
				for ( i = 0; i < pSetting_item->count; i++ ){
#if defined(BUILD_UBOOT) || defined (BUILD_LK)
//					printf("0x%02x, ", pSetting_item->params[i]);
#endif
					send_data_cmd( pSetting_item->params[i] );
				}
				p += pSetting_item->count + 2;
			break;
		}
	}
	return p - table; //return the size of  settings array.
}

static void init_lcm_registers(void)
{
#if defined(BUILD_UBOOT) || defined (BUILD_LK)
	printf("init_lcm_registers : ...\n");
#endif
	push_table(lcm_initialization_setting);
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DBI;
    params->ctrl   = LCM_CTRL_PARALLEL_DBI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
    params->io_select_mode = 3;

    params->dbi.port                    = 0;
    params->dbi.clock_freq              = LCM_DBI_CLOCK_FREQ_104M;
    params->dbi.data_width              = LCM_DBI_DATA_WIDTH_18BITS;
    params->dbi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dbi.data_format.trans_seq   = LCM_DBI_TRANS_SEQ_MSB_FIRST;
    params->dbi.data_format.padding     = LCM_DBI_PADDING_ON_LSB;
    params->dbi.data_format.format      = LCM_DBI_FORMAT_RGB666;
    params->dbi.data_format.width       = LCM_DBI_DATA_WIDTH_18BITS;
    params->dbi.cpu_write_bits          = LCM_DBI_CPU_WRITE_32_BITS;
    params->dbi.io_driving_current      = LCM_DRIVING_CURRENT_8MA;

	//fmark
    params->dbi.te_mode                 = LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity        = LCM_POLARITY_FALLING;
	//fmark

    params->dbi.parallel.write_setup    = 3;   // RS TO WR LOW
    params->dbi.parallel.write_hold     = 3;   // WR TO CS HOLD
    params->dbi.parallel.write_wait     = 8;    // 
	params->dbi.parallel.read_setup     = 3;
	params->dbi.parallel.read_latency   = 32;
	params->dbi.parallel.wait_period    = 4;



//    params->dbi.te_mode			= LCM_DBI_TE_MODE_DISABLED;
    
}

static void lcm_reset(void)
{
	SET_RESET_PIN(0);
	MDELAY(2);
	SET_RESET_PIN(1);
	MDELAY(128);

}

static void lcm_init(void)
{
    init_lcm_registers();
}

static void lcm_suspend(void)
{
    send_ctrl_cmd(0x28);
    send_ctrl_cmd(0x10);
    MDELAY(120);
}

static void lcm_resume(void)
{
    send_ctrl_cmd(0x11);
    MDELAY(120);
    send_ctrl_cmd(0x29);
	MDELAY(40); //modify by alik bug fix:SSFZF-57 
}

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
    unsigned int x0 = x;
    unsigned int y0 = y;
    unsigned int x1 = x0 + width - 1;
    unsigned int y1 = y0 + height - 1;
	send_ctrl_cmd(0x2A);
	send_data_cmd(HIGH_BYTE(x0));
	send_data_cmd(LOW_BYTE(x0));
	send_data_cmd(HIGH_BYTE(x1));
	send_data_cmd(LOW_BYTE(x1));

	send_ctrl_cmd(0x2B);
	send_data_cmd(HIGH_BYTE(y0));
	send_data_cmd(LOW_BYTE(y0));
	send_data_cmd(HIGH_BYTE(y1));
	send_data_cmd(LOW_BYTE(y1));

	// Write To GRAM
	send_ctrl_cmd(0x2C);
}

#if defined(BUILD_UBOOT) || defined(BUILD_LK)
#include "cust_adc.h"
#define LCM_MAX_VOLTAGE 2000 
#define LCM_MIN_VOLTAGE 1600 

extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

static unsigned int lcm_adc_read_chip_id()
{
	int data[4] = {0, 0, 0, 0};
	int tmp = 0, rc = 0, iVoltage = 0;
	rc = IMM_GetOneChannelValue(AUXADC_LCD_ID_CHANNEL, data, &tmp);
	if(rc < 0) {
		printf("read LCD_ID vol error--Liu\n");
		return 0;
	}
	else {
		iVoltage = (data[0]*1000) + (data[1]*10) + (data[2]);
		printf("read LCD_ID success, data[0]=%d, data[1]=%d, data[2]=%d, data[3]=%d, iVoltage=%d\n", 
			data[0], data[1], data[2], data[3], iVoltage);
		if(	LCM_MIN_VOLTAGE < iVoltage &&
			iVoltage < LCM_MAX_VOLTAGE)
			return 1;
		else
			return 0;
	}
	return 0;
}
#endif	


static unsigned int lcm_compare_id()
{
    int para1,para2,para3,para4,para5,para6;

	lcm_reset();//must be ahead of this function.

#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	if(lcm_adc_read_chip_id())
		return 1;
	else
		return 1; //It's a default vlaue in order to power up system when it have no LCM coupled to board.
#endif

    send_ctrl_cmd(0xD3);
    para1 = read_data_cmd();	//Dummy
    para2 = read_data_cmd();
    para3 = read_data_cmd();
    para4 = read_data_cmd();
#ifdef LCM_DEBUG
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
    printf("il9481 LCD read id1 = %x \n",para1);		//0x00
    printf("il9481 LCD read id2 = %x \n",para2);		//0x02
    printf("il9481 LCD read id3 = %x \n",para3);		//0x04
    printf("il9481 LCD read id4 = %x \n",para4);		//0x94
    printf("il9481 LCD read id5 = %x \n",para5);		//0x81
    printf("il9481 LCD read id6 = %x \n",para6);		//0xff
#else
    printk("il9481 LCD read id1 = %x \n",para1);		//0x00
    printk("il9481 LCD read id2 = %x \n",para2);		//0x02
    printk("il9481 LCD read id3 = %x \n",para3);		//0x04
    printk("il9481 LCD read id4 = %x \n",para4);		//0x94
    printk("il9481 LCD read id5 = %x \n",para5);		//0x81
    printk("il9481 LCD read id6 = %x \n",para6);		//0xff
#endif
#endif


    para1 = para1 & 0xFF;
    para2 = para2 & 0xFF;
    para3 = para3 & 0xFF;
    para4 = para4 & 0xFF;
    para5 = para5 & 0xFF;
    para6 = para6 & 0xFF;
	
    if ( (para2 == 0x00) && (para3 == 0x94) && (para4 == 0x86)/* && (para5 == LCM_ID4)*/)
	return 1;
    else
	return 0;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER ili9481_18bits_lcm_drv =
{
    .name           = "ili9486(18-bits)",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .update         = lcm_update,
    .compare_id     = lcm_compare_id,
#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
    .get_initialization_settings = _get_initialization_settings,
    .set_initialization_settings = _set_initialization_settings,
#endif
};
