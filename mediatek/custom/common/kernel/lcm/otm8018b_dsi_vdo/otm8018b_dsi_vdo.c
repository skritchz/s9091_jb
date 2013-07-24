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
	LCM_PRINT("[OTM8018B] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)
#else
#define LCM_DBG(fmt, arg...) do {} while (0)
#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)


#define LCM_ID_OTM8018B	0x8009

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
	0x00,	1,		0x00,
	0xff,	3,		0x80, 0x09, 0x01,
		     	  			
	0x00,	1,		0x80,
	0xff,	2,		0x80, 0x09,
            	
	0x00,	1,		0x03,
	0xff,	1,		0x01,

	0x00,	1,		0x00,
	0xD8,	2,		0x87,0X87,
            	
	0x00,	1,		0xB1,
	0xC5,	1,		0xA9,
            	
	0x00,	1,		0x00,
	0xD9,	1,		0x20,
            	
	0x00,	1,		0x90,
	0xC5,	3,		0x96, 0xA7,0X01,
	
	0x00,	1,		0x81,
	0xC1,	1,		0x66,

       0x00,	1,		0xA1,
	0xC1,	3,		0x08,0X02,0X1B,//0X08

       0x00,	1,		0x81,
	0xC4,	1,		0x83,
	
	0x00,	1,		0x90,
	0xB3,	1,		0x02,
	
	0x00,	1,		0x92,
	0xB3,	1,		0x45,

       0x00,	1,		0xA7,
	0xB3,	1,		0x00,
	
	0xB4,	1,		0x00,
	0xC0,	1,		0x10,
	
	0x00,	1,		0x00,
	0x36,	1,		0x00,
	
	0x00,	1,		0x90,
	0xC0,	6,		0x00, 0x44, 0x00, 0x00, 0x00,0X03,
	0x00,	1,		0xA6,
	0xC1,	3,		0x00, 0x00, 0x00,
	
	0x00,	1,		0x80,
	0xCE,	6,		0x87, 0x03, 0x00, 0x86,0x03, 0x00,
	0x00,	1,		0x90,
	0xCE,	6,		0x33, 0x54, 0x00,0x33,0x55, 0x00,
	0x00,	1,		0xA0,
	0xCE,	14,		0x38, 0x03, 0x03, 0x58,0x00, 0x00, 0x00, 0x38,0x02, 0x03, 0x59, 0x00,0x00, 0x00,
	0x00,	1,		0xB0,
	0xCE,	14,		0x38, 0x01, 0x03, 0x5A,0x00, 0x00, 0x00, 0x38,0x00, 0x03, 0x5B, 0x00,0x00, 0x00,
	0x00,	1,		0xC0,
	0xCE,	14,		0x30, 0x00, 0x03, 0x5C,0x00, 0x00, 0x00, 0x30,0x01, 0x03, 0x5D, 0x00,0x00, 0x00,
	0x00,	1,		0xD0,
	0xCE,	14,		0x38, 0x05, 0x03, 0x5E,0x00, 0x00, 0x00, 0x38,0x04, 0x03, 0x5F, 0x00,0x00, 0x00,
	0x00,	1,		0xC0,
	0xCF,	10,		0x0, 0x0, 0x0, 0x0,0x0, 0x0, 0x01, 0x80,0x0, 0x09,
	0x00,	1,		0xC0,
	0xCB,	15,		0x0, 0x4, 0x4, 0x4, 0x4, 0x0, 0x0,0x04, 0x04,0X04,0X04,0X00,0X00,0X00,0X00,
	0x00,	1,		0xD0,
	0xCB,	15,		0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04,
	0x00,	1,		0xE0,
	0xCB,	10,		0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0x80,
	0xCC,	10,		0x00,0x26,0x25,0x02,0x06,0x00,0x00,0x0A,0x0E,0x0C,
	0x00,	1,		0x90,
	0xCC,	15,		0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x25,0x01,0x05,
	0x00,	1,		0xA0,
	0xCC,	15,		0x00,0x00,0x09,0x0D,0x0B,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0xB0,
	0xCC,	10,		0x00,0x25,0x26,0x05,0x01,0x00,0x00,0x0D,0x09,0x0B,
	0x00,	1,		0xC0,
	0xCC,	15,		0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x26,0x06,0x02,
	0x00,	1,		0xD0,
	0xCC,	15,		0x00,0x00,0x0E,0x0A,0x0C,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0x00,
	0xE1,	16,		0x06,0x07,0x0A,0x0A,0x04,0x16,0x0F,0x0F,0x00,0x04,0x03,0x07,0x0E,0x21,0x1D,0x0B,
	0x00,	1,		0x00,
	0xE2,	16,		0x06,0x07,0x0A,0x0A,0x04,0x17,0x0F,0x0F,0x00,0x04,0x02,0x07,0x0F,0x22,0x1E,0x0B,

	0x00,	1,		0x00,
	0xFF,	3,		0xFF,0XFF,0XFF,

	// Setting ending by predefined flag
	REGFLAG_END_OF_TABLE
};

static unsigned char lcm_sleep_out_setting[] = {
	// Sleep Out
	0x11, 0, 
	REGFLAG_DELAY, 120,

	// Display ON
	0x29, 0,
	0x2C,      0,
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
			MDELAY(pSetting_item->count);
			p += 2;
		break;

		case REGFLAG_END_OF_TABLE :
			p += 2;
			bExit = 1;
		break;

		default:
			dsi_set_cmdq_V2(pSetting_item->cmd, 
							pSetting_item->count, pSetting_item->params, 1);
			//MDELAY(2);
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

	params->dbi.te_mode = LCM_DBI_TE_MODE_DISABLED;
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

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 16;
	params->dsi.vertical_frontporch					= 15;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 6;
	params->dsi.horizontal_backporch				= 44;
	params->dsi.horizontal_frontporch				= 46;
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

#if !defined(BUILD_UBOOT) && !defined(BUILD_LK) && defined(LCM_DSI_CMD_MODE)
//For CMD mode only
static unsigned int lcm_esd_check(void)
{
	static unsigned int lcm_esd_test = 0;      ///only for ESD test
	unsigned int esd_tag = 0;
	if(lcm_esd_test)
	{
		lcm_esd_test = 0;
		return 1;
	}

	/// please notice: the max return packet size is 1
	/// if you want to change it, you can refer to the following marked code
	/// but read_reg currently only support read no more than 4 bytes....
	/// if you need to read more, please let BinHan knows.
	/*
	unsigned int data_array[16];
	unsigned int max_return_size = 1;

	data_array[0]= 0x00003700 | (max_return_size << 16);    

	dsi_set_cmdq(&data_array, 1, 1);
	*/

	esd_tag = read_reg(0x0A);
	if( esd_tag == 0x9C 
		/*&& read_reg(0x0B) == 0x00 
		   && read_reg(0x0C) == 0x07 
		   && read_reg(0x0D) == 0x00*/)
	{
		printk("lcm_esd_check false\n");       
		return 0;
	}
	else
	{            
		return 1;
	}
}

static unsigned int lcm_esd_recover(void)
{
	unsigned char para = 0;
	printk("lcm_esd_recover ...\n");       

	lcm_init();
	push_table(lcm_sleep_out_setting);

	return 1;
}
#endif

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

static unsigned int lcm_compare_id(void)
{
	int array[4];
	char buffer[5];
	char id_high=0;
	char id_low=0;
	int id=0;

	lcm_reset();//must be ahead of this function.
		
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	if(!lcm_adc_read_chip_id())
		return 0;
#endif

	array[0] = 0x00053700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xa1, buffer, 5);

	id_high = buffer[2];
	id_low = buffer[3];
	id = (id_high<<8) | id_low;

#if defined(BUILD_UBOOT) || defined(BUILD_LK)
		printf("OTM8018B %s id = 0x%08x \n", __func__, id);
#endif

	return (LCM_ID_OTM8018B == id)?1:0;
}

LCM_DRIVER otm8018b_dsi_vdo_lcm_drv = 
{
    .name			= "otm8018b_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.compare_id    = lcm_compare_id,	
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
#if defined(LCM_DSI_CMD_MODE)
	.esd_check   = lcm_esd_check,
	.esd_recover   = lcm_esd_recover,
#endif
	.get_initialization_settings = get_initialization_settings,
	.set_initialization_settings = set_initialization_settings,
#endif
};
