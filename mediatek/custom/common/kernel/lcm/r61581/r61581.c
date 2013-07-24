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

#include "lcm_drv.h"
#define LCM_DEBUG  1
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
#if defined(MT6575)
    #include <mach/mt6575_gpio.h>
#elif defined(CONFIG_ARCH_MT6573)
    #include <mach/mt6573_gpio.h>
#endif
    #include <cust_gpio_usage.h>
   // #include <linux/delay.h>	
#endif
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
//#define LCM_ID1 0x01
//#define LCM_ID2 0x22
#define LCM_ID3 0x15
#define LCM_ID4 0x81

#define FRAME_WIDTH  (320)
#define FRAME_HEIGHT (480)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

static int flicker = 0x0040 ;

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n)  lcm_util.udelay(n)
#define MDELAY(n)  lcm_util.mdelay(n)

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

static __inline void set_lcm_register(unsigned int regIndex,
                                      unsigned int regData)
{
    send_ctrl_cmd(regIndex);
    send_data_cmd(regData);
}

//edit by Magnum declare lcm_update
static void lcm_update(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
static void init_lcm_registers(void)
{
#if 1

	send_ctrl_cmd(0xFF);
	send_ctrl_cmd(0xFF);
	MDELAY(5);
	send_ctrl_cmd(0xFF);
	send_ctrl_cmd(0xFF);
	send_ctrl_cmd(0xFF);
	send_ctrl_cmd(0xFF);
	MDELAY(10);

	send_ctrl_cmd(0x11);
	MDELAY(150);

	send_ctrl_cmd(0xB0);
	send_data_cmd(0x00);

	send_ctrl_cmd(0xB3);
	send_data_cmd(0x02);
	send_data_cmd(0x00);
	send_data_cmd(0x00);
	send_data_cmd(0x00);

	send_ctrl_cmd(0xC0);
	send_data_cmd(0x13);
	send_data_cmd(0x3B);//480
	send_data_cmd(0x00);
	send_data_cmd(0x00);
	send_data_cmd(0x00);
	send_data_cmd(0x01);
	send_data_cmd(0x00);//NW
	send_data_cmd(0x43);

	send_ctrl_cmd(0xC1);
	send_data_cmd(0x08);// 0x08 Line inversion, 0x00 frame inversion
	send_data_cmd(0x1B);//CLOCK 0x1B=60HZ, 0x16=70Hz
	send_data_cmd(0x08);
	send_data_cmd(0x08);

	send_ctrl_cmd(0xC4);
	send_data_cmd(0x11);
	send_data_cmd(0x01);
	send_data_cmd(0x73);
	send_data_cmd(0x01);

	send_ctrl_cmd(0xC6);
	send_data_cmd(0x00);

	send_ctrl_cmd(0xC8);
	send_data_cmd(0x0F);
	send_data_cmd(0x05);
	send_data_cmd(0x14);
	send_data_cmd(0x5C);
	send_data_cmd(0x03);
	send_data_cmd(0x07);
	send_data_cmd(0x07);
	send_data_cmd(0x10);
	send_data_cmd(0x00);
	send_data_cmd(0x23);

	send_data_cmd(0x10);
	send_data_cmd(0x07);
	send_data_cmd(0x07);
	send_data_cmd(0x53);
	send_data_cmd(0x0C);
	send_data_cmd(0x14);
	send_data_cmd(0x05);
	send_data_cmd(0x0F);
	send_data_cmd(0x23);
	send_data_cmd(0x00);

	send_ctrl_cmd(0xD0);
	send_data_cmd(0x07);
	send_data_cmd(0x07);//VCI1
	send_data_cmd(0x1D);//VRH
	send_data_cmd(0x03);//BT

	send_ctrl_cmd(0xD1);
	send_data_cmd(0x03);
	send_data_cmd(0x56);//VCM 0x5B
	send_data_cmd(0x10);//VDV

	send_ctrl_cmd(0xD2);
	send_data_cmd(0x03);
	send_data_cmd(0x24);
	send_data_cmd(0x04);
	MDELAY(100);

	send_ctrl_cmd(0x35);
	send_data_cmd(0x00);//LCM_DBI_TE_MODE_VSYNC_ONLY

	send_ctrl_cmd(0x36);
	send_data_cmd(0x00);

	send_ctrl_cmd(0x3A);
	send_data_cmd(0x66);

	send_ctrl_cmd(0x44);
	send_data_cmd(0x00);
	send_data_cmd(0xFF);

	send_ctrl_cmd(0x29);
	MDELAY(30);

	send_ctrl_cmd(0x2A);
	send_data_cmd(0x00);
	send_data_cmd(0x00);
	send_data_cmd(0x01);
	send_data_cmd(0x3F);//320

	send_ctrl_cmd(0x2B);
	send_data_cmd(0x00);
	send_data_cmd(0x00);
	send_data_cmd(0x01);
	send_data_cmd(0xDF);//480

	send_ctrl_cmd(0x2C);

#else
	send_ctrl_cmd(0x0011);
	MDELAY(150);

	send_ctrl_cmd(0x00B0);
	send_data_cmd(0x0000);

	send_ctrl_cmd(0x00B3);
	send_data_cmd(0x0002);
	send_data_cmd(0x0000);
	send_data_cmd(0x0000);
	send_data_cmd(0x0000);

	send_ctrl_cmd(0x00B4);///DPI
	send_data_cmd(0x0000);

	send_ctrl_cmd(0x00C0);
	send_data_cmd(0x0013);
	send_data_cmd(0x003B);
	send_data_cmd(0x0000);
	send_data_cmd(0x0002);
	send_data_cmd(0x0000);
	send_data_cmd(0x0001);
	send_data_cmd(0x0000);
	send_data_cmd(0x0043);

	send_ctrl_cmd(0x00C1);
	send_data_cmd(0x0008);
	send_data_cmd(0x0014);//12
	send_data_cmd(0x0008);
	send_data_cmd(0x0008);

	send_ctrl_cmd(0x00C4);
	send_data_cmd(0x0022);
	send_data_cmd(0x0006);//05
	send_data_cmd(0x0000);
	send_data_cmd(0x0000);
#ifdef LCM_DEBUG    
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
    printf("test************************** \n");		//0x0
#else
    printk("test************************** \n");		//0x0    
#endif    
#endif    
	send_ctrl_cmd(0x00C6);
	send_data_cmd(0x0002);

	send_ctrl_cmd(0x00C8);
	send_data_cmd(0x0006);//04
	send_data_cmd(0x000f);//02
	send_data_cmd(0x000c);//03
	send_data_cmd(0x0077);
	send_data_cmd(0x0008);//04
	send_data_cmd(0x000b);//10
	send_data_cmd(0x0010);//00
	send_data_cmd(0x0008);//00  05

	send_data_cmd(0x0000);
	send_data_cmd(0x0032);

	send_data_cmd(0x0008);//03  05
	send_data_cmd(0x0010);//19
	send_data_cmd(0x000b);//04
	send_data_cmd(0x0088);
	send_data_cmd(0x0007);//09
	send_data_cmd(0x0012);//16
	send_data_cmd(0x0010);//07
	send_data_cmd(0x0006);//03

	send_data_cmd(0x0032);
	send_data_cmd(0x0000);

	send_ctrl_cmd(0x002A);
	send_data_cmd(0x0000);
	send_data_cmd(0x0000);
	send_data_cmd(0x0001);
	send_data_cmd(0x003F);

	send_ctrl_cmd(0x002B);
	send_data_cmd(0x0000);
	send_data_cmd(0x0000);
	send_data_cmd(0x0001);
	send_data_cmd(0x00DF);

	send_ctrl_cmd(0x0035);
	send_data_cmd(0x0000);

       // send_ctrl_cmd(0x0021);

	send_ctrl_cmd(0x003A);
	send_data_cmd(0x0066);

	send_ctrl_cmd(0x0044);
	send_data_cmd(0x0001);
	send_data_cmd(0x00D1);// 01

	send_ctrl_cmd(0x00D0);
	send_data_cmd(0x0007);
	send_data_cmd(0x0007);
	send_data_cmd(0x0016);//19
	send_data_cmd(0x0073);

	send_ctrl_cmd(0x00D1);
	send_data_cmd(0x0003);
	send_data_cmd(0x0066);//48
	send_data_cmd(0x0010);//1f

	send_ctrl_cmd(0x00D2);
	send_data_cmd(0x0003);
	send_data_cmd(0x0011);
	send_data_cmd(0x0001);

//edit by Magnum solve shine eg: shine in mp3
        send_ctrl_cmd(0x00FE);
	send_data_cmd(0x0000);
	send_data_cmd(0x0000);
	send_data_cmd(0x0000);
        send_data_cmd(0x00c0);
	send_data_cmd(0x00b0);
	send_data_cmd(0x0000);

	send_ctrl_cmd(0x0029);
	MDELAY(10);
	send_ctrl_cmd(0x002C);
    
//------------------------------
#endif
    

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
	params->dbi.data_format.padding     = LCM_DBI_PADDING_ON_MSB;
	params->dbi.data_format.format      = LCM_DBI_FORMAT_RGB666;
	params->dbi.data_format.width       = LCM_DBI_DATA_WIDTH_18BITS;
	params->dbi.cpu_write_bits          = LCM_DBI_CPU_WRITE_32_BITS;
	params->dbi.io_driving_current      = LCM_DRIVING_CURRENT_2MA;

	//fmark
	params->dbi.te_mode                 = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity        = LCM_POLARITY_FALLING; //FALLING
	//end fmark

/*
    Jieve Liu
    9.17 ns pre 1 clock @104MHz
*/
	params->dbi.parallel.write_setup    = 2;
	params->dbi.parallel.write_hold     = 2;
	params->dbi.parallel.write_wait     = 4;
	params->dbi.parallel.read_setup     = 3;
	params->dbi.parallel.read_latency   = 32;
	params->dbi.parallel.wait_period    = 4;
}

static void lcm_reset(void)
{
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(150);
}

static void lcm_init(void)
{   
#ifdef LCM_DEBUG    
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	printf("r61581 LCD DRIVER INIT \n");		//0x0
#else
	printk("r61581 LCD DRIVER INIT \n");		//0x0    
#endif    
#endif    
	//edit by Magnum  high->low->high stable
	SET_RESET_PIN(1);
	MDELAY(10);
	//end edit 
	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);
	MDELAY(20);

	init_lcm_registers();
	//edit by Magnum  to solve flash before boot logo
	//   lcm_update(0,0,320,480);
	lcm_update(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	int loop=0;
	while(loop<320*480)
	{
		send_data_cmd(0x00);
		loop++;
	}
}


static void lcm_suspend(void)
{
//	send_ctrl_cmd(0x10);	//Ivan modified

#if defined(BUILD_UBOOT) || defined(BUILD_LK)
    printf("r61581 LCD DRIVER SUSPEND! !!\n");		//0x0
#else
    printk("r61581 LCD DRIVER SUSPEND! !!\n");		//0x0    
#endif    

    send_ctrl_cmd(0x0028); 
    send_ctrl_cmd(0x0010); 
    MDELAY(120);		
}

int temp = 0x0025;
static void lcm_resume(void)
{

	send_ctrl_cmd(0x0011); 
	MDELAY(150);//120
	send_ctrl_cmd(0x0029); 
	MDELAY(20);//120
#if 0
	//init_lcm_registers();

	send_ctrl_cmd(0x00D1);
	send_data_cmd(0x0003);
	send_data_cmd(temp);//48
	send_data_cmd(0x0014);//1f

	//edit by Magnum 2011-9-28
	flicker++;
#ifdef LCM_DEBUG
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	printf("r61581 LCD DRIVER RESUME! !!*****************************************2***\n");		//0x0
#else
	printk("r61581 LCD DRIVER RESUME***************************************7***! !!\n");		//0x0    
	printk("%x\n",flicker);
	printk("flicker no=%x",flicker);		//0x0    
#endif
#endif
	init_lcm_registers();
#endif
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
#define LCM_MAX_VOLTAGE 1600 
#define LCM_MIN_VOLTAGE 1200 

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

//Ivan modified
    int para1,para2,para3,para4,para5;
    int adc_ret =0;
	lcm_reset();//must be ahead of this function.
	
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	if(lcm_adc_read_chip_id())
		return 1;
	else
		return 0;
#endif

    send_ctrl_cmd(0xB0);
    send_data_cmd(0x00);
	
    send_ctrl_cmd(0xBF);
    para1 = read_data_cmd();    //Dummy read
    para2 = read_data_cmd();
    para3 = read_data_cmd();
    para4 = read_data_cmd();
    para5 = read_data_cmd();

    if (para1 == 0xBF || para1 == 0x00)
    {
#ifdef LCM_DEBUG	
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	printf("r61581 LCD read id RESET \n");		//0x0
#else
	printk("r61581 LCD read id RESET \n");		//0x0	
#endif	
#endif
	
	lcm_init();	    

	send_ctrl_cmd(0xB0);
	send_data_cmd(0x00);
	
	send_ctrl_cmd(0xBF);
	para1 = read_data_cmd();    //Dummy read
	para2 = read_data_cmd();	//0x01
	para3 = read_data_cmd();	//0x22
	para4 = read_data_cmd();	//0x15
	para5 = read_data_cmd();	//0x81
    }
#ifdef LCM_DEBUG    
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
    printf("r61581 LCD read id1 = %x \n",para1);		//0x0
    printf("r61581 LCD read id2 = %x \n",para2);		//0x02
    printf("r61581 LCD read id3 = %x \n",para3);		//0x04
    printf("r61581 LCD read id4 = %x \n",para4);		//0x94
    printf("r61581 LCD read id5 = %x \n",para5);		//0x81
#else
    printk("r61581 LCD read id1 = %x \n",para1);		//0x0
    printk("r61581 LCD read id2 = %x \n",para2);		//0x02
    printk("r61581 LCD read id3 = %x \n",para3);		//0x04
    printk("r61581 LCD read id4 = %x \n",para4);		//0x94
    printk("r61581 LCD read id5 = %x \n",para5);		//0x81
    
#endif    
#endif
    para2 = para2 & 0xFF;
    para3 = para3 & 0xFF;
    para4 = para4 & 0xFF;
    para5 = para5 & 0xFF;

    if ((para4 == LCM_ID3) && (para5 == LCM_ID4))
	return 1;
    else
	return 0;	//Always return true for default
    
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER r61581_lcm_drv = //18bit
{
    .name           = "r61581(18-bits)",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .update         = lcm_update,
    .compare_id     = lcm_compare_id,
};
