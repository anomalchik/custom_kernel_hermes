#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/i2c.h>
#include <linux/kthread.h>
#include "aw9110b.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpt.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#define USE_UNLOCKED_IOCTL


#include <linux/hrtimer.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
//#include <linux/io.h>

#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/interrupt.h>
#include <linux/rtpm_prio.h>

#include <linux/proc_fs.h>
#include <mach/mt_boot.h>

#include <cust_eint.h>
#include <linux/jiffies.h>
#include <pmic_drv.h>



//s_add new breathlight driver here

//e_add new breathlight driver here
/******************************************************************************
 * Definition
******************************************************************************/
#ifndef TRUE
#define TRUE KAL_TRUE
#endif
#ifndef FALSE
#define FALSE KAL_FALSE
#endif

/* device name and major number */
#define BREATHLIGHT_DEVNAME            "breathlight"

#define DELAY_MS(ms) {mdelay(ms);}	//unit: ms(10^-3)
#define DELAY_US(us) {mdelay(us);}	//unit: us(10^-6)
#define DELAY_NS(ns) {mdelay(ns);}	//unit: ns(10^-9)
/*
    non-busy dealy(/kernel/timer.c)(CANNOT be used in interrupt context):
        ssleep(sec)
        msleep(msec)
        msleep_interruptible(msec)

    kernel timer
*/

#define ALLOC_DEVNO

/******************************************************************************
 * Project specified configuration
******************************************************************************/
#define AW9110B_I2C_ADDR 0x5B
#define AW9110B_I2C_BUSNUM 2
#define AW9110B_EN_PIN                  (GPIO151 | 0x80000000)

static U8 AW9110_timer_ID;
// P1_0~7依次对应OUT0~3,OUT12~15
// P0_0~7依次对应OUT4~11

U8 Paoma_cnt=0;
U8 Breath_cnt=0;
U8 cnt2=0;

int loop_flag = 0;
int charging_count = 0;
static struct task_struct *charging_thread = NULL;

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[BREATHLIGHT]"

#define DEBUG_BREATHLIGHT
#ifdef DEBUG_BREATHLIGHT
#define BL_DBG(fmt, arg...) printk(PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define BL_TRC(fmt, arg...) printk(PFX "<%s>\n", __FUNCTION__);
#define BL_ERR(fmt, arg...) printk(PFX "%s: " fmt, __FUNCTION__ ,##arg)
#else
#define BL_DBG(a,...)
#define BL_TRC(a,...)
#define BL_ERR(a,...)
#endif

/*****************************************************************************

*****************************************************************************/
#define BREATHLIGHT_DEVICE "aw9110b"

static struct task_struct *thread = NULL;
static struct task_struct *probe_thread = NULL;
static struct i2c_client *aw9110b_i2c_client = NULL;

static const struct i2c_device_id bl_id[] = { {BREATHLIGHT_DEVICE, 0}, {} };
static struct i2c_board_info __initdata i2c_aw9110b = { I2C_BOARD_INFO("aw9110b", (AW9110B_I2C_ADDR)) };

//      static unsigned short force[] = {0,AW9110B_I2C_ADDR,I2C_CLIENT_END,I2C_CLIENT_END};
//      static const unsigned short * const forces[] = { force, NULL };
//      static struct i2c_client_address_data addr_data = { .forces = forces, };
void aw9110b_pwron(int status)
{
    if(status){
        mt_set_gpio_mode(AW9110B_EN_PIN,GPIO_MODE_00);
        mt_set_gpio_dir(AW9110B_EN_PIN,GPIO_DIR_OUT);
        mt_set_gpio_out(AW9110B_EN_PIN,GPIO_OUT_ONE);
        msleep(50);
        mt_set_gpio_out(AW9110B_EN_PIN,GPIO_OUT_ZERO);
        msleep(50);
        mt_set_gpio_out(AW9110B_EN_PIN,GPIO_OUT_ONE);
      }
    else{
          mt_set_gpio_out(AW9110B_EN_PIN,GPIO_OUT_ZERO);
        }
}

void aw9110b_HW_reset(void)
{
    aw9110b_pwron(0);
    msleep(50);
    aw9110b_pwron(1);
}
static int aw9110b_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    
    int err = 0; 
	aw9110b_i2c_client = client;

	BL_DBG("[aw9110b_probe] E\n");
	return 0;
}

static int aw9110b_remove(struct i2c_client *client)
{
	BL_DBG("[aw9110b_remove] E\n");
	return 0;
}

static int aw9110b_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	BL_DBG("[aw9110b_detect] E\n");

	strcpy(info->type, BREATHLIGHT_DEVICE);
	return 0;
}

static struct i2c_driver aw9110b_i2c_driver = {
    .driver = {
           .name = BREATHLIGHT_DEVICE,
//              .owner = THIS_MODULE,
           },
    .probe = aw9110b_probe,
    .remove = aw9110b_remove,
    .id_table = bl_id,
    .detect = aw9110b_detect,
//       .address_data = &addr_data,
};

/*****************************************************************************

*****************************************************************************/
#ifdef USE_UNLOCKED_IOCTL
static int breathlight_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int breathlight_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	int i4RetValue = 0;

//	BL_DBG("%x, %x \n", cmd, arg);

	switch (cmd) {
	case BREATHLIGHT_IOCTL_INIT:
		BL_DBG("BREATHLIGHT_IOCTL_INIT\n");
		break;

	case BREATHLIGHT_IOCTL_MODE:
		BL_DBG("BREATHLIGHT_IOCTL_MODE\n");
		break;

	case BREATHLIGHT_IOCTL_CURRENT:
		BL_DBG("BREATHLIGHT_IOCTL_CURRENT\n");
		break;

	case BREATHLIGHT_IOCTL_ONOFF:
		BL_DBG("BREATHLIGHT_IOCTL_ONOFF\n");
		break;

	case BREATHLIGHT_IOCTL_PWM_ONESHOT:
		BL_DBG("BREATHLIGHT_IOCTL_ONOFF\n");
		break;

	case BREATHLIGHT_IOCTL_TIME:
		BL_DBG("BREATHLIGHT_IOCTL_ONOFF\n");
		break;

	default:
		break;
	}

	return i4RetValue;
}

static ssize_t breathlight_read(struct file *file, const char __user * user_buf, size_t count,loff_t * ppos)
{
	int i4RetValue = 0;
	BL_DBG("[breathlight_read] E\n");

	return i4RetValue;
}

static ssize_t breathlight_write(struct file *file, char __user * buf, size_t size, loff_t * ppos)
{
	int i4RetValue = 0;
	BL_DBG("[breathlight_write] E\n");

	return size;
}

static int breathlight_open(struct inode *inode, struct file *file)
{
	int i4RetValue = 0;
	BL_DBG("[breathlight_open] E\n");

	return i4RetValue;
}

static int breathlight_release(struct inode *inode, struct file *file)
{
	int i4RetValue = 0;
	BL_DBG("[breathlight_release] E\n");

	return i4RetValue;
}

/*****************************************************************************/
/* Kernel interface */
static struct file_operations breathlight_fops = {
	.owner = THIS_MODULE,
#ifdef USE_UNLOCKED_IOCTL
	.unlocked_ioctl = breathlight_ioctl,
#else
	.ioctl = breathlight_ioctl,
#endif
	.open = breathlight_open,
	.release = breathlight_release,
	.read = breathlight_read,
	.write = breathlight_write,
};

/*****************************************************************************
Driver interface
*****************************************************************************/
struct breathlight_data {
	spinlock_t lock;
	wait_queue_head_t read_wait;
	struct semaphore sem;
};
static struct class *breathlight_class = NULL;
static struct device *breathlight_device = NULL;
//static struct breathlight_data breathlight_private;
static dev_t breathlight_devno;
static struct cdev breathlight_cdev;

static int breath_num = 0;
static int breath_time = 0;
/****************************************************************************/

static unsigned char g_Ledctrl1 = 0, g_Ledctrl2 = 0, g_LedCurrentSet = 1, g_LedAudioGainSet = 4;

void RE_IIC_WriteReg(unsigned char index, unsigned char value)
{
	unsigned char databuf[2];
	int res;

	databuf[0] = index;
	databuf[1] = value;
	res = i2c_master_send(aw9110b_i2c_client, databuf, 0x2);
	if (res <= 0) {
		BL_ERR("[breathlight_init] i2c_master_send fail\n");
	}
}

enum {
  Off = 0,
  Charging,
  AllOn,
  ComeCall,
  MissCall,
  ComeMsg,
  MissMsg,
};

/*********************************************************/
//Function list
/********************************************************/
void AW9110_SoftReset()
{ 
    int ret =0;
    if(charging_count){
      charging_count = 0;
      ret = kthread_stop(charging_thread);
    }
    msleep(500);
    aw9110b_HW_reset();
    RE_IIC_WriteReg(0x7f,0x00); 
    msleep(30);
}
//-------------------------------------------------------------------------------------------
//函数名: AW9110_AllOn
//调用此函数，则10路灯全亮，每路的电流分别由软件独立控制。
//-------------------------------------------------------------------------------------------
void AW9110_AllOn(void)
{
    BL_DBG("-------------------------AW9110_AllOn  Entry ------------------------- \r\n");

    AW9110_SoftReset();
    RE_IIC_WriteReg(0x12,0x00);   //OUT4~9ÅäÖÃÎªºôÎüµÆÄ£Ê½
    RE_IIC_WriteReg(0x13,0x00);   //OUT0~3ÅäÖÃÎªºôÎüµÆÄ£Ê½
    RE_IIC_WriteReg(0x20,0x3f);//OUT0¿Úµ÷¹â£¬µ÷¹âµÈ¼¶Îª0-255¡£OUT0~OUT9µÄµ÷¹âÖ¸ÁîÒÀ´ÎÎª0x20~0x29. Ð´0¹Ø±Õ
    RE_IIC_WriteReg(0x21,0x3f);
    RE_IIC_WriteReg(0x22,0x3f);
    RE_IIC_WriteReg(0x23,0x3f);
    RE_IIC_WriteReg(0x24,0x3f);
    RE_IIC_WriteReg(0x25,0x3f);
}

//-------------------------------------------------------------------------------------------
//函数名: AW9110_init_pattern
//调用此函数，则10路灯全亮，前6路为自主呼吸BLINK 模式，后6路为被动调光
//-------------------------------------------------------------------------------------------
void AW9110_charging(void) 
{
 //AW9110基本的效果实现。前6路自主呼吸，后6路点亮
    BL_DBG("-------------------------AW9110_init_pattern  Entry ------------------------- \r\n");
    do{
        RE_IIC_WriteReg(0x12,0x00);   //OUT4~9配置为呼吸灯模式
        RE_IIC_WriteReg(0x13,0x00);   //OUT0~3配置为呼吸灯模式
        RE_IIC_WriteReg(0x04,0x00);   //OUT4-OUT5自主呼吸BLINK模式使能
        RE_IIC_WriteReg(0x05,0x00);   //OUT0-OUT3自主呼吸BLINK模式使能
         
        RE_IIC_WriteReg(0x15,0x03);      //淡进淡出时间设置
        RE_IIC_WriteReg(0x16,0x00);  //全亮全暗时间设置 
            
        RE_IIC_WriteReg(0x14,0x3f);//自主呼吸使能 
        RE_IIC_WriteReg(0x11,0x02); //开始自主呼吸，并设置最大电流

        RE_IIC_WriteReg(0x02,0x00);//自主呼吸使能 
        RE_IIC_WriteReg(0x03,0x00);//自主呼吸使能 
        RE_IIC_WriteReg(0x03,0x01);//自主呼吸使能 
        mdelay(500);
        RE_IIC_WriteReg(0x03,0x03);//自主呼吸使能 
        mdelay(500);
        RE_IIC_WriteReg(0x03,0x0b);//自主呼吸使能 
        mdelay(500);
        RE_IIC_WriteReg(0x03,0x0f);//自主呼吸使能 
        mdelay(500);
        RE_IIC_WriteReg(0x03,0x0f);//自主呼吸使能 
        RE_IIC_WriteReg(0x02,0x01);//自主呼吸使能 
        mdelay(500);
        RE_IIC_WriteReg(0x03,0x0f);//自主呼吸使能 
        RE_IIC_WriteReg(0x02,0x03);//自主呼吸使能 
        mdelay(500);
        RE_IIC_WriteReg(0x7f,0x00);
        msleep(500);
    }while(!kthread_should_stop());
}

void charging_func(void)
{
    AW9110_SoftReset();
    charging_count = 1;
    charging_thread = kthread_run(AW9110_charging,0,BREATHLIGHT_DEVNAME);   
    if(IS_ERR(charging_thread)){
      BL_DBG("charging thread create failed!!!");
    }
    else{
      BL_DBG("charging thread create OK!!!");
    }
}

//-------------------------------------------------------------------------------------------
//函数名: AW9110_ComeCall
//来电时的效果。6路自主呼吸，呼吸频率较快
//-------------------------------------------------------------------------------------------
void AW9110_ComeCall(void)
{
  BL_DBG("-------------------------AW9110_ComeCall  Entry ------------------------- \r\n");

  AW9110_SoftReset();
  RE_IIC_WriteReg(0x12,0x00);   //OUT4~9配置为呼吸灯模式
  RE_IIC_WriteReg(0x13,0x00);   //OUT0~3配置为呼吸灯模式
  
   RE_IIC_WriteReg(0x04,0x03);   //OUT4-OUT5自主呼吸BLINK模式使能
   RE_IIC_WriteReg(0x05,0x0f);   //OUT0-OUT3自主呼吸BLINK模式使能   
   RE_IIC_WriteReg(0x15,0x12);      //淡进淡出时间设置
   RE_IIC_WriteReg(0x16,0x09);  //全亮全暗时间设置              
   RE_IIC_WriteReg(0x14,0x3f);//自主呼吸使能    
   RE_IIC_WriteReg(0x11,0x82); 
}

//-------------------------------------------------------------------------------------------
//函数名: AW9110_MissCall
//有未接来电时的效果。只有2 路自主呼吸，其他灯灭。呼吸频率较慢，并且有全暗时间
//-------------------------------------------------------------------------------------------
void AW9110_MissCall(void)
{

  BL_DBG("-------------------------AW9110_MissCall  Entry ------------------------- \r\n");

  AW9110_SoftReset();
  RE_IIC_WriteReg(0x12,0x00);   //OUT4~9配置为呼吸灯模式
  RE_IIC_WriteReg(0x13,0x00);   //OUT0~3配置为呼吸灯模式
  
   RE_IIC_WriteReg(0x04,0x03);   //OUT4-OUT5自主呼吸BLINK模式使能
   RE_IIC_WriteReg(0x05,0x0f);   //OUT0-OUT3自主呼吸BLINK模式使能   
   RE_IIC_WriteReg(0x15,0x09);      //淡进淡出时间设置
   RE_IIC_WriteReg(0x16,0x20);  //全亮全暗时间设置              
   RE_IIC_WriteReg(0x14,0x30);//自主呼吸使能    
   RE_IIC_WriteReg(0x11,0x83); 
}

//-------------------------------------------------------------------------------------------
//函数名: AW9110_ComeMsg
//来短信时的效果。1路自主呼吸，呼吸频率较快
//-------------------------------------------------------------------------------------------
void AW9110_ComeMsg(void)
{

  BL_DBG("-------------------------AW9110_ComeMsg Entry ------------------------- \r\n");

  AW9110_SoftReset();
  RE_IIC_WriteReg(0x12,0x00);   //OUT4~9配置为呼吸灯模式
  RE_IIC_WriteReg(0x13,0x00);   //OUT0~3配置为呼吸灯模式
  
   RE_IIC_WriteReg(0x04,0x03);   //OUT4-OUT5自主呼吸BLINK模式使能
   RE_IIC_WriteReg(0x05,0x0f);   //OUT0-OUT3自主呼吸BLINK模式使能   
   RE_IIC_WriteReg(0x15,0x09);      //淡进淡出时间设置
   RE_IIC_WriteReg(0x16,0x10);  //全亮全暗时间设置              
   RE_IIC_WriteReg(0x14,0x20);    //自主呼吸使能    
   RE_IIC_WriteReg(0x11,0x82); 
}

//-------------------------------------------------------------------------------------------
//函数名: AW9110_MissMsg
//有未接  短信时的效果。只有一路自主呼吸，呼吸频率较慢，并且有全暗时间
//-------------------------------------------------------------------------------------------
void AW9110_MissMsg (void)
{
  BL_DBG("-------------------------AW9110_MissMsg Entry ------------------------- \r\n");
  AW9110_SoftReset();
  RE_IIC_WriteReg(0x12,0x00);   //OUT4~9配置为呼吸灯模式
  RE_IIC_WriteReg(0x13,0x00);   //OUT0~3配置为呼吸灯模式
  
   RE_IIC_WriteReg(0x04,0x03);   //OUT4-OUT5自主呼吸BLINK模式使能
   RE_IIC_WriteReg(0x05,0x0f);   //OUT0-OUT3自主呼吸BLINK模式使能   
   RE_IIC_WriteReg(0x15,0x09);      //淡进淡出时间设置
   RE_IIC_WriteReg(0x16,0x20);  //全亮全暗时间设置              
   RE_IIC_WriteReg(0x14,0x20);//自主呼吸使能    
   RE_IIC_WriteReg(0x11,0x82); 
}

//-------------------------------------------------------------------------------------------
//函数名: AW9110_GPIO_LED
//当用GPIO 6789来控制LED的阳极，则下面这个函数能实现基本的呼吸功能
//-------------------------------------------------------------------------------------------
void AW9110_GPIO_LED(void)
{

  BL_DBG("-------------------------AW9110_GPIO_LED Entry ------------------------- \r\n");

  AW9110_SoftReset();
  RE_IIC_WriteReg(0x12,0xfc);   //OUT4、5配置为呼吸灯模式，OUT6、7、8、9配置为GPIO模式
  RE_IIC_WriteReg(0x13,0x00);   //OUT0~3配置为呼吸灯模式

  //下面这两条指令配置OUT6~9 输出高电平。
  //用户可根据需要来调整某几路阳极为高，或通过timer来循环以达到想要的效果
  RE_IIC_WriteReg(0x11,0x10);   //OUT6 ~9 配置为推挽模式
  RE_IIC_WriteReg(0x02,0xfc);   //OUT6~9输出高电平。
  
   RE_IIC_WriteReg(0x04,0x03); //OUT4-OUT5自主呼吸BLINK模式使能
   RE_IIC_WriteReg(0x05,0x0f); //OUT0-OUT3自主呼吸BLINK模式使能     
   RE_IIC_WriteReg(0x15,0x12);  //淡进淡出时间设置
   RE_IIC_WriteReg(0x16,0x09);  //全亮全暗时间设置              
   RE_IIC_WriteReg(0x14,0x3f);  //自主呼吸使能      
   RE_IIC_WriteReg(0x11,0x92);     //开始自主呼吸。注意    OUT6 ~9 必须配置为推挽模式
}

//-------------------------------------------------------------------------------------------
//函数名: End
//-------------------------------------------------------------------------------------------

void led_breath_turnon()
{
    switch (breath_num)
    {
	  case Charging:
           charging_func();
	  break;
	  case AllOn:
           AW9110_AllOn();
	  break;
      case ComeCall:
           AW9110_ComeCall();
      break;

      case MissCall:
           AW9110_MissCall();
      break;

      case ComeMsg:
           AW9110_ComeMsg();
      break;

      case MissMsg:
           AW9110_MissMsg();
      break;

	  default:
           AW9110_SoftReset();
	    break;
    }
}

static size_t breaghlight_show_mode(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	BL_DBG();

	return snprintf(buf, PAGE_SIZE, "breaghlight_show_mode\n");
}

static size_t breathlight_store_mode(struct device *dev, struct device_attribute *attr,
				     const char *buf, size_t size)
{
	BL_DBG();

	return size;
}

static size_t breaghlight_show_pwm(struct device *dev, struct device_attribute *attr,
				   char *buf)
{
	BL_DBG();

	return snprintf(buf, PAGE_SIZE, "breaghlight_show_pwm\n");
}

static size_t breathlight_store_pwm(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	BL_DBG();

	return size;
}

static size_t breaghlight_show_open(struct device *dev, struct device_attribute *attr, char *buf)
{
	BL_DBG();

	return snprintf(buf, PAGE_SIZE, "breaghlight_show_open\n");
}

static size_t breathlight_store_open(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	BL_DBG("[breathlight_dev_open] E\n");

	sscanf(buf, "%d", &breath_num);
	BL_DBG("breathlight num=%d\n",breath_num);
    led_breath_turnon();
	return size;
}

static size_t breaghlight_show_rgb(struct device *dev, struct device_attribute *attr, char *buf)
{
	BL_DBG();

	return snprintf(buf, PAGE_SIZE, "this rgb has four parameter R_level G_level B_level mode\n");
}

static size_t breathlight_store_rgb(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int mode,levelr,levelg,levelb;
	if(4 == sscanf(buf,"%d %d %d %d",&levelr,&levelg,&levelb,&mode))
    Led_Set_Mode(levelr,levelg,levelb,mode);
	return size;
}

static size_t breaghlight_show_time(struct device *dev, struct device_attribute *attr, char *buf)
{
	BL_DBG();

	return snprintf(buf, PAGE_SIZE, "breaghlight_show_time\n");
}

static size_t breathlight_store_time(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	BL_DBG("[breathlight_dev_time] E\n");

	sscanf(buf, "%d", &breath_time);
	BL_DBG("breathlight time =%d\n",breath_time);
	return size;
}

static DEVICE_ATTR(mode, 0664, breaghlight_show_mode, breathlight_store_mode);
static DEVICE_ATTR(pwm, 0664, breaghlight_show_pwm, breathlight_store_pwm);
static DEVICE_ATTR(open, 0666, breaghlight_show_open, breathlight_store_open);
//static DEVICE_ATTR(rgb, 0666, breaghlight_show_rgb, breathlight_store_rgb);
static DEVICE_ATTR(time, 0666, breaghlight_show_time, breathlight_store_time);

static struct device_attribute *breathlight_attr_list[] = {
	&dev_attr_mode,
	&dev_attr_pwm,
	&dev_attr_open,
//	&dev_attr_rgb,
	&dev_attr_time,
};

static int breathlight_create_attr(struct device *dev)
{
	int idx, err = 0;
	int num =
	    (int) sizeof(breathlight_attr_list) /
	    sizeof(breathlight_attr_list[0]);

	if (!dev)
		return -EINVAL;

	for (idx = 0; idx < num; idx++) {
		if (err =
		    device_create_file(dev, breathlight_attr_list[idx]))
			break;
	}
	return err;
}

static int breathlight_delete_attr(struct device *dev)
{
	int idx, err = 0;
	int num =
	    (int) sizeof(breathlight_attr_list) /
	    sizeof(breathlight_attr_list[0]);

	if (!dev)
		return -EINVAL;

	for (idx = 0; idx < num; idx++) {
		device_remove_file(dev, breathlight_attr_list[idx]);
	}

	return err;
}

static int breathlight_probe(struct platform_device *dev)
{
	int ret = 0, err = 0;

	BL_DBG("[breathlight_probe] start\n");

#ifdef ALLOC_DEVNO		//dynamic allocate device no
	
    ret = alloc_chrdev_region(&breathlight_devno, 0, 1,	BREATHLIGHT_DEVNAME);
	if (ret) {
		BL_ERR("[breathlight_probe] alloc_chrdev_region fail: %d\n", ret);
		goto breathlight_probe_error;
	} else {
		BL_DBG("[breathlight_probe] major: %d, minor: %d\n",
		       MAJOR(breathlight_devno), MINOR(breathlight_devno));
	}
	cdev_init(&breathlight_cdev, &breathlight_fops);
	breathlight_cdev.owner = THIS_MODULE;
	err = cdev_add(&breathlight_cdev, breathlight_devno, 1);
	if (err) {
		BL_ERR("[breathlight_probe] cdev_add fail: %d\n", err);
		goto breathlight_probe_error;
	}
#else
#define BREATHLIGHT_MAJOR 242
	ret = register_chrdev(BREATHLIGHT_MAJOR, BREATHLIGHT_DEVNAME, &breathlight_fops);
	if (ret != 0) {
		BL_ERR("[breathlight_probe] Unable to register chardev on major=%d (%d)\n", BREATHLIGHT_MAJOR, ret);
		return ret;
	}
	breathlight_devno = MKDEV(BREATHLIGHT_MAJOR, 0);
#endif


	breathlight_class = class_create(THIS_MODULE, "breathlightdrv");
	if (IS_ERR(breathlight_class)) {
		BL_ERR("[breathlight_probe] Unable to create class, err = %d\n",(int) PTR_ERR(breathlight_class));
		goto breathlight_probe_error;
	}

	breathlight_device = device_create(breathlight_class, NULL, breathlight_devno, NULL, BREATHLIGHT_DEVNAME);
	if (NULL == breathlight_device) {
		BL_ERR("[breathlight_probe] device_create fail\n");
		goto breathlight_probe_error;
	}

	if (breathlight_create_attr(breathlight_device))
		BL_ERR("create_attr fail\n");

	if (i2c_add_driver(&aw9110b_i2c_driver) != 0) {
		BL_ERR("unable to add i2c driver.\n");
		return -1;
	}

	BL_DBG("[breathlight_probe] Done\n");
	return 0;

      breathlight_probe_error:
#ifdef ALLOC_DEVNO
	if (err == 0)
		cdev_del(&breathlight_cdev);
	if (ret == 0)
		unregister_chrdev_region(breathlight_devno, 1);
#else
	if (ret == 0)
		unregister_chrdev(MAJOR(breathlight_devno),
				  BREATHLIGHT_DEVNAME);
#endif
	return -1;
}

static int breathlight_remove(struct platform_device *dev)
{

	BL_DBG("[breathlight_remove] start\n");

	if (breathlight_delete_attr(breathlight_device))
		BL_ERR("delete_attr fail\n");

#ifdef ALLOC_DEVNO
	cdev_del(&breathlight_cdev);
	unregister_chrdev_region(breathlight_devno, 1);
#else
	unregister_chrdev(MAJOR(breathlight_devno), BREATHLIGHT_DEVNAME);
#endif
	device_destroy(breathlight_class, breathlight_devno);
	class_destroy(breathlight_class);

	BL_DBG("[breathlight_remove] Done\n");
	return 0;
}

static int breathlight_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int breathlight_resume(struct platform_device *pdev)
{
    return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id breathlight_of_match[] = {
    { .compatible = "mediatek,breathlight", },
    {},
};
#endif

static struct platform_driver breathlight_platform_driver = {
	.probe = breathlight_probe,
    .suspend = breathlight_suspend,
    .resume = breathlight_resume,
	.remove = breathlight_remove,
	.driver = {
		   .name = BREATHLIGHT_DEVNAME,
		   .owner = THIS_MODULE,
     #ifdef CONFIG_OF
     .of_match_table = breathlight_of_match,
     #endif
		   },

};

static int __init breathlight_init(void)
{
	int ret = 0;
	BL_DBG("[breathlight_init] start\n");

    aw9110b_pwron(1);
	i2c_register_board_info(AW9110B_I2C_BUSNUM, &i2c_aw9110b, 1);
	ret = platform_driver_register(&breathlight_platform_driver);
	if (ret) {
		BL_ERR
		    ("[breathlight_init] platform_driver_register fail\n");
		return ret;
	}

	BL_DBG("[breathlight_init] done!\n");
	return ret;
}

static void __exit breathlight_exit(void)
{
	BL_DBG("[breathlight_exit] start\n");
    aw9110b_pwron(0);
	platform_driver_unregister(&breathlight_platform_driver);
	BL_DBG("[breathlight_exit] done!\n");
}

/*****************************************************************************/
module_init(breathlight_init);
module_exit(breathlight_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arnold Lu <lubaoquan@vanzotec.com>");
MODULE_DESCRIPTION("Breath LED light");
