/*
 * cy8ctst242.h -- cy8ctst242 touch driver core interface
 *
 * Copyright (C) 2012 Liu Yang
 * All rights reserved.
 *
 */

#ifndef __CY8CTST242__
#define __CY8CTST242__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
//#include <linux/debug_control.h>


#define TPD_OK	0
#define TPD_DISTANCE_LIMIT	100
#define TPD_REG_BASE	0x00
#define TPD_SOFT_RESET_MODE	0x01
#define TPD_OP_MODE 0x00
#define TPD_LOW_PWR_MODE 0x04
#define TPD_SYSINFO_MODE 0x10
#define TPD_TYPE_CAPACITIVE
#define TPD_HAVE_BUTTON

#ifdef TPD_HAVE_BUTTON
#define TPD_YMAX		520
#define TPD_BUTTON_HEIGHT		480
#define TPD_Y_OFFSET		2

#define TPD_KEY_COUNT           3
#define TPD_KEYS                {KEY_MENU, KEY_HOMEPAGE, KEY_BACK}

#define TPD_BUTTON_SIZE_HEIGHT  (TPD_YMAX - TPD_BUTTON_HEIGHT - TPD_Y_OFFSET)+10
#define TPD_BUTTON_Y_CENTER   	(TPD_BUTTON_HEIGHT + (TPD_YMAX - TPD_BUTTON_HEIGHT)/2 + TPD_Y_OFFSET)

//TP virtual key customization

// |                                                                                                   |
// |                                                                                                   |  Touch Pad area ( H < 480)
//  ---------------------------------------------------------------------------------------------------
// |                                           TPD_Y_OFFSET                                            |  Virtual key area ( H > 480)
// |---------------------------------------------------------------------------------------------------
// |TPD_B1_FP | [TPD_B1_W] | TPD_B2_FP | [TPD_B2_W] | TPD_B3_FP | [TPD_B3_W] | TPD_B4_FP | [TPD_B4_W]  |  
// -----------------------------------------------------------------------------------------------------

/*
#define TPD_B1_FP	10		//Button 1 pad space
#define TPD_B1_W	70		//Button 1 Width
#define TPD_B2_FP	10		//Button 2 pad space
#define TPD_B2_W	70		//Button 2 Width
#define TPD_B3_FP	10		//Button 3 pad space
#define TPD_B3_W	70		//Button 3 Width
*/

#define TPD_B1_FP	13		//Button 1 pad space
#define TPD_B1_W	93		//Button 1 Width
#define TPD_B2_FP	13		//Button 2 pad space
#define TPD_B2_W	93		//Button 2 Width
#define TPD_B3_FP	13		//Button 3 pad space
#define TPD_B3_W	93		//Button 3 Width

//-------------------------------------------------------------------------
#define TPD_BUTTON1_X_CENTER	TPD_B1_FP + TPD_B1_W/2
#define TPD_BUTTON2_X_CENTER	TPD_B1_FP + TPD_B1_W + TPD_B2_FP + TPD_B2_W/2 + 1
#define TPD_BUTTON3_X_CENTER	TPD_B1_FP + TPD_B1_W + TPD_B2_FP + TPD_B2_W + TPD_B3_FP + TPD_B3_W/2 + 1


#define TPD_KEYS_DIM            {{TPD_BUTTON1_X_CENTER, TPD_BUTTON_Y_CENTER, TPD_B1_W+TPD_B1_FP, TPD_BUTTON_SIZE_HEIGHT},	\
				 			{TPD_BUTTON2_X_CENTER, TPD_BUTTON_Y_CENTER, TPD_B2_W+TPD_B1_FP, TPD_BUTTON_SIZE_HEIGHT},	\
							{TPD_BUTTON3_X_CENTER, TPD_BUTTON_Y_CENTER, TPD_B3_W+TPD_B1_FP, TPD_BUTTON_SIZE_HEIGHT}}
#endif

#endif
