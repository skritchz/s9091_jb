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


#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>

#include <linux/dma-mapping.h>
#include <linux/slab.h>

//add by liyaohua start
#include <linux/wakelock.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <asm/unistd.h>
#include <linux/device.h>
//add by liyaohua end


#if defined(MT6575)|| defined(MT6577)
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>
#endif

#include "cust_gpio_usage.h"

#include "msg2133.h"

//add by liyaohua 
#define MEM_MALLOC_SIZE 	4096	
#define MEM_MAJOR      999
#define MEM_MINOR    	0 

struct wake_lock  Tp_wake_lock;

bool  tfopensenor = false;

static int boot_mode;
static int g_update;

char *mem_spvm; 	 
struct class *mem_class;
static struct mutex tp_mutex;
static struct mutex readtp_mutex;
static struct mutex rwtp_mutex;






static int sensor_open = 0;
static int updating;
static int iIsMsg2133  = 1;
static int icy =0;
//add by liyaohua end
extern struct tpd_device *tpd;

static struct i2c_client *i2c_client = NULL;
static struct task_struct *mthread = NULL;

static unsigned char touch_up_num = 0;




static DECLARE_WAIT_QUEUE_HEAD(waiter);


static void tpd_eint_interrupt_handler(void);


extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
									 kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
									 kal_bool auto_umask);
int msg2133_read_fw_ver_custom(void);

//add by liyaohua start
#define ABS(X)                  ((X > 0) ? (X) : (-X))
static int  __init  register_unregister_chrdev_init(void);
static void  __exit register_unregister_chrdev_exit(void); 
static ssize_t firmware_tpdinfo_show(struct device *dev,
struct device_attribute *attr, char *buf);

static struct device_attribute dev_attr_tpd_info= {
	.attr = { .name = "tpd_info", .mode = S_IRUGO },
	.show = firmware_tpdinfo_show,
};
int tempx = 0;
int tempy = 0;

//add by liyaohua end
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);

static int tpd_flag = 0;
static int point_num = 0;
static int p_point_num = 0;
#define TPD_RES_X 320
#define TPD_RES_Y 480

/*
#define TPD_RES_X 240
#define TPD_RES_Y 320
*/

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif

//#define TPD_CLOSE_POWER_IN_SLEEP
#define TP_DEBUG
#define TP_FIRMWARE_UPDATE
//Tinno: temp skip it
#define TP_PROXIMITY_SENSOR
#define TPD_OK 0

// debug macros
#define TP_DEBUG_MSG

#ifdef TP_DEBUG_MSG
/*#define pr_tp(format, args...) printk("<MSG>" format, ##args)
#define pr_ch(format, args...)                      \
	printk("<MSG>" "%s <%d>,%s(),cheehwa_print:\n\t"  \
	format,__FILE__,__LINE__,__func__, ##arg)*/
	
#define pr_tp printk
#define pr_ch printk

#else
#define pr_tp(format, args...)  do {} while (0)
#define pr_ch(format, args...)  do {} while (0)
#define pr_k(format, args...)  do {} while (0)
#endif

#ifdef TP_PROXIMITY_SENSOR
char ps_data_state[1] = {0};
enum
{
	DISABLE_CTP_PS,
	ENABLE_CTP_PS,
	RESET_CTP_PS
};
#endif

#ifdef TP_FIRMWARE_UPDATE
#define U8 unsigned char
#define S8 signed char
#define U16 unsigned short
#define S16 signed short
#define U32 unsigned int
#define S32 signed int
#define TOUCH_ADDR_MSG21XX   (0x4C>>1)
#define FW_ADDR_MSG21XX      (0xC4>>1)
#define FW_UPDATE_ADDR_MSG21XX  0x49
static  char *fw_version;
#define DWIIC_MODE_ISP    0
#define DWIIC_MODE_DBBUS  1
#define   DOWNLOAD_FIRMWARE_BUF_SIZE   59*1024
static U8 *download_firmware_buf = NULL;


#define POWERUP_AUTO_CHECK
#ifdef POWERUP_AUTO_CHECK
static unsigned char CTPM_FW[]=
{
#include "MSG2133_S7500_20120701.i"
};
static  unsigned short msg2133_fm_major=100, msg2133_fm_minor=0;
static  unsigned short msg2133_bin_major=0, msg2133_bin_minor=0;
#endif

//static int FwVersion;
struct class *firmware_class;
struct device *firmware_cmd_dev;

static int update_switch = 0;
#define ENABLE_DMA      0
#if ENABLE_DMA
static u8 *gpDMABuf_va = NULL;
static u32 gpDMABuf_pa = NULL;

#endif

#endif

struct touch_info
{
	unsigned short y[3];
	unsigned short x[3];
	unsigned short p[3];
	unsigned short count;
};

typedef struct
{
	unsigned short pos_x;
	unsigned short pos_y;
	unsigned short pos_x2;
	unsigned short pos_y2;
	unsigned short temp2;
	unsigned short temp;
	short dst_x;
	short dst_y;
	unsigned char checksum;
} SHORT_TOUCH_STATE;


static const struct i2c_device_id msg2133_tpd_id[] = {{"msg2133", 0}, {}};

//Tinno:CJ
//new kernel not support this , you need use  i2c_board_info instand of it
//unsigned short myforce[] = {0, TOUCH_ADDR_MSG21XX, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const forces[] = { myforce, NULL };
//static struct i2c_client_address_data addr_data = { .forces = forces, };
static struct i2c_board_info __initdata msg2133_i2c_tpd={ I2C_BOARD_INFO("msg2133", (TOUCH_ADDR_MSG21XX))};


//add by  liyaohua start
static int mem_open(struct inode *ind, struct file *filp); 
static int mem_release(struct inode *ind, struct file *filp);
static ssize_t mem_read(struct file *filp, char __user *buf, size_t size, loff_t *fpos);
static ssize_t mem_write(struct file *filp, const char __user *buf, size_t size, loff_t *fpos);
void Get_Chip_Version(void);



struct file_operations mem_fops = 
{
	//.owner=THIS_MODULE,  
	.open = mem_open,
	.release = mem_release,	
	.read = mem_read,	  
	.write = mem_write,  
};
/*
static ssize_t input_dev_show_modalias(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	struct input_dev *id = to_input_dev(dev);
	ssize_t len;

	len = input_print_modalias(buf, PAGE_SIZE, id, 1);

	return min_t(int, len, PAGE_SIZE);
}
static DEVICE_ATTR(modalias, S_IRUGO, input_dev_show_modalias, NULL);

*/
unsigned char tpd_check_sum(unsigned char *pval)
{
	int i, sum = 0;

	for(i = 0; i < 7; i++)
	{
		sum += pval[i];
	}

	return (unsigned char)((-sum) & 0xFF);
}
static bool msg2133_i2c_read(char *pbt_buf, int dw_lenth)
{
	int ret;
	i2c_client->timing = 100;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	ret = i2c_master_recv(i2c_client, pbt_buf, dw_lenth);


	if(ret <= 0)
	{
		pr_tp("msg_i2c_read_interface error\n");
		return false;
	}

	return true;
}

int __init register_unregister_chrdev_init(void)
{
	int res;
	pr_tp("<0>into register_unregister_chrdev_init\n");
	mem_spvm = (char *)vmalloc(MEM_MALLOC_SIZE);  
	res=register_chrdev(MEM_MAJOR,"my_char_dev",&mem_fops);
	if(res)    //¡Á¡é2¨¢¨º¡ì¡ã¨¹
	{
		unregister_chrdev(MEM_MAJOR,"my_char_dev"); 
		pr_tp("<0>register char dev failed\n");
		return -1;
	}
	pr_tp("<0>register char dev success\n");
 	mem_class = class_create(THIS_MODULE, "my_char_dev"); 
	mutex_init(&tp_mutex);	
	mutex_init(&readtp_mutex);	
	mutex_init(&rwtp_mutex);	
	

	
  	if(IS_ERR(mem_class)) 
       {
  	      pr_tp("<0>failed in creating class.\n");
	      class_destroy(mem_class);
 	      return -1;
 	}
	pr_tp("<0>class create success\n");
    	device_create(mem_class, NULL, MKDEV(MEM_MAJOR,MEM_MINOR), NULL, "my_char_dev");  //¡Á¡é2¨¢¨¦¨¨¡À????t?¦Ì¨ª3¡ê?2¡é??¨¢¡é¨¦¨¨¡À??¨²¦Ì?
	pr_tp("<0>device create success\n");
	pr_tp("<0>out register_unregister_chrdev_init\n");
	return 0;
}

void __exit register_unregister_chrdev_exit (void)
{
	pr_tp("<0>into register_unregister_chrdev_exit\n");	
	unregister_chrdev(MEM_MAJOR,"my_char_dev"); 
	pr_tp("<0>unregister char dev success\n");
       device_destroy(mem_class, MKDEV(MEM_MAJOR,MEM_MINOR));
	pr_tp("<0>device destroy success\n");
       class_destroy(mem_class);	    
	pr_tp("<0>class destroy success\n");
	if (mem_spvm != NULL)
		vfree(mem_spvm);     
	pr_tp("<0>vfree ok!\n");
	pr_tp("<0>out register_unregister_chrdev_exit\n");
}
int mem_open(struct inode *ind, struct file *filp)
{	
	pr_tp("<0>open vmalloc space\n");
	try_module_get(THIS_MODULE); 
	pr_tp("<0>open vmalloc space success\n");
	return 0;
}

ssize_t mem_read(struct file *filp, char *buf, size_t size, loff_t *lofp)
{
	int res = -1;
	char *tmp =NULL;
	pr_tp("<0>copy data to the user space\n");
	tmp = mem_spvm;
	
	if (size > MEM_MALLOC_SIZE)  
		size = MEM_MALLOC_SIZE;
	if (tmp != NULL)
		res = copy_to_user(buf, tmp, size); 
	if (res == 0)
	{
		pr_tp("<0>copy data success and the data is:%s\n",tmp);
		return size;
	}
	else
	{
		pr_tp("<0>copy data fail to the user space\n");
		return 0;
	}
}


int mem_release(struct inode *ind, struct file *filp)
{
	pr_tp("<0>close vmalloc space\n");
	module_put(THIS_MODULE);
	pr_tp("<0>close vmalloc space success\n");
	return 0;
}

//add by liyaohua end
void msta_power(int on)
{
	pr_tp("====================================================msta_power %d===========================\n",on);
	if(on) {
	#if defined(MT6575) || defined(MT6577)
		hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_2800, "touch"); 
	#else
		mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
	#endif 
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT); 
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);  
		msleep(200);
		mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
		mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
		mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
	}
	else {
	#if defined(MT6575) || defined(MT6577)
		hwPowerDown(MT65XX_POWER_LDO_VGP, "touch"); 
	#else
		mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_IN);
		mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
	#endif
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_IN);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
		mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
	}
}


#ifdef TP_FIRMWARE_UPDATE
static int i2c_write(u8 addr, u8 *pbt_buf, int dw_lenth)
{
	int ret;
	i2c_client->addr = addr;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	ret = i2c_master_send(i2c_client, pbt_buf, dw_lenth);
	i2c_client->addr = TOUCH_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;

	if(ret <= 0)
	{
		pr_tp("i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
	}
	return ret;
}

static int i2c_read(u8 addr, u8 *pbt_buf, int dw_lenth)
{
	int ret;
	i2c_client->addr = addr;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	ret = i2c_master_recv(i2c_client, pbt_buf, dw_lenth);
	i2c_client->addr = TOUCH_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;

	if(ret <= 0)
	{
		pr_tp("i2c_read_interface error line = %d, ret = %d\n", __LINE__, ret);
	}
	return ret;
}

static void i2c_write_update_msg2133(u8 *pbt_buf, int dw_lenth)
{
	int ret;
	i2c_client->addr = FW_UPDATE_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	ret = i2c_master_send(i2c_client, pbt_buf, dw_lenth);
	i2c_client->addr = TOUCH_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;

	// ret = i2c_smbus_write_i2c_block_data(i2c_client, *pbt_buf, dw_lenth-1, pbt_buf+1);
	if(ret <= 0)
	{
		pr_tp("i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
	}
}

static void i2c_write_msg2133(u8 *pbt_buf, int dw_lenth)
{
	int ret;
	i2c_client->timing = 40;
	i2c_client->addr = FW_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	ret = i2c_master_send(i2c_client, pbt_buf, dw_lenth);
	i2c_client->addr = TOUCH_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	i2c_client->timing = 240;

	// ret = i2c_smbus_write_i2c_block_data(i2c_client, *pbt_buf, dw_lenth-1, pbt_buf+1);
	if(ret <= 0)
	{
		pr_tp("i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
	}
}
static void i2c_read_update_msg2133(u8 *pbt_buf, int dw_lenth)
{
	int ret;
	i2c_client->addr = FW_UPDATE_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	ret = i2c_master_recv(i2c_client, pbt_buf, dw_lenth);
	i2c_client->addr = TOUCH_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	//  ret=i2c_smbus_read_i2c_block_data(i2c_client, *pbt_buf, dw_lenth-1, pbt_buf+1);

	if(ret <= 0)
	{
		pr_tp("i2c_read_interface error line = %d, ret = %d\n", __LINE__, ret);
	}
}

static void i2c_read_msg2133(u8 *pbt_buf, int dw_lenth)
{
	int ret;
	i2c_client->timing = 40;
	i2c_client->addr = FW_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	ret = i2c_master_recv(i2c_client, pbt_buf, dw_lenth);
	i2c_client->addr = TOUCH_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	i2c_client->timing = 240;
	//  ret=i2c_smbus_read_i2c_block_data(i2c_client, *pbt_buf, dw_lenth-1, pbt_buf+1);

	if(ret <= 0)
	{
		pr_tp("i2c_read_interface error line = %d, ret = %d\n", __LINE__, ret);
	}
}

#if ENABLE_DMA
ssize_t msg2133_dma_read_m_byte(u8 *returnData_va, u32 returnData_pa, U16 len)
{
	char     readData = 0;
	int     ret = 0, read_count = 0, read_length = 0;
	int    i, total_count = len;

	if(len == 0)
	{
		pr_tp("[Error]msg2133_dma_read Read Len should not be zero!! \n");
		return 0;
	}

	//gpDMABuf_va = returnData_va; //mtk
	i2c_client->addr = FW_UPDATE_ADDR_MSG21XX;
	i2c_client->addr |= I2C_DMA_FLAG;
	i2c_client->addr|=I2C_ENEXT_FLAG;
	returnData_va[0] = 0x11;
	ret = i2c_master_send(i2c_client, returnData_pa, 1);

	if(ret < 0)
	{
		pr_tp("[Error]MATV sends command error!! \n");
		i2c_client->addr = TOUCH_ADDR_MSG21XX;
		i2c_client->addr|=I2C_ENEXT_FLAG;
		return 0;
	}

	ret = i2c_master_recv(i2c_client, returnData_pa, len); // mtk
	i2c_client->addr = TOUCH_ADDR_MSG21XX;
	i2c_client->addr|=I2C_ENEXT_FLAG;

	if(ret < 0)
	{
		pr_tp("[Error]msg2133_dma_read reads data error!! \n");
		return 0;
	}

	//for (i = 0; i< total_count; i++)
	//    MATV_LOGD("[MATV]I2C ReadData[%d] = %x\n",i,returnData_va[i]);
	return 1;
}

#define MAX_CMD_LEN 255
ssize_t msg2133_dma_write_m_byte(u8 *Data, U16 len)
{
	char addr_bak;
	u32 phyAddr = 0; 
	u8 *buf_dma = NULL;
	u32 old_addr = 0; 
	int ret = 0; 
	int retry = 3; 

	addr_bak = i2c_client->addr;
	i2c_client->addr = FW_UPDATE_ADDR_MSG21XX;
	i2c_client->addr |= I2C_ENEXT_FLAG;
	if (len > MAX_CMD_LEN) {
		//TPD_LOGI("[i2c_master_send_ext] exceed the max write length \n"); 
		return -1; 
	}

	phyAddr = 0; 
	buf_dma = dma_alloc_coherent(0, len, &phyAddr, GFP_KERNEL);
	if (NULL == buf_dma) {
		//TPD_LOGI("[i2c_master_send_ext] Not enough memory \n"); 
		return -1; 
	}
	memcpy(buf_dma, Data, len); 
	i2c_client->addr |= I2C_DMA_FLAG;
	i2c_client->addr|=I2C_ENEXT_FLAG;

	do {
		ret = i2c_master_send(i2c_client, (u8*)phyAddr, len);     
		retry --; 
		if (ret != len) {
			//TPD_LOGI("[i2c_master_send_ext] Error sent I2C ret = %d\n", ret); 
		}
	}while ((ret != len) && (retry > 0)); 

	dma_free_coherent(0, len, buf_dma, phyAddr);

	i2c_client->addr = addr_bak;

	return 1;	
}

U8 drvISP_DMA_Read(U8 *pDataToRead, U32 pa_addr, U8 n)    //First it needs send 0x11 to notify we want to get flash data back.
{
	//    U8 Read_cmd = 0x11;
	//    unsigned char dbbus_rx_data[2] = {0};
	//    i2c_write_update_msg2133( &Read_cmd, 1);
	msg2133_dma_read_m_byte(pDataToRead, pa_addr, n);
	return 0;
}

#endif

void Get_Chip_Version(void)
{
	unsigned char dbbus_tx_data[3];
	unsigned char dbbus_rx_data[2];
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0xCE;
	i2c_write_msg2133(&dbbus_tx_data[0], 3);
	i2c_read_msg2133(&dbbus_rx_data[0], 2);

	pr_tp("======Get_Chip_Version============%s",dbbus_rx_data);
	if(dbbus_rx_data[1] == 0)
	{
		// it is Catch2
		pr_tp("*** Catch2 ***\n");
		//FwVersion  = 2;// 2 means Catch2
	}
	else
	{
		// it is catch1
		pr_tp("*** Catch1 ***\n");
		//FwVersion  = 1;// 1 means Catch1
	}
}

void dbbusDWIICEnterSerialDebugMode(void)
{
	U8 data[5];
	// Enter the Serial Debug Mode
	data[0] = 0x53;
	data[1] = 0x45;
	data[2] = 0x52;
	data[3] = 0x44;
	data[4] = 0x42;
	i2c_write_msg2133(data, 5);
}

void dbbusDWIICStopMCU(void)
{
	U8 data[1];
	// Stop the MCU
	data[0] = 0x37;
	i2c_write_msg2133(data, 1);
}

void dbbusDWIICIICUseBus(void)
{
	U8 data[1];
	// IIC Use Bus
	data[0] = 0x35;
	i2c_write_msg2133(data, 1);
}

void dbbusDWIICIICReshape(void)
{
	U8 data[1];
	// IIC Re-shape
	data[0] = 0x71;
	i2c_write_msg2133(data, 1);
}

void dbbusDWIICIICNotUseBus(void)
{
	U8 data[1];
	// IIC Not Use Bus
	data[0] = 0x34;
	i2c_write_msg2133(data, 1);
}

void dbbusDWIICNotStopMCU(void)
{
	U8 data[1];
	// Not Stop the MCU
	data[0] = 0x36;
	i2c_write_msg2133(data, 1);
}

void dbbusDWIICExitSerialDebugMode(void)
{
	U8 data[1];
	// Exit the Serial Debug Mode
	data[0] = 0x45;
	i2c_write_msg2133(data, 1);
	// Delay some interval to guard the next transaction
	//udelay ( 200 );        // delay about 0.2ms
}

void drvISP_EntryIspMode(void)
{
	U8 bWriteData[5] =
	{
		0x4D, 0x53, 0x54, 0x41, 0x52
	};
	i2c_write_update_msg2133(bWriteData, 5);
	mdelay(10);           // delay about 1ms
}

U8 drvISP_Read(U8 n, U8 *pDataToRead)    //First it needs send 0x11 to notify we want to get flash data back.
{
	U8 Read_cmd = 0x11;
	U8 i = 0;
	unsigned char dbbus_rx_data[16] = {0};
	i2c_write_update_msg2133(&Read_cmd, 1);
	//if (n == 1)
	{
		i2c_read_update_msg2133(&dbbus_rx_data[0], n + 1);

		for(i = 0; i < n; i++)
		{
			*(pDataToRead + i) = dbbus_rx_data[i + 1];
			//pr_tp("%s===============RX_data================", &dbbus_rx_data[i + 1]);
		}
	}
	//else
	{
		//     i2c_read_update_msg2133(pDataToRead, n);
	}
	return 0;
}

void drvISP_WriteEnable(void)
{
   pr_tp("drvISP_WriteEnable start\n");
	U8 bWriteData[2] =
	{
		0x10, 0x06
	};
	U8 bWriteData1 = 0x12;
	i2c_write_update_msg2133(bWriteData, 2);
	i2c_write_update_msg2133(&bWriteData1, 1);
	pr_tp("drvISP_WriteEnable end\n");
}


void drvISP_ExitIspMode(void)
{
	U8 bWriteData = 0x24;
	i2c_write_update_msg2133(&bWriteData, 1);
}

U8 drvISP_ReadStatus(void)
{
	U8 bReadData = 0;
	U8 bWriteData[2] =
	{
		0x10, 0x05
	};
	U8 bWriteData1 = 0x12;
	i2c_write_update_msg2133(bWriteData, 2);
	drvISP_Read(1, &bReadData);
	i2c_write_update_msg2133(&bWriteData1, 1);
	return bReadData;
}


void drvISP_BlockErase(U32 addr)
{
	U8 bWriteData[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
	U8 bWriteData1 = 0x12;
	pr_tp("The drvISP_ReadStatus0=%d\n", drvISP_ReadStatus());
	drvISP_WriteEnable();
	pr_tp("The drvISP_ReadStatus1=%d\n", drvISP_ReadStatus());
	//Enable write status register
	bWriteData[0] = 0x10;
	bWriteData[1] = 0x50;
	i2c_write_update_msg2133(bWriteData, 2);
	i2c_write_update_msg2133(&bWriteData1, 1);
	//Write Status
	bWriteData[0] = 0x10;
	bWriteData[1] = 0x01;
	bWriteData[2] = 0x00;
	i2c_write_update_msg2133(bWriteData, 3);
	i2c_write_update_msg2133(&bWriteData1, 1);
	//Write disable
	bWriteData[0] = 0x10;
	bWriteData[1] = 0x04;
	i2c_write_update_msg2133(bWriteData, 2);
	i2c_write_update_msg2133(&bWriteData1, 1);
	while((drvISP_ReadStatus() & 0x01) == 0x01)
	{
	//	++i;
	}
	pr_tp("The drvISP_ReadStatus3=%d\n", drvISP_ReadStatus());
	drvISP_WriteEnable();
	pr_tp("The drvISP_ReadStatus4=%d\n", drvISP_ReadStatus());
	bWriteData[0] = 0x10;
	bWriteData[1] = 0xC7;        //Block Erase
	//bWriteData[2] = ((addr >> 16) & 0xFF) ;
	//bWriteData[3] = ((addr >> 8) & 0xFF) ;
	// bWriteData[4] = (addr & 0xFF) ;
	i2c_write_update_msg2133(bWriteData, 2);
	//i2c_write_update_msg2133( &bWriteData, 5);
	i2c_write_update_msg2133(&bWriteData1, 1);

	/*while((drvISP_ReadStatus() & 0x01) == 0x01)
	{
		;
	}*/
		while((drvISP_ReadStatus()&0x01) == 0x01)
		{
		//	++i;    //wait until not in write operation
		;
		}
}

void drvISP_Program(U16 k, U8 *pDataToWrite)
{
	pr_tp("drvISP_Program start.");
	U16 i = 0;
	U16 j = 0;
	//U16 n = 0;
	U8 TX_data[133];
	U8 bWriteData1 = 0x12;
	U32 addr = k * 1024;
#if ENABLE_DMA

	for(j = 0; j < 8; j++)    //128*8 cycle
	{
		TX_data[0] = 0x10;
		TX_data[1] = 0x02;// Page Program CMD
		TX_data[2] = (addr + 128 * j) >> 16;
		TX_data[3] = (addr + 128 * j) >> 8;
		TX_data[4] = (addr + 128 * j);

		for(i = 0; i < 128; i++)
		{
			TX_data[5 + i] = pDataToWrite[j * 128 + i];
		}

	/*	while((drvISP_ReadStatus() & 0x01) == 0x01)
		{
			;    //wait until not in write operation
		}*/
		while((drvISP_ReadStatus() & 0x01 ) == 0x01)
		{
			++i;    //wait until not in write operation
		}

		drvISP_WriteEnable();
		//        i2c_write_update_msg2133( TX_data, 133);   //write 133 byte per cycle
		msg2133_dma_write_m_byte(TX_data, 133);
		i2c_write_update_msg2133(&bWriteData1, 1);
		
	}

#else

	for(j = 0; j < 512; j++)    //128*8 cycle
	{
		TX_data[0] = 0x10;
		TX_data[1] = 0x02;// Page Program CMD
		TX_data[2] = (addr + 2 * j) >> 16;
		TX_data[3] = (addr + 2 * j) >> 8;
		TX_data[4] = (addr + 2 * j);

		for(i = 0; i < 2; i++)
		{
			TX_data[5 + i] = pDataToWrite[j * 2 + i];
		}

	/*	while((drvISP_ReadStatus() & 0x01) == 0x01)
		{
			;    //wait until not in write operation
		}*/
		while((drvISP_ReadStatus() & 0x01) == 0x01)
		{
			++i;//wait until not in write operation
		}
		pr_tp("drvISP_WriteEnable.\n");
                drvISP_WriteEnable();
		pr_tp("i2c_write_update_msg2133\n");
		i2c_write_update_msg2133(TX_data, 7);    //write 133 byte per cycle
		pr_tp("i2c_write_update_msg2133bWriteData1\n");
		i2c_write_update_msg2133(&bWriteData1, 1);
		pr_tp("=============1sleep==============\n");
		//msleep(1);
	}

#endif
pr_tp("drvISP_Program end.");

}


void drvISP_Verify(U16 k, U8 *pDataToVerify)
{
	U16 i = 0, j = 0;
	U8 bWriteData[5] =
	{
		0x10, 0x03, 0, 0, 0
	};
	U8 bWriteData1 = 0x12;
	U32 addr = k * 1024;
	U8 index = 0;
	
#if ENABLE_DMA
	U8 *RX_data = gpDMABuf_va; //mtk

	for(j = 0; j < 8; j++)    //128*8 cycle
	{
		bWriteData[2] = (U8)((addr + j * 128) >> 16);
		bWriteData[3] = (U8)((addr + j * 128) >> 8);
		bWriteData[4] = (U8)(addr + j * 128);

	/*
	   while((drvISP_ReadStatus() & 0x01) == 0x01)
		{
			;    //wait until not in write operation
		}
	*/

		i2c_write_update_msg2133(bWriteData, 5);     //write read flash addr
		drvISP_DMA_Read(gpDMABuf_va, gpDMABuf_pa, 128); //mtk
		i2c_write_update_msg2133(&bWriteData1, 1);     //cmd end

		for(i = 0; i < 128; i++)    //log out if verify error
		{
			if((RX_data[i] != 0) && index < 10)
			{
				pr_tp("j=%d,RX_data[%d]=0x%x\n", j, i, RX_data[i]);
				index++;
			}

			if(RX_data[i] != pDataToVerify[128 * j + i])
			{
				pr_tp("k=%d,j=%d,i=%d===============Update Firmware Error================", k, j, i);
			}
		}
	}

#else
	U8 RX_data[128];

	for(j = 0; j < 256; j++)    //128*8 cycle
	{
		bWriteData[2] = (U8)((addr + j * 4) >> 16);
		bWriteData[3] = (U8)((addr + j * 4) >> 8);
		bWriteData[4] = (U8)(addr + j * 4);

	/*
	    while((drvISP_ReadStatus() & 0x01) == 0x01)
		{
			;    //wait until not in write operation
		}
		*/

		i2c_write_update_msg2133(bWriteData, 5);     //write read flash addr
		drvISP_Read(4, RX_data);
		i2c_write_update_msg2133(&bWriteData1, 1);     //cmd end

		for(i = 0; i < 4; i++)    //log out if verify error
		{
			if((RX_data[i] != 0) && index < 10)
			{
				pr_tp("j=%d,RX_data[%d]=0x%x\n", j, i, RX_data[i]);
				index++;
			}

			if(RX_data[i] != pDataToVerify[4 * j + i])
			{
				pr_tp("k=%d,j=%d,RX_data[%d]=0x%x===============Update Firmware Error================", k, j, i, RX_data[i]);
			}
		}
	}

#endif
}

static ssize_t firmware_update_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", fw_version);
}


static ssize_t firmware_update_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
	

	U8 i;
	U8 dbbus_tx_data[4];
	unsigned char dbbus_rx_data[2] = {0};
	update_switch = 1;

	
	pr_ch("\n");
	//drvISP_EntryIspMode();
	//drvISP_BlockErase(0x00000);
	//M by cheehwa _HalTscrHWReset();

	pr_tp("firmware_update_store start\n");
	 
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);		
	msta_power(1);
	msleep(200);

	dbbusDWIICEnterSerialDebugMode();
	pr_ch("dbbusDWIICEnterSerialDebugMode\n");
	dbbusDWIICStopMCU();
	pr_ch("dbbusDWIICStopMCU\n");
	dbbusDWIICIICUseBus();
	pr_ch("dbbusDWIICIICUseBus\n");
	dbbusDWIICIICReshape();
	pr_ch("dbbusDWIICIICReshape\n");
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x08;
	dbbus_tx_data[2] = 0x0c;
	dbbus_tx_data[3] = 0x08;
	pr_tp(" Enable slave's ISP ECO mode \n");
	// Disable the Watchdog
	pr_ch("Disable the Watchdog\n");
	i2c_write_msg2133(dbbus_tx_data, 4);
	//Get_Chip_Version();
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x11;
	dbbus_tx_data[2] = 0xE2;
	dbbus_tx_data[3] = 0x00;
	pr_ch("Get_Chip_Version\n");
	i2c_write_msg2133(dbbus_tx_data, 4);
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x3C;
	dbbus_tx_data[2] = 0x60;
	dbbus_tx_data[3] = 0x55;
	i2c_write_msg2133(dbbus_tx_data, 4);
	pr_ch("update\n");
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x3C;
	dbbus_tx_data[2] = 0x61;
	dbbus_tx_data[3] = 0xAA;
	i2c_write_msg2133(dbbus_tx_data, 4);
	pr_ch("Stop MCU\n");
	//Stop MCU
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x0F;
	dbbus_tx_data[2] = 0xE6;
	dbbus_tx_data[3] = 0x01;
	i2c_write_msg2133(dbbus_tx_data, 4);
	pr_ch("Enable SPI Pad\n");
	//Enable SPI Pad
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x02;
	i2c_write_msg2133(dbbus_tx_data, 3);
	i2c_read_msg2133(&dbbus_rx_data[0], 2);
	pr_tp("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
	dbbus_tx_data[3] = (dbbus_rx_data[0] | 0x20);  //Set Bit 5
	i2c_write_msg2133(dbbus_tx_data, 4);
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x25;
	i2c_write_msg2133(dbbus_tx_data, 3);
	dbbus_rx_data[0] = 0;
	dbbus_rx_data[1] = 0;
	i2c_read_msg2133(&dbbus_rx_data[0], 2);
	pr_tp("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
	dbbus_tx_data[3] = dbbus_rx_data[0] & 0xFC;  //Clear Bit 1,0
	i2c_write_msg2133(dbbus_tx_data, 4);
	/*
	//------------
	// ISP Speed Change to 400K
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x11;
	dbbus_tx_data[2] = 0xE2;
	i2c_write_msg2133( dbbus_tx_data, 3);
	i2c_read_msg2133( &dbbus_rx_data[3], 1);
	//pr_tp("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
	dbbus_tx_data[3] = dbbus_tx_data[3]&0xf7;  //Clear Bit3
	i2c_write_msg2133( dbbus_tx_data, 4);
	*/
	//WP overwrite
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x0E;
	dbbus_tx_data[3] = 0x02;
	i2c_write_msg2133(dbbus_tx_data, 4);
	//set pin high
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x10;
	dbbus_tx_data[3] = 0x08;
	i2c_write_msg2133(dbbus_tx_data, 4);
	dbbusDWIICIICNotUseBus();
	dbbusDWIICNotStopMCU();
	dbbusDWIICExitSerialDebugMode();
	///////////////////////////////////////
	// Start to load firmware
	///////////////////////////////////////
	pr_tp("====================================================  Start to load firmware ===============================================\n");
	drvISP_EntryIspMode();
    pr_tp("======================================== entryisp======================================== \n");
	drvISP_BlockErase(0x00000);
	pr_tp("======================================== drvISP_BlockErase======================================== \n");
	msleep(500);
	pr_tp("FwVersion=2");
	updating = 1;
	for(i = 0; i < 59; i++)    // total  59 KB : 1 byte per R/W
	{

		pr_tp(" drvISP_Program%d\n" ,i+1);
		if (download_firmware_buf == NULL)
		{
		
			pr_tp("download_firmware_buf NULL==== \n");
			return 0;
		}
			
		drvISP_Program(i,&download_firmware_buf[i*1024]);
		msleep(10);
		//pr_tp("drvISP_Program%d %s\n",i,&download_firmware_buf[i*1024]);
		//pr_ch("drvISP_Verify\n");
		//drvISP_Verify ( i, download_firmware_buf[i*1024] ); //verify data
	}

	pr_tp(" update OK \n");
	drvISP_ExitIspMode();
	pr_tp("update OK2 \n");

	updating = 0;
	update_switch = 0;
		
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	msleep(100);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT); 
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);  
	msleep(500);
	
	msg2133_read_fw_ver_custom();
	
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	return size;
}

static DEVICE_ATTR(update, 0777, firmware_update_show, firmware_update_store);

/*test=================*/
static ssize_t firmware_clear_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	U16 k = 0, i = 0, j = 0;
	U8 bWriteData[5] =
	{
		0x10, 0x03, 0, 0, 0
	};
	U8 RX_data[256];
	U8 bWriteData1 = 0x12;
	U32 addr = 0;
	pr_ch("\n");

	for(k = 0; k < 59; i++)    // total  59 KB : 1 byte per R/W
	{
		addr = k * 1024;

		for(j = 0; j < 8; j++)    //128*8 cycle
		{
			bWriteData[2] = (U8)((addr + j * 128) >> 16);
			bWriteData[3] = (U8)((addr + j * 128) >> 8);
			bWriteData[4] = (U8)(addr + j * 128);

		/*	while((drvISP_ReadStatus() & 0x01) == 0x01)
			{
				;    //wait until not in write operation
			}*/
		while((drvISP_ReadStatus() & 0x01 ) == 0x01)
		{
			++i;//wait until not in write operation
		}
		i2c_write_update_msg2133(bWriteData, 5);     //write read flash addr
			drvISP_Read(128, RX_data);
	
			i2c_write_update_msg2133(&bWriteData1, 1);    //cmd end

			for(i = 0; i < 128; i++)    //log out if verify error
			{
				if(RX_data[i] != 0xFF)
				{
				  pr_tp("k=%d,j=%d,i=%d===============erase not clean================", k, j, i);
				}
			}
		}
	}

 pr_tp("read finish\n");
	return sprintf(buf, "%s\n", fw_version);
}

static ssize_t firmware_clear_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
	U8 dbbus_tx_data[4];
	unsigned char dbbus_rx_data[2] = {0};
	//msctpc_LoopDelay ( 100 );        // delay about 100ms*****
	// Enable slave's ISP ECO mode
	/*
	dbbusDWIICEnterSerialDebugMode();
	dbbusDWIICStopMCU();
	dbbusDWIICIICUseBus();
	dbbusDWIICIICReshape();*/

	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x08;
	dbbus_tx_data[2] = 0x0c;
	dbbus_tx_data[3] = 0x08;
	// Disable the Watchdog
	i2c_write_msg2133(dbbus_tx_data, 4);
	//Get_Chip_Version();
	//FwVersion  = 2;
	//if (FwVersion  == 2)
	{
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x11;
		dbbus_tx_data[2] = 0xE2;
		dbbus_tx_data[3] = 0x00;
		i2c_write_msg2133(dbbus_tx_data, 4);
	}
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x3C;
	dbbus_tx_data[2] = 0x60;
	dbbus_tx_data[3] = 0x55;
	i2c_write_msg2133(dbbus_tx_data, 4);
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x3C;
	dbbus_tx_data[2] = 0x61;
	dbbus_tx_data[3] = 0xAA;
	i2c_write_msg2133(dbbus_tx_data, 4);
	//Stop MCU
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x0F;
	dbbus_tx_data[2] = 0xE6;
	dbbus_tx_data[3] = 0x01;
	i2c_write_msg2133(dbbus_tx_data, 4);
	//Enable SPI Pad
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x02;
	i2c_write_msg2133(dbbus_tx_data, 3);
	i2c_read_msg2133(&dbbus_rx_data[0], 2);
	pr_tp("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
	dbbus_tx_data[3] = (dbbus_rx_data[0] | 0x20);  //Set Bit 5
	i2c_write_msg2133(dbbus_tx_data, 4);
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x25;
	i2c_write_msg2133(dbbus_tx_data, 3);
	dbbus_rx_data[0] = 0;
	dbbus_rx_data[1] = 0;
	i2c_read_msg2133(&dbbus_rx_data[0], 2);
	pr_tp("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
	dbbus_tx_data[3] = dbbus_rx_data[0] & 0xFC;  //Clear Bit 1,0
	i2c_write_msg2133(dbbus_tx_data, 4);
	//WP overwrite
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x0E;
	dbbus_tx_data[3] = 0x02;
	i2c_write_msg2133(dbbus_tx_data, 4);
	//set pin high
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x10;
	dbbus_tx_data[3] = 0x08;
	i2c_write_msg2133(dbbus_tx_data, 4);
	dbbusDWIICIICNotUseBus();
	dbbusDWIICNotStopMCU();
	dbbusDWIICExitSerialDebugMode();
	///////////////////////////////////////
	// Start to load firmware
	///////////////////////////////////////
	drvISP_EntryIspMode();
	pr_tp("chip erase+\n");
	drvISP_BlockErase(0x00000);
	pr_tp("chip erase-\n");
	drvISP_ExitIspMode();
	return size;
}

static DEVICE_ATTR(clear, 0777, firmware_clear_show, firmware_clear_store);

/*test=================*/
/*Add by Tracy.Lin for update touch panel firmware and get fw version*/

//add liyaohua start
static ssize_t firmware_tpdinfo_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	pr_tp("*** firmware_version_show fw_version \n");
	return  sprintf(buf, "%s\n", "Mstar2133");
}

//add liyaohua end
static ssize_t firmware_version_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	pr_tp("====================firmware_version_show fw_version ==================================***\n");
	
	return sprintf(buf, "%d%d\n",msg2133_fm_major,msg2133_fm_minor);
}


static DEVICE_ATTR(version, 0777, firmware_version_show, NULL);


#endif

#ifdef TP_PROXIMITY_SENSOR
static ssize_t show_proximity_sensor(struct device *dev, struct device_attribute *attr, char *buf)
{
	static char temp=2;
	buf = ps_data_state;
	if(temp!=*buf)
	{
		pr_tp("proximity_sensor_show: buf=%d\n\n", *buf);
		temp=*buf;
	}    return 1;
}

static ssize_t store_proximity_sensor(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	U8 ps_store_data[4];

	if(buf != NULL && size != 0)
	{
		if(DISABLE_CTP_PS == *buf)
		{
			pr_tp("DISABLE_CTP_PS buf=%d,size=%d\n", *buf, size);
			ps_store_data[0] = 0x52;
			ps_store_data[1] = 0x00;
			ps_store_data[2] = 0x62;
			ps_store_data[3] = 0xa1;
			i2c_write(TOUCH_ADDR_MSG21XX, &ps_store_data[0], 4);
			/*msleep(2000);
			pr_tp("RESET_CTP_PS buf=%d\n", *buf);
			mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
#if defined(MT6575) || defined(MT6577)
			hwPowerDown(MT65XX_POWER_LDO_VGP, "touch"); 
#else
			mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
			mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_IN);
			mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif
			msleep(100);
#if defined(MT6575) || defined(MT6577)
			hwPowerOn(MT65XX_POWER_LDO_VGP, VOL_2800, "touch"); 
#else
			mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
			mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
			mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif 
			mdelay(500);
			mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);*/
		}
		else if(ENABLE_CTP_PS == *buf)
		{
			pr_tp("ENABLE_CTP_PS buf=%d,size=%d\n", *buf, size);
			ps_store_data[0] = 0x52;
			ps_store_data[1] = 0x00;
			ps_store_data[2] = 0x62;
			ps_store_data[3] = 0xa0;
			i2c_write(TOUCH_ADDR_MSG21XX, &ps_store_data[0], 4);

		}
	}

	return size;
}
static DEVICE_ATTR(proximity_sensor, 0777, show_proximity_sensor, store_proximity_sensor);
//add by liyaohua

int cy8ctst_sersor_snedC0(void)
{
	U8 ps_store_data[4];

	int ret = 0;
	ps_store_data[0] = 0x52;
	ps_store_data[1] = 0x00;
	ps_store_data[2] = 0x62;
	ps_store_data[3] = 0xa0;
	
    ret =	i2c_write(TOUCH_ADDR_MSG21XX, &ps_store_data[0], 4);
	pr_tp("tpd_touchinfo cy8ctst_sersor_snedC0_ ret= %d\n", ret);
	
}


int cy8ctst_open_sersor(int on)
{

	
	U8 ps_store_data[4];
	int ret = 0, value = 0;
	pr_tp("tpd_touchinfo---Enable cy8ctst_open_sersor on:%d \n",on);
	if(on)
	{
			//add by liyaohua start
			  wake_lock(&Tp_wake_lock);
			//add by liyaohua end
			sensor_open = 1;
			pr_tp("@@@@@@@@tpd_touchinfo---Enable cy8ctst_open_sersor\n");			
			tfopensenor = true;
			ps_store_data[0] = 0x52;
			pr_tp("tpd_touchinfo---0x52\n");
			ps_store_data[1] = 0x00;
			pr_tp("tpd_touchinfo---0x00\n");
			ps_store_data[2] = 0x62;
			pr_tp("tpd_touchinfo---0x62\n");
			ps_store_data[3] = 0xa0;
			pr_tp("tpd_touchinfo---0xa0\n");
			
		    ret =	i2c_write(TOUCH_ADDR_MSG21XX, &ps_store_data[0], 4);
			pr_tp("tpd_touchinfo cy8ctst_open_sersor ret= %d\n", ret);
	}
	else
	{
	        //add by liyaohua start
			wake_unlock(&Tp_wake_lock);
			//add by liyaohua end
			pr_tp("@@@@@@@tpd_touchinfo---DISABLE cy8ctst_open_sersor\n");
			tfopensenor =false;
			ps_store_data[0] = 0x52;
			pr_tp("tpd_touchinfo---0x52\n");
			ps_store_data[1] = 0x00;
			pr_tp("tpd_touchinfo---0x00\n");
			ps_store_data[2] = 0x62;
			pr_tp("tpd_touchinfo---0x62\n");
			ps_store_data[3] = 0xa1;
			pr_tp("tpd_touchinfo---0xa1\n");
			
			ret =	i2c_write(TOUCH_ADDR_MSG21XX, &ps_store_data[0], 4);
					
				
			sensor_open = 0;
			
	}	
	return ret;
}
EXPORT_SYMBOL(cy8ctst_open_sersor);
int cy8ctst_get_sensor_state(void)
{
	pr_tp("@@@@@tpd_touchinfo---cy8ctst_get_sensor_stateps_data_state[0] %u ",ps_data_state[0]);
	return ps_data_state[0];
}
EXPORT_SYMBOL( cy8ctst_get_sensor_state);



#endif

static struct i2c_driver tpd_i2c_driver =
{
	.driver = {
        .name = "msg2133",
		//.owner = THIS_MODULE,
	},
	.probe = tpd_probe,
	.remove = __devexit_p(tpd_remove),
	.id_table = msg2133_tpd_id,
	.detect = tpd_detect,
	//Tinno:CJ
	//new kernel not support this , you need use  i2c_board_info instand of it
	//.address_data = &addr_data,
};


static  void tpd_down(int x, int y)
{

#ifdef TPD_HAVE_BUTTON
	  if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())  
		{
				tpd_button(x, y, 1);
				pr_tp("===================tpd_down: x=%d, y=%d\n\n", x, y);
			
		}
#endif

	pr_tp("tpd_down: x=%d, y=%d\n\n", x, y);

	input_report_key(tpd->dev, BTN_TOUCH, 1);
	input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(tpd->dev);
	TPD_DOWN_DEBUG_TRACK(x, y);
}

static  int tpd_up(int x, int y)
{
     	

	  if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
     {   
     	pr_tp("===================tpd_up button: x=%d, y=%d\n\n", x, y);
        tpd_button(x, y, 0); 
     }   		 
//add by liyaohua 
	 input_report_abs(tpd->dev, ABS_PRESSURE, 0);
	 input_report_key(tpd->dev, BTN_TOUCH, 0);
	// input_report_abs(tpd->dev,ABS_MT_TRACKING_ID,i);
	 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	// mt6575_touch_debug("%s: x=%d, y=%d, p=%d--Liu\n", 	 __func__, x, y, 0);

	input_mt_sync(tpd->dev);
	//input_sync(tpd->dev);
	TPD_UP_DEBUG_TRACK(x, y);
	
	return 1;
}


#ifdef TP_PROXIMITY_SENSOR

extern void cy8ctst_alsps_do_work(void);
#endif


//static int tpd_touchinfo(struct touch_info *cinfo)
static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)

{
	pr_tp("tpd_touchinfo---start!\n");

	SHORT_TOUCH_STATE ShortTouchState;
	BYTE reg_val[8] = {0};
	unsigned int  temp = 0;

	if(update_switch)
	{
		return false;
	}
	
	
//	memset(cinfo, 0, sizeof(struct touch_info));
	
	pr_tp("tpd_touchinfo---msg2133_i2c_readstart!\n");
	msg2133_i2c_read(reg_val, 8);
	pr_tp("tpd_touchinfo---msg2133_i2c_readend!\n");

	ShortTouchState.pos_x = ((reg_val[1] & 0xF0) << 4) | reg_val[2];
	ShortTouchState.pos_y = ((reg_val[1] & 0x0F) << 8) | reg_val[3];
	ShortTouchState.dst_x = ((reg_val[4] & 0xF0) << 4) | reg_val[5];
	ShortTouchState.dst_y = ((reg_val[4] & 0x0F) << 8) | reg_val[6];

	if((ShortTouchState.dst_x) & 0x0800)
	{
		ShortTouchState.dst_x |= 0xF000;
	}

	if((ShortTouchState.dst_y) & 0x0800)
	{
		ShortTouchState.dst_y |= 0xF000;
	}

	ShortTouchState.pos_x2 = ShortTouchState.pos_x + ShortTouchState.dst_x;
	ShortTouchState.pos_y2 = ShortTouchState.pos_y + ShortTouchState.dst_y;
	temp = tpd_check_sum(reg_val);

	pr_tp("tpd_touchinfo---msg2133_i2c_readendtemp %u  reg_val[7] %u !\n",temp,reg_val[7]);
	if(temp == reg_val[7])
	{
	  pr_tp("tpd_touchinfo---msg2133_i2c_readend reg_val[0] %u !\n",reg_val[0]);
		if(reg_val[0] == 0x52) //CTP  ID
		{
		 pr_tp("tpd_touchinfo---msg2133_i2c_readend reg_val[1] %u reg_val[4] %u !\n",reg_val[1],reg_val[4] );
			if(reg_val[1] == 0xFF&& reg_val[4] == 0xFF)
				
			{
			pr_tp("tpd_touchinfo---msg2133_i2c_readend reg_val[5] %u !\n",reg_val[5] );
		#ifdef TP_PROXIMITY_SENSOR
				if(reg_val[5] == 0x80) // close to
				{
					 pr_tp("tpd_touchinfo---cy8ctst_get_sensor_state read ps_data_state[0] 1!\n");
					ps_data_state[0] = 1;
					if(tfopensenor)
					{
					      pr_tp("@@@@@@@@@@@@@@@@tpd_touchinfo-- cy8ctst_alsps_do_work  0x80 start!\n");
						  cy8ctst_alsps_do_work();
						  pr_tp("@@@@@@@@@@@@@@@@tpd_touchinfo-- cy8ctst_alsps_do_work 0x80 end !\n");
					}

		        }
				else  if(reg_val[5] == 0x40) // leave
				{
					 pr_tp("tpd_touchinfo---cy8ctst_get_sensor_state read ps_data_state[0] 0!\n");
					ps_data_state[0] = 0;
					if(tfopensenor)
					{
					      pr_tp("@@@@@@@@@@@@@@@tpd_touchinfo-- cy8ctst_alsps_do_work start 0x40!\n");
						  cy8ctst_alsps_do_work();
						  pr_tp("@@@@@@@@@@@@@@@tpd_touchinfo-- cy8ctst_alsps_do_work 0x40 end !\n");
					}
				}
				else if(reg_val[5] == 0xC0) 
				{
					 //pr_tp("tpd_touchinfo---cy8ctst_get_sensor_state read ps_data_state[0] 0!\n");
					 	if( sensor_open || updating )
						{
							cy8ctst_sersor_snedC0();
						}
					
					 
				}
				else
		#endif
#ifdef TPD_HAVE_BUTTON
				if(reg_val[5] == 0x01)
				{
				     cinfo->x[0] = tpd_keys_dim_local[0][0];
				     cinfo->y[0] = tpd_keys_dim_local[0][1];
				     point_num = 1;
				}
				#if (TPD_KEY_COUNT>=2)
				else if(reg_val[5] == 0x02)
				{
				     cinfo->x[0] = tpd_keys_dim_local[1][0];
				     cinfo->y[0] = tpd_keys_dim_local[1][1];
				     point_num = 1;
				}
				#endif
				#if (TPD_KEY_COUNT>=3)
				else if(reg_val[5] == 0x04)
				{
				     cinfo->x[0] = tpd_keys_dim_local[2][0];
				     cinfo->y[0] = tpd_keys_dim_local[2][1];
				     point_num = 1;
				}
				#endif
				#if (TPD_KEY_COUNT>=4)
				else if(reg_val[5] == 0x08)
				{
				     cinfo->x[0] = tpd_keys_dim_local[3][0];
				     cinfo->y[0] = tpd_keys_dim_local[3][1];
				     point_num = 1;
				}
				#endif
				else
#endif
				{
					if(point_num ==2)
					{
						touch_up_num =2;
					}
					point_num = 0;
				}
			}
			else if(ShortTouchState.pos_x > 2047 || ShortTouchState.pos_y > 2047)
			{
				return  false;
			}
			else if((ShortTouchState.dst_x == 0) && (ShortTouchState.dst_y == 0))
			{
				//cinfo->x[0] = (ShortTouchState.pos_x * TPD_RES_X) / 2048;
				//cinfo->y[0] = (ShortTouchState.pos_y *TPD_RES_Y) / 2048;

				tempx= (ShortTouchState.pos_x * TPD_RES_X) / 2048;
				tempy = (ShortTouchState.pos_y *TPD_RES_Y) / 2048;

				
				if(point_num == 2)
				{
					if((ABS(cinfo->x[0]-tempx)<5)&&(ABS(cinfo->y[0]-tempy)<5))
					{
						 tpd_up(cinfo->x[1], cinfo->y[1]);
						
					}else
					{
						 tpd_up(cinfo->x[0], cinfo->y[0]);
					}
				}

				cinfo->x[0]= tempx;
				cinfo->y[0]= tempy;
				point_num = 1;
			}
			else
			{
				if(ShortTouchState.pos_x2 > 2047 || ShortTouchState.pos_y2 > 2047)
					return false;
				cinfo->x[0] = (ShortTouchState.pos_x * TPD_RES_X) / 2048;
				cinfo->y[0] = (ShortTouchState.pos_y *  TPD_RES_Y) / 2048;
				cinfo->x[1] = (ShortTouchState.pos_x2 * TPD_RES_X) / 2048;
				cinfo->y[1] = (ShortTouchState.pos_y2 * TPD_RES_Y) / 2048;
				point_num = 2;
			}
		}
		pr_tp("tpd_touchinfo---end true!\n");

		return true;
	}
	else
	{
	pr_tp("tpd_touchinfo---end false!\n");
		return  false;
	}

}



static int touch_event_handler(void *unused)
{
	struct touch_info cinfo,pinfo;;
   
    static unsigned short Pos_x = 0,Pos_y = 0;
	unsigned long time_eclapse;
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	sched_setscheduler(current, SCHED_RR, &param);
	do
	{
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		set_current_state(TASK_INTERRUPTIBLE);
		wait_event_interruptible(waiter, tpd_flag != 0);
		tpd_flag = 0;
		set_current_state(TASK_RUNNING);

		if(tpd_touchinfo(&cinfo, &pinfo))
		{
			if(point_num == 1)
			{
				
				tpd_down(cinfo.x[0], cinfo.y[0]);	
				
				input_sync(tpd->dev);
			}
			else if(point_num == 2)
			{
	            tpd_down(cinfo.x[0], cinfo.y[0]);
	            tpd_down(cinfo.x[1], cinfo.y[1]);
				input_sync(tpd->dev);
			}
			else if(point_num == 0)
			{

			    tpd_up(cinfo.x[0], cinfo.y[0]);
				if(touch_up_num ==2)
				{
					tpd_up(cinfo.x[1], cinfo.y[1]);
					touch_up_num =0;
				}
		
				input_sync(tpd->dev);
			}
		}
		
	}
	while(!kthread_should_stop());

	return 0;
}

int mstar_get_fw()
{
	uint8_t fw = -1;
	int ret = 0;
	return (int)fw;
}


static int tpd_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{
	strcpy(info->type, TPD_DEVICE);
	return 0;
}

static void tpd_eint_interrupt_handler(void)
{
	pr_tp("Magnum msg2133 tpd_eint_interrupt.....\n ");
	tpd_flag = 1;
	wake_up_interruptible(&waiter);
}
#ifdef POWERUP_AUTO_CHECK
void msg2133_ctpm_get_upg_ver_custom(void)
{
	msg2133_bin_major=(CTPM_FW[0x3076]<<8)|CTPM_FW[0x3077];
	msg2133_bin_minor=(CTPM_FW[0x3074]<<8)|CTPM_FW[0x3075];

	pr_tp("***msg2133_bin_majormajor = %d ***\n", msg2133_bin_major);
	pr_tp("***msg2133_bin_majorminor = %d ***\n", msg2133_bin_minor);
}
//extern int cy8ctst_get_vendor_fw (void);
int msg2133_read_fw_ver_custom(void)

{
  int rc = 0;
  int icyfw=0;
	unsigned char dbbus_tx_data[3];
	unsigned char dbbus_rx_data[4] ;
	unsigned short major = 0, minor = 0;
	msg2133_fm_major = 100;
	pr_tp("==================================================== msg2133_read_fw_ver_custom customupdate start ==========================\n");

	
	pr_ch("\n");
	dbbus_tx_data[0] = 0x53;
	dbbus_tx_data[1] = 0x00;
	dbbus_tx_data[2] = 0x74;
	rc = i2c_write(TOUCH_ADDR_MSG21XX, &dbbus_tx_data[0], 3);
	if(rc<0)
		return -1;
	rc = i2c_read(TOUCH_ADDR_MSG21XX, &dbbus_rx_data[0], 4);
	msg2133_fm_major = (dbbus_rx_data[1] << 8) + dbbus_rx_data[0];
	msg2133_fm_minor = (dbbus_rx_data[3] << 8) + dbbus_rx_data[2];
	pr_tp("***msg2133_fm_majormajor = %d ***\n", msg2133_fm_major);
	pr_tp("***msg2133_fm_majorminor = %d ***\n", msg2133_fm_minor);
	return msg2133_fm_major;
}
#endif

static int __devinit 
tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    pr_tp("tpd_probe\n");
	
	int retval = TPD_OK;
	client->timing = 100;

	i2c_client = client;
	
	
	pr_tp("msta_powermsta_power start rc %d=\n",icy);
	
		
	msta_power(1);

	msg2133_read_fw_ver_custom();
	
	tpd_load_status = 1;
	mthread = kthread_run(touch_event_handler, 0, TPD_DEVICE);

	if(IS_ERR(mthread))
	{
		retval = PTR_ERR(mthread);
		pr_tp(TPD_DEVICE " failed to create kernel thread: %d\n", retval);
	}
#ifdef TP_FIRMWARE_UPDATE
#if ENABLE_DMA
	gpDMABuf_va = (u8 *)dma_alloc_coherent(NULL, 4096, &gpDMABuf_pa, GFP_KERNEL);

	if(!gpDMABuf_va)
	{
		pr_tp("[MATV][Error] Allocate DMA I2C Buffer failed!\n");
	}

#endif
    firmware_class = class_create(THIS_MODULE, "mtk-tpd");
	pr_tp("================ms-touchscreen-msg20xx!=============START=============\n");
	if(IS_ERR(firmware_class))
	{
		pr_err("Failed to create class(firmware)!\n");
	}

	firmware_cmd_dev = device_create(firmware_class,NULL, 0, NULL, "device");

	if(IS_ERR(firmware_cmd_dev))
	{
		pr_err("Failed to create device(firmware_cmd_dev)!\n");
	}

	// version
	if(device_create_file(firmware_cmd_dev, &dev_attr_version) < 0)
	{
		pr_err("Failed to create device file(%s)!\n", dev_attr_version.attr.name);
	}

	// update
	if(device_create_file(firmware_cmd_dev, &dev_attr_update) < 0)
	{
		pr_err("Failed to create device file(%s)!\n", dev_attr_update.attr.name);
	}
	
	pr_tp("================ms-touchscreen-msg20xx!=============END=============\n");
	// tpd_info
	if(device_create_file(firmware_cmd_dev, &dev_attr_tpd_info) < 0)
	{
		pr_err("Failed to create device file(%s)!\n", dev_attr_tpd_info.attr.name);
	}

	pr_tp("================ms-touchscreen-msg20xx!=============END=============\n");

#endif
#ifdef TP_PROXIMITY_SENSOR

	if(device_create_file(firmware_cmd_dev, &dev_attr_proximity_sensor) < 0) // /sys/class/mtk-tpd/device/proximity_sensor
	{
		pr_err("Failed to create device file(%s)!\n", dev_attr_proximity_sensor.attr.name);
	}
#endif

	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	//msleep(100);

	pr_tp("Touch Panel Device Probe %s\n", (retval < TPD_OK) ? "FAIL" : "PASS");
	return 0;
}

static int __devexit tpd_remove(struct i2c_client *client)

{
	pr_tp("TPD removed\n");
#ifdef TP_FIRMWARE_UPDATE
#if ENABLE_DMA

	if(gpDMABuf_va)
	{
		dma_free_coherent(NULL, 4096, gpDMABuf_va, gpDMABuf_pa);
		gpDMABuf_va = NULL;
		gpDMABuf_pa = NULL;
	}

#endif
#endif
	return 0;
}


static int tpd_local_init(void)
{
	pr_tp(" MSG2133 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);

	if(i2c_add_driver(&tpd_i2c_driver) != 0)
	{
		pr_tp("unable to add i2c driver.\n");
		return -1;
	}

#ifdef TPD_HAVE_BUTTON
	tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
	boot_mode = get_boot_mode();
#endif
	//add by liyaohua
	 wake_lock_init(&Tp_wake_lock,WAKE_LOCK_SUSPEND,"TP_WakeLock");
	//add by liyaohua
	pr_tp("end %s, %d\n", __FUNCTION__, __LINE__);
	tpd_type_cap = 1;
	return 0;
}

static int tpd_resume(struct i2c_client *client)
{
	printk("==============TPD enter sleep tpd_resume  start===============\n");

	int retval = TPD_OK;
	pr_tp("TPD wake up\n");
	if(  sensor_open || updating ) {
		//mt6575_touch_info("%s: process_cmd=%d, updating=%d, sensor_open=%d--Liu\n", 
		//	__func__, sensor_open);
		printk("==============sensor_open:%d || updating:%d===============\n",sensor_open , updating);
		return 1;
	}

	msta_power(1);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	return retval;
}

static int tpd_suspend(struct i2c_client *client, pm_message_t message)
{
	printk("==============TPD enter sleep tpd_suspend  start===============\n");
	int retval = TPD_OK;
	static char data = 0x3;
	pr_tp("TPD enter sleep\n");
	if( sensor_open || updating ) {
		//mt6575_touch_info("%s: process_cmd=%d, updating=%d, sensor_open=%d--Liu\n", __func__,  sensor_open);
		printk("==============sensor_open:%d || updating:%d===============\n",sensor_open , updating);
		return 1;
	}

#ifdef TP_FIRMWARE_UPDATE
	if(update_switch==0)
#endif
	{
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
	msta_power(0);
	}
	return retval;
}

int tpd_power_switch( int flag )
{ 
	return 0;
}

EXPORT_SYMBOL( tpd_power_switch );

static struct tpd_driver_t tpd_device_driver =
{
	.tpd_device_name = "MSG2133",
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
	.tpd_have_button = 1,
#else
	.tpd_have_button = 0,
#endif
};
/* called when loaded into kernel */
static int __init tpd_driver_init(void)
{
	pr_tp("MediaTek MSG2133 touch panel driver init\n");
	i2c_register_board_info(0, &msg2133_i2c_tpd, 1);
	if(tpd_driver_add(&tpd_device_driver) < 0)
	{
		iIsMsg2133 = 0;
		pr_tp("add MSG2133 driver failed\n");
	}

	return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void)
{
	pr_tp("MediaTek MSG2133 touch panel driver exit\n");
	//input_unregister_device(tpd->dev);
	tpd_driver_remove(&tpd_device_driver);
}

ssize_t mem_write(struct file *filp, const char *buf, size_t size, loff_t *lofp)
{
	int res = -1;
	char *tmp;
	int imemcy =0 ;
	pr_tp("<0>read data from the user space\n");
	tmp = mem_spvm;
	if (size > MEM_MALLOC_SIZE)    
		size = MEM_MALLOC_SIZE;
	
	if (tmp != NULL)
		res = copy_from_user(tmp, buf, size);
	if (res == 0)
	{
	    pr_tp("=======<0>read data success and the data is22222222222:%s=================\n",buf);
		
		
		//add by liyaohua 
		if(0==strcmp(buf,"I"))
		{
				msg2133_ctpm_get_upg_ver_custom();
				//msg2133_read_fw_ver_custom();
				pr_tp("================================IMsg2133Version start ====================================\n");
				if((msg2133_fm_major!=msg2133_bin_major)||(msg2133_fm_minor!=msg2133_bin_minor))
				{
					pr_tp("IMsg2133Version 0 \n");
					strcpy(mem_spvm,"0");
				}
				else

				{
					pr_tp("IMsg2133Version 1\n");
					strcpy(mem_spvm,"1");
				}
				
		}
		
				
		if(0==strcmp(buf,"O"))
		{
			int itpup =0;
			mutex_lock(&readtp_mutex);
			itpup =  g_update;
			mutex_unlock(&readtp_mutex);
			pr_tp("================================OK====================================\n");
			return sprintf(mem_spvm,"%d",itpup);

		}
		
	    if(0==strcmp(buf,"X"))
		{ //imemcy =0;
			mutex_lock(&tp_mutex);
			pr_tp("=XXXXXXXXXXXXXXX %s==\n",buf);
	
#ifdef TP_FIRMWARE_UPDATE
			
#ifdef POWERUP_AUTO_CHECK			
				
			pr_tp("=tpd_resume=msg2133_read_fw_ver_custom%d\n",msg2133_fm_major);
		
			
	        updating =1;
		 
 			pr_tp("====================================================ifwupdate===============================================\n");
			
			download_firmware_buf=CTPM_FW;
		
			firmware_update_store(NULL,NULL,NULL,0);
			
			pr_tp("tpd_resume2\n");
		
			//mutex_unlock(&tp_mutex);
			 updating =0;
			mutex_lock(&readtp_mutex);
			 g_update = 1;
			mutex_unlock(&readtp_mutex);
#endif
#endif
//add by liyaohua end 
			strcpy(mem_spvm,"EEEEEEEEEEEEE");
			mutex_unlock(&tp_mutex);

		}
 		
		return size;
	}
	else
	{
		pr_tp("<0>read data from user space fail\n");
		return 0;
	}
}
module_init(tpd_driver_init);
module_exit(tpd_driver_exit);
module_init(register_unregister_chrdev_init);   
module_exit(register_unregister_chrdev_exit);   
