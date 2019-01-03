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

//sp_1_uart232_rs485_sat
#define RS232_EN		8
#define IRIDIUM_EN		9
#define GPS_EN		10
#define MOTHER_SP1 		11
/*
#define READ_UART0 	5
#define READ_UART1 	6*/
/*sp_2_gprs_9602_wifi
#define RS232_EN	0
#define SAT_EN	1
#define RS485_EN	2
#define MOTHER_SP1 	4
#define READ_UART0 	5
#define READ_UART1 	6*/
/*
u21*/
#define ADRO2		13	//DSR_N 13
#define ADRO3		12	//DCD_N 12
#define ADRI2		11	//DTR_N 11
#define ADRI3		14	//RIN 14
/*sp_2_gprs_9602_wifi*/
#define ADRO0		30	//gpio30
#define ADRO1		31	//gpio31
#define ADRI0		29	//gpio29
#define ADRI1		28	//gpio28

volatile unsigned long *GPIOMODE;
volatile unsigned long *GPIO23_00_DIR;
volatile unsigned long *GPIO23_00_DATA;

static struct class *sp2349_class;

static unsigned long com_table[]=
	{
		ADRO2,ADRO3,ADRO0,ADRO1,// 13 12 
		ADRI3,ADRI2,ADRI1,ADRI0 // 14 11 
	};

static int mode_init(void)
{
	*GPIO23_00_DATA &= ~((1<<14)|(1<<11));//
	return 0;
}

static long read_gpio_bit(unsigned long io_num)
{
	unsigned long io=0,ret=0;
	io=23-io_num;
	ret=(0X1&(*GPIO23_00_DATA>>io));
	//printk("ret=%d \n",ret);
	return ret;
}

static int write_gpio_bit(unsigned long io_num ,unsigned long arg)
{
	unsigned char io=0;
	io=io_num-0;
	if(arg){
	  *GPIO23_00_DATA |= (arg<<io);
	}
	else{
	  *GPIO23_00_DATA &= ~((!arg)<<io);
	}
	return 0;
}
	
static int sp2349_open(struct inode *inode, struct file *file)
{
	return 0;
}

static long sp2349_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case 0:	
		case 1:
		case 2:
		case 3:
			return read_gpio_bit(com_table[cmd]);
		case 4:	
		case 5:
		case 6:
		case 7:
			write_gpio_bit(com_table[cmd],arg);
		case RS232_EN:
			*GPIO23_00_DATA &= ~((1<<14)|(1<<11));//
			udelay(250);
			break;
		case IRIDIUM_EN:
			*GPIO23_00_DATA |= (1<<14);//
			*GPIO23_00_DATA &= ~(1<<11);
			udelay(250);
			break;
		case GPS_EN:
			*GPIO23_00_DATA |= (1<<11);//
			*GPIO23_00_DATA &= ~(1<<14);//
			udelay(250);
			break;
		case MOTHER_SP1:
			*GPIO23_00_DATA |= (1<<14)|(1<<11);//
			udelay(250);
			break;
		return -EINVAL;
	}
	
	return 0;
}

/* 1.分配、设置一个file_operations结构体 */
static struct file_operations sp2349_fops = {
	.owner   			= THIS_MODULE,    				/* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open    			= sp2349_open,
	.unlocked_ioctl   		= sp2349_unlocked_ioctl,
};

int major;
static int __init sp2349_init(void)
{
	major = register_chrdev(0, "sp2349", &sp2349_fops);

	sp2349_class = class_create(THIS_MODULE, "sp2349");
	device_create(sp2349_class, NULL, MKDEV(major, 0), NULL, "sp2349");		// /dev/sp2349

	GPIOMODE = (volatile unsigned long *)ioremap(0x10000060, 4);
	GPIO23_00_DIR = (volatile unsigned long *)ioremap(0x10000624, 4);
	GPIO23_00_DATA = (volatile unsigned long *)ioremap(0x10000620, 4);
	*GPIOMODE |= (0x5<<2);//UARTF_SHARE_MODE
	*GPIO23_00_DIR |= (1<<14)|(1<<11);
	*GPIO23_00_DIR &= ~((1<<13)|(1<<12));
	mode_init();
	return 0;
}

static void __exit sp2349_exit(void)
{
	unregister_chrdev(major, "sp2349");
	device_destroy(sp2349_class, MKDEV(major, 0));
	class_destroy(sp2349_class);
	iounmap(GPIOMODE);
	iounmap(GPIO23_00_DIR);
	iounmap(GPIO23_00_DATA);
}

module_init(sp2349_init);
module_exit(sp2349_exit);

MODULE_LICENSE("GPL");


