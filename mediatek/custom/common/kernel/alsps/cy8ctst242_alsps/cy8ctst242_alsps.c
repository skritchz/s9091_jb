#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include <linux/hwmsen_helper.h>

#ifdef MT6516
#include <mach/mt6516_devs.h>
#include <mach/mt6516_typedefs.h>
#include <mach/mt6516_gpio.h>
#include <mach/mt6516_pll.h>
#endif

#ifdef MT6573
#include <mach/mt6573_devs.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_gpio.h>
#include <mach/mt6573_pll.h>
#endif

#ifdef MT6575
#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#endif

static int alsps_debug_mask = 1;
#define alsps_debug(format,...) \
	do { \
		if (alsps_debug_mask) \
			printk(format,__VA_ARGS__); \
	} while (0)


struct cy8ctst_alsps_ctxt
{
	int opened;
	uint32_t enable;
	uint8_t dat;
	uint8_t ps;
	uint32_t pending_intr;   /*pending interrupt*/
	atomic_t ps_mask;        /*mask ps: always return far away*/
	atomic_t ps_debounce;    /*debounce time after enabling ps*/
	atomic_t ps_deb_on;      /*indicates if the debounce is on*/
	atomic_t ps_deb_end;     /*the jiffies representing the end of debounce*/
	atomic_t ps_suspend;
	struct mutex lock;
	struct delayed_work alsps_work;
};
static struct cy8ctst_alsps_ctxt *_cy8ctst_alsps;

extern int cy8ctst_open_sersor(int on);
extern int cy8ctst_get_sensor_state(void);

static void cy8ctst_alsps_work(struct work_struct *work)
{
	int rc = 0;
	struct cy8ctst_alsps_ctxt *cy8ctst_alsps = 
		container_of(work, struct cy8ctst_alsps_ctxt, alsps_work.work);

	hwm_sensor_data sensor_data;
	memset(&sensor_data, 0, sizeof(sensor_data));
	if(!cy8ctst_alsps) {
		alsps_debug("%s: cy8ctst_alsps is null--Liu\n", __func__);
		return;
	}
	alsps_debug("%s: pending_intr=0x%x--Liu\n", 
		__func__, cy8ctst_alsps->pending_intr);
	cy8ctst_alsps->ps = !cy8ctst_get_sensor_state();
	sensor_data.values[0] = cy8ctst_alsps->ps;
	sensor_data.value_divide = 1;
	sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
	rc = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data);
	if(rc) {
		alsps_debug("%s: hwmsen_get_interrupt_data error, rc=%d--Liu\n", 
			__func__, rc);
	}
}

void cy8ctst_alsps_do_work(void)
{
	struct cy8ctst_alsps_ctxt *cy8ctst_alsps = _cy8ctst_alsps;
	if(!cy8ctst_alsps) {
		alsps_debug("%s:cy8ctst_alsps is NULL--Liu\n", __func__);
		return;
	}
	schedule_delayed_work(&cy8ctst_alsps->alsps_work, 0);
}
EXPORT_SYMBOL(cy8ctst_alsps_do_work);

static int cy8ctst242_alsps_open(struct inode *inode, struct file *file)
{
	int rc = 0;
	struct cy8ctst_alsps_ctxt *cy8ctst_alsps = _cy8ctst_alsps;
	alsps_debug("%s--Liu\n", __func__);
	mutex_lock(&cy8ctst_alsps->lock);
	if(cy8ctst_alsps->opened == 0) {
		file->private_data = cy8ctst_alsps;
		cy8ctst_alsps->opened = 1;
	}
	else {
		alsps_debug("%s:cy8ctst_alsps_dev already opened--Liu\n", 
			__func__);
		rc = -EBUSY;
	}
	mutex_unlock(&cy8ctst_alsps->lock);
	rc = nonseekable_open(inode, file);
	return rc;
}

static int cy8ctst242_alsps_release(struct inode *inode, struct file *file)
{
	struct cy8ctst_alsps_ctxt *cy8ctst_alsps = file->private_data;
	alsps_debug("%s--Liu\n", __func__);
	mutex_lock(&cy8ctst_alsps->lock);
	cy8ctst_alsps->opened = 0;
	mutex_unlock(&cy8ctst_alsps->lock);
	return 0;
}

static long cy8ctst242_alsps_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int rc = 0;
	struct cy8ctst_alsps_ctxt *cy8ctst_alsps = file->private_data; 
	if(!cy8ctst_alsps) {
		alsps_debug("%s: cy8ctst_alsps is NULL--Liu\n", __func__);
		return -1;
	}
	alsps_debug("%s:cmd=%x--Liu\n", __func__, cmd);
	mutex_lock(&cy8ctst_alsps->lock);
	switch(cmd) {
		case ALSPS_SET_PS_MODE:
			rc = copy_from_user(&(cy8ctst_alsps->enable), 
				(void __user *) arg, sizeof(cy8ctst_alsps->enable));
			if (rc) {
				alsps_debug("%s: get led_show error: invalid pointer--Liu\n", 
					__func__);
				rc = -EFAULT;
			}
			cy8ctst_open_sersor(cy8ctst_alsps->enable);
			alsps_debug("%s: ALSPS_SET_PS_MODE, enable=%d--Liu\n", 
				__func__, cy8ctst_alsps->enable);
			break;
		case ALSPS_GET_PS_MODE:
			rc = copy_to_user((void __user *) arg, &(cy8ctst_alsps->enable), 
				sizeof(cy8ctst_alsps->enable));
			if (rc) {
				alsps_debug("%s: GET_PS_MODE error: invalid pointer--Liu\n", 
					__func__);
				rc = -EFAULT;
			}
			alsps_debug("%s: ALSPS_GET_PS_MODE, enable=%d--Liu\n", 
				__func__, cy8ctst_alsps->enable);
			break;
		case ALSPS_GET_PS_DATA:
			cy8ctst_alsps->ps = !cy8ctst_get_sensor_state();
			rc = copy_to_user((void __user *) arg, &(cy8ctst_alsps->ps), 
				sizeof(cy8ctst_alsps->ps));
			if (rc) {
				alsps_debug("%s: GET_PS_DATA error: invalid pointer--Liu\n", 
					__func__);
				rc = -EFAULT;
			}
			alsps_debug("%s: ALSPS_GET_PS_DATA, ps=%d--Liu\n", 
				__func__, cy8ctst_alsps->ps);
			break;
		case ALSPS_GET_PS_RAW_DATA	:
			cy8ctst_alsps->dat = !cy8ctst_get_sensor_state();
			rc = copy_to_user((void __user *) arg, &(cy8ctst_alsps->dat), 
				sizeof(cy8ctst_alsps->dat));
			if (rc) {
				alsps_debug("%s: GET_RAW_DATA error: invalid pointer--Liu\n", __func__);
				rc = -EFAULT;
			}
			alsps_debug("%s: ALSPS_GET_PS_RAW_DATA, dat=%d--Liu\n", 
				__func__, cy8ctst_alsps->dat);
			break;
		default:
			break;
	}
	mutex_unlock(&cy8ctst_alsps->lock);
	return rc;
}

static struct file_operations cy8ctst242_alsps_fops = {
	.owner = THIS_MODULE,
	.open = cy8ctst242_alsps_open,
	.release = cy8ctst242_alsps_release,
	.unlocked_ioctl = cy8ctst242_alsps_ioctl,
};

static struct miscdevice cy8ctst242_alsps_cdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &cy8ctst242_alsps_fops,
};

int cy8ctst242_alsps_ps_operate(void* self, uint32_t command, 
		void* buff_in, int size_in, void* buff_out, int size_out, int* actualout)
{
	int rc = 0, value,ret;
	hwm_sensor_data* sensor_data;
	struct cy8ctst_alsps_ctxt *cy8ctst_alsps = (struct cy8ctst_alsps_ctxt *)self;
	
	alsps_debug("%s--Liu\n", __func__);
	if(!cy8ctst_alsps) {
		alsps_debug("%s: cy8ctst_alsps is NULL--Liu\n", __func__);
		return -1;
	}
	switch (command) {
		case SENSOR_DELAY:
			alsps_debug("%s:SENSOR_DELAY--Liu\n", __func__);
			if((buff_in == NULL) || (size_in < sizeof(int))) {
				alsps_debug("%s:Set delay parameter error!--Liu\n", 
					__func__);
				rc = -EINVAL;
			}
			break;
		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int))) {
				alsps_debug("%s:Enable sensor parameter error!--Liu\n", 
					__func__);
				rc = -EINVAL;
			}
			else {				
				value = *(int *)buff_in;
				alsps_debug("%s:SENSOR_ENABLE, value=%d--Liu\n", __func__, value);
				cy8ctst_alsps->enable = value;
				ret = cy8ctst_open_sersor(cy8ctst_alsps->enable);
				rc = (ret < 0) ? rc : 0; 
				if(rc < 0){
					alsps_debug("%s:open_sersor error, value=%d--Liu\n", 
						__func__, value);
					return rc;
				}
				cy8ctst_alsps_do_work();
			}
			break;
		case SENSOR_GET_DATA:
			alsps_debug("%s:SENSOR_GET_DATA--Liu\n", __func__);
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data))) {
				alsps_debug("%s: SENSOR_GET_DATA parameter error!--Liu\n", __func__);
				rc = -EINVAL;
			}
			else {
				sensor_data = (hwm_sensor_data *)buff_out;
				cy8ctst_alsps->ps = !cy8ctst_get_sensor_state();
				if(cy8ctst_alsps->ps < 0){
					alsps_debug("%s: SENSOR_GET_DATA, error--Liu\n", __func__);
					return -1;
				}
				sensor_data->values[0] = cy8ctst_alsps->ps;
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
				alsps_debug("%s:ps=%d--Liu\n", __func__, cy8ctst_alsps->ps);

			}
			break;
		default:
			alsps_debug("%s: command=%d no found--Liu!\n", 
				__func__, command);
			rc = -1;
			break;
	}
	alsps_debug("%s:cy8ctst242_alsps_ps_operate rc=%d--Liu\n", 
						__func__, rc);
	return rc;
}

static int cy8ctst242_alsps_probe(struct platform_device *pdev) 
{
	int rc = 0;
	struct hwmsen_object obj_ps;
	struct cy8ctst_alsps_ctxt *cy8ctst_alsps;
	alsps_debug("%s--Liu\n", __func__);
	cy8ctst_alsps = kzalloc(sizeof(*cy8ctst_alsps), GFP_KERNEL);
	if (!cy8ctst_alsps) {
		alsps_debug("%s: no memery!--Liu\n", __func__);
		return -ENOMEM;
	}
	mutex_init(&cy8ctst_alsps->lock);
	INIT_DELAYED_WORK(&cy8ctst_alsps->alsps_work, cy8ctst_alsps_work);
	cy8ctst_alsps->opened = 0;
	cy8ctst_alsps->enable = 0;
	atomic_set(&cy8ctst_alsps->ps_debounce, 1000);
	atomic_set(&cy8ctst_alsps->ps_deb_on, 0);
	atomic_set(&cy8ctst_alsps->ps_deb_end, 0);
	atomic_set(&cy8ctst_alsps->ps_mask, 0);
	_cy8ctst_alsps = cy8ctst_alsps;
	rc = misc_register(&cy8ctst242_alsps_cdev);
	if(rc) {
		alsps_debug("%s:alsps_cdev register error, rc=%d--Liu\n", 
			__func__, rc);
		return rc;
	}
	obj_ps.self = cy8ctst_alsps;
	obj_ps.polling = 0;
	obj_ps.sensor_operate = cy8ctst242_alsps_ps_operate;
	rc = hwmsen_attach(ID_PROXIMITY, &obj_ps);
	if(rc) {
		alsps_debug("%s: hwmsen_attach error, rc=%d--Liu\n", 
			__func__, rc);
		return rc;
	}
	return 0;
}

static int cy8ctst242_alsps_remove(struct platform_device *pdev)
{
	alsps_debug("%s--Liu\n", __func__);
	return 0;
}

static struct platform_driver cy8ctst242_alsps_driver = {
	.probe      = cy8ctst242_alsps_probe,
	.remove     = cy8ctst242_alsps_remove,    
	.driver     = {
		.name  = "als_ps",
		.owner = THIS_MODULE,
	}
};

static int __init cy8ctst242_alsps_init(void)
{
	int rc = 0;
	alsps_debug("%s--Liu\n", __func__);
	rc = platform_driver_register(&cy8ctst242_alsps_driver);
	if(rc) {
		alsps_debug("%s: failed to register drive--Liu\n", __func__);
		return -ENODEV;
	}	
	return 0;
}

static void __exit cy8ctst242_alsps_exit(void)
{
	alsps_debug("%s--Liu\n", __func__);
	platform_driver_unregister(&cy8ctst242_alsps_driver);
}

module_init(cy8ctst242_alsps_init);
module_exit(cy8ctst242_alsps_exit);

MODULE_AUTHOR("Liu.Yang");
MODULE_DESCRIPTION("CY8CTST ALSPS DRIVER");
MODULE_LICENSE("GPL");

