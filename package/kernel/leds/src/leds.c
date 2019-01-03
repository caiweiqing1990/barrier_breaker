#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/aio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>

#include <asm/rt2880/surfboardint.h>

#include "ralink_gpio.h"

#define RALINK_SYSCTL_ADDR        RALINK_SYSCTL_BASE    // system control 
#define RALINK_REG_GPIOMODE        (RALINK_SYSCTL_ADDR + 0x60) //GPIO MODE

#define RALINK_PRGIO_ADDR        RALINK_PIO_BASE // Programmable I/O

#define  RALINK_REG_PIO7140DATA  (RALINK_PRGIO_ADDR + 0x70)    //数据地址

#define  RALINK_REG_PIO7140DIR     (RALINK_PRGIO_ADDR + 0x74)    //方向地址

typedef  enum  led_stat//ioctl控制cmd 
{ 
    SET_LED_ON = 1, //led trun on 
    SET_LEN_OF,       //led trun off 
    SET_LED_DIR      //gpio方向 
}SET_LED_CMD;

//static struct class *myleds_class;

#define LED_DRIVER_NAME "led_driver" //设备名字

/** 
* 设置gpio40-gpio71模式为gpio. 
*RGMII2_GPIO_MODE 对应寄存器第10位 
*/ 
static void set_7140_gpio_mode(void) 
{ 
    u32 gpiomode; 
    
    gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE)); 
    gpiomode |= (0x1<<15); 
    *(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode); 
}


/** 
* 设置gpio40-gpio71的数据方向. 
*/ 
static void set_7140_dir(int gpio,int dir) 
{ 
    u32 gpiomode; 
    
    gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DIR)); 
    gpiomode &= ~(0x01<<(gpio-40)); 
    gpiomode |= (dir?0x01:0x0)<<(gpio-40);

    *(volatile u32 *)(RALINK_REG_PIO7140DIR) = cpu_to_le32(gpiomode);    
}


/** 
* 向gpio40-gpio71脚写数据，0 or 1. 
* 每一位对应一个引脚澹(40---71) 
*/ 
static void gpio7140_write_data(int gpio, int data) 
{ 
    unsigned long tmp;

    if(gpio < 40 || gpio > 71) 
        return; 
    
    tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DATA)); 
    tmp &= ~(0x01<<(gpio-40));//gpio位置0 
    tmp |= (data?0x01:0x0)<<(gpio-40);//gpio位置data 
    
    *(volatile u32 *)(RALINK_REG_PIO7140DATA) = cpu_to_le32(tmp);

    //printk("write data reg 0x%8x,tmp 0x%8x\n",RALINK_REG_PIO7140DATA,tmp); 
}


/** 
* led常亮. 
*/ 
static void set_led_on(unsigned long gpio) 
{ 
    gpio7140_write_data(gpio,1); 
}

/** 
* led常灭. 
*/ 
static void set_led_off(unsigned long gpio) 
{ 
    gpio7140_write_data(gpio,0); 
}

long led_driver_ioctl(struct file *filp, unsigned int req, unsigned long arg) 
{ 
    unsigned long gpio, data; 
    
    switch(req) { 
        case SET_LED_ON: 
            if(arg < 40 || arg > 71) 
                return -EINVAL; 
            set_led_on(arg); 
            //printk("[driver]led on,gpio %d.\n",(int)arg); 
            break; 
        case SET_LEN_OF: 
            if(arg < 40 || arg > 71) 
                return -EINVAL; 
            set_led_off(arg); 
            //printk("[driver]led off,gpio %d.\n",(int)arg); 
            break; 
        case SET_LED_DIR: 
            gpio = (arg&0xFFFF0000)>>16; 
            data = arg&0x0FFFF; 
            set_7140_dir(gpio, data); 
            break; 
        default: 
            return -EINVAL; 
            break; 
    } 
    return 0; 
}

static int led_driver_open(struct inode *inode, struct file *file) 
{ 
    return 0; 
}

static int led_driver_close(struct inode *inode, struct file *file) 
{ 
    return 0; 
}

static struct file_operations led_fops = //设备文件描述符 
{ 
    .owner        = THIS_MODULE, 
    .open        = led_driver_open, 
    .release    = led_driver_close, 
    .unlocked_ioctl = led_driver_ioctl, 
};

static struct miscdevice led_misc = //杂项设备 
{ 
    .minor = MISC_DYNAMIC_MINOR,  //动态设备名字 
    .name = LED_DRIVER_NAME, 
    .fops = &led_fops, 
};


static int __init led_driver_init(void) 
{ 
    int ret;

    set_7140_gpio_mode();//set RGMII2_GPIO_MODE to gpio mode.pro.p38 
    /* 3.自动创建设备节点 */
	/* 创建类 */
 //   myleds_class = class_create(THIS_MODULE, "led_driver");
	/* 类下面创建设备节点 */
  //  device_create(myleds_class, NULL, MKDEV(major, 0), NULL, "led_driver");		// /dev/myleds
    ret = misc_register(&led_misc); 
    printk("led_driver_init OK!\n"); 
    return ret; 
}

static void __exit led_driver_exit(void) 
{ 
    int ret; 
    
    ret = misc_deregister(&led_misc); 
/*    device_destroy(myleds_class, MKDEV(major, 0));
    class_destroy(myleds_class);*/
    if(ret < 0) 
    printk("led_driver_exit error.\n"); 
    printk("led_driver_exit.\n"); 
}

module_init(led_driver_init); 
module_exit(led_driver_exit); 
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("XYH");

