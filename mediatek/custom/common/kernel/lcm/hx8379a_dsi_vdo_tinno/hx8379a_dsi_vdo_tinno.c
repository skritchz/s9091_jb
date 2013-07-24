#ifdef BUILD_LK
#define LCM_PRINT printf
#else
#include <linux/string.h>

#if defined(BUILD_UBOOT)
	#include <asm/arch/mt6577_gpio.h>
	#define LCM_PRINT printf
	#ifndef KERN_INFO
		#define KERN_INFO
	#endif
#else
	#include <linux/kernel.h>
	#include <mach/mt6577_gpio.h>
	#define LCM_PRINT printk
#endif
#endif
#if 0
#define LCM_DBG(fmt, arg...) \
	LCM_PRINT("[HX8379A] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)
#else
#define LCM_DBG(fmt, arg...) do {} while (0)
#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)


#define LCM_ID_HX8379A	0x8379

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)									lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

static unsigned char lcm_initialization_setting[LCM_INIT_TABLE_SIZE_MAX] = {
/*	cmd,		count,	params*/
	0xB9,	3, 		0XFF,0X83,0X79,
	REGFLAG_DELAY, 10,
	0xB1,	31,		0x00,0x50,0x44,0xEA,0x90,0x08,0x11,0x11,0x71,0x2F,
					0x37,0x9A,0x1A,0x42,0x1B,0x6E,0xF1,0x00,0xE6,0xE6,
					0xE6,0xE6,0xE6,0x00,0x04,0x05,0x0A,0x0B,0x04,0x05,0x6F, 
	REGFLAG_DELAY, 10,
	0xBA,	18,		0x51,0xA3,0x00,0x16,0xA4,0x00,0x18,0xFF,0x0F,0x29,
					0x03,0x21,0x23,0x25,0x20,0x02,0x35,0x40,
	REGFLAG_DELAY, 10,
	0xB2,	13,		0x00,0x00,0xFE,0x08,0x0C,0x19,0x22,0x00,0xFF,0x08,
					0x0C,0x19,0x20,
	REGFLAG_DELAY, 10,
	0xB4,	31,		0x80,0x00,0x00,0x32,0x10,0x03,0x32,0x13,0x5F,0x32,
					0x10,0x08,0x35,0x01,0x28,0x07,0x37,0x00,0x3F,0x08,
					0x30,0x30,0x04,0x00,0x40,0x08,0x28,0x08,0x30,0x30,0x04,
	REGFLAG_DELAY, 10,
	0xD5,	47,		0x00,0x00,0x0A,0x00,0x01,0x05,0x00,0x00,0x18,0x88,
					0x99,0x88,0x01,0x45,0x88,0x88,0x01,0x45,0x23,0x67,
					0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x99,0x54,0x10,
					0x88,0x88,0x76,0x32,0x54,0x10,0x88,0x88,0x88,0x88,
					0x88,0x00,0x00,0x00,0x00,0x00,0x00, 
	REGFLAG_DELAY, 10,
	0xDE,	3, 		0x05,0x70,0x04,
	REGFLAG_DELAY, 10,
	0xE0,	35,		0x79,0x07,0x12,0x14,0x3F,0x3F,0x3F,0x25,0x4F,0x06,
					0x0C,0x0E,0x12,0x17,0x12,0x13,0x14,0x1F,0x07,0x12,
					0x14,0x3F,0x3F,0x3F,0x25,0x4F,0x06,0x0C,0x0E,0x12,
					0x17,0x12,0x13,0x14,0x1F,
	REGFLAG_DELAY, 10,
	0x53,	1, 		0X40,
	REGFLAG_DELAY, 10,
	0xCC,	1, 		0X02,

	REGFLAG_DELAY, 10,
	// Setting ending by predefined flag
	REGFLAG_END_OF_TABLE
};

static unsigned char lcm_sleep_out_setting[] = {
	// Sleep Out
	0x11, 0, 
	REGFLAG_DELAY, 120,

	// Display ON
	0x29,	0,
//	REGFLAG_DELAY, 20,/*It needn't timeout anymore in Video mode.---Jieve*/
	
	REGFLAG_END_OF_TABLE
};

static unsigned char lcm_sleep_mode_in_setting[] = {
	// Display off sequence
	0x28, 0,

	// Sleep Mode On
	0x10, 0, 

	REGFLAG_END_OF_TABLE
};

static int push_table(unsigned char table[])
{
	unsigned int i, bExit = 0;
	unsigned char *p = (unsigned char *)table;
	LCM_SETTING_ITEM *pSetting_item;

	while(!bExit) {
		pSetting_item = (LCM_SETTING_ITEM *)p;

		switch (pSetting_item->cmd) {
			
		case REGFLAG_DELAY :
			UDELAY(pSetting_item->count*1000);
			p += 2;
		break;

		case REGFLAG_END_OF_TABLE :
			p += 2;
			bExit = 1;
		break;

		default:
			dsi_set_cmdq_V2(pSetting_item->cmd, 
							pSetting_item->count, pSetting_item->params, 1);
			//MDELAY(10);
			p += pSetting_item->count + 2;
		break;
		}
	}
	return p - table; //return the size of  settings array.
}

#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
static int get_initialization_settings(unsigned char table[])
{
	memcpy(table, lcm_initialization_setting, sizeof(lcm_initialization_setting));
	return sizeof(lcm_initialization_setting);
}

static void lcm_init(void);

static int set_initialization_settings(const unsigned char table[], const int count)
{
	if ( count > LCM_INIT_TABLE_SIZE_MAX ){
		return -EIO;
	}
	memset(lcm_initialization_setting, REGFLAG_END_OF_TABLE, sizeof(lcm_initialization_setting));
	memcpy(lcm_initialization_setting, table, count);

	lcm_init();
	push_table(lcm_sleep_out_setting);

	return count;
}
#endif

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

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;
	params->dsi.force_dcs_packet=1;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 5;
	params->dsi.vertical_backporch					= 6;
	params->dsi.vertical_frontporch					= 5;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 32;
	params->dsi.horizontal_backporch				= 32;
	params->dsi.horizontal_frontporch				= 32;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	/* ESD or noise interference recovery For video mode LCM only. */
	// Send TE packet to LCM in a period of n frames and check the response.
	params->dsi.lcm_int_te_monitor = 0;
	params->dsi.lcm_int_te_period = 1;		// Unit : frames

	// Need longer FP for more opportunity to do int. TE monitor applicably.
	if(params->dsi.lcm_int_te_monitor)
		params->dsi.vertical_frontporch *= 2;
	
	// Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.)
	params->dsi.lcm_ext_te_monitor = 0;
	// Non-continuous clock
	params->dsi.noncont_clock = 1;
	params->dsi.noncont_clock_period = 2;	// Unit : frames

	// Bit rate calculation
	params->dsi.pll_div1=24;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	params->dsi.pll_div2=1;			// div2=0~15: fout=fvo/(2*div2)
	//params->dsi.LPX      = 13; //read =7-12, write=13
}

static void lcm_reset(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(128);
}

static void lcm_init(void)
{
	LCM_DBG();
	lcm_reset();
	push_table(lcm_initialization_setting);
}

static void lcm_suspend(void)
{
	LCM_DBG();
	lcm_reset();
	push_table(lcm_sleep_mode_in_setting);
}

static void lcm_resume(void)
{
	LCM_DBG();
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	lcm_reset();
#endif	
	push_table(lcm_initialization_setting);
	push_table(lcm_sleep_out_setting);
}


#if defined(BUILD_UBOOT) || defined(BUILD_LK)
#include "cust_adc.h"
#define LCM_MAX_VOLTAGE 400 
#define LCM_MIN_VOLTAGE 0 

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
		if(iVoltage < LCM_MAX_VOLTAGE)
			return 1;
		else
			return 0;
	}
	return 0;
}
#endif	

static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

	lcm_reset();//must be ahead of this function.
		
	return 1;//Default LCM, no need to check anything.
	
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	if(lcm_adc_read_chip_id())
		return 1;
	else
		return 0;
#endif

	array[0]=0x00063902;
	array[1]=0x52aa55f0;
	array[2]=0x00000108;
	dsi_set_cmdq(array, 3, 1);
	MDELAY(10);

	array[0] = 0x00083700;
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);

	read_reg_v2(0x04, buffer, 3);
	id = buffer[0]<<8 |buffer[1]; 

#if defined(BUILD_UBOOT) || defined(BUILD_LK)
		printf("HX8379A %s id = 0x%08x \n", __func__, id);
#endif

	return (LCM_ID_HX8379A == id)?1:0;
}

LCM_DRIVER hx8379a_dsi_vdo_tinno_lcm_drv = 
{
    .name			= "hx8379a_vdo_tinno",
	.set_util_funcs = lcm_set_util_funcs,
	.compare_id    = lcm_compare_id,	
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
	.get_initialization_settings = get_initialization_settings,
	.set_initialization_settings = set_initialization_settings,
#endif
};
