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
#include <linux/slab.h>

#include <asm/uaccess.h>
#define DEVICE_NAME  "devire"/*设备名*/
#define DEVIRE_MAJOR 0 /*主设备号，0表示自动分配*/

volatile unsigned long *GPIOMODE;
volatile unsigned long *GPIO71_40_DIR;
volatile unsigned long *GPIO71_40_DATA;

static struct class *devire_drv_class;

static int devire_drv_open(struct inode *inode, struct file *file)
{
    printk("%s:Hello tangshuo open\n",__FUNCTION__);
    return 0;
}

static ssize_t devire_drv_read(struct file * file, char __user *buf, size_t size,
 loff_t *ppos)
{
    printk("%s:Hello tangshuo read\n",__FUNCTION__);
    return 0;
}

static ssize_t devire_drv_write(struct file * file, const char __user *buf, size_t size,
 loff_t *ppos)
{
    printk("%s:Hello tangshuo write\n",__FUNCTION__);
    return 0;
}

static struct file_operations devire_drv_fops = {
    .owner = THIS_MODULE,/*这是一个宏，推向编译模块时自动创建的__this_module变量*/
    .open  = devire_drv_open,
    .read  = devire_drv_read,
    .write = devire_drv_write,
};

int major;
/*执行insmod命令时就会调用这个函数*/
static int __init devire_drv_init(void)
{
    /*注册字符设备
    **参数为主设备号、设备名字、file_operations结构
    **操作主设备为devire_MAJOR的设备文件时，就会调用devire_drv_fops中的相关成员函数*/
    major = register_chrdev(DEVIRE_MAJOR, DEVICE_NAME, &devire_drv_fops);
    if(major < 0)/*注册失败*/
    {
        printk(DEVICE_NAME"can't register major number\n");
        return major;
    }
    

/*下面两行用于创建设备节点，
如果不写这两行，必须在linux系统命令行中通过mknod创建设备节点
*/
/*创建类*/
devire_drv_class = class_create(THIS_MODULE, "devire");
/*类下面创建设备节点*/
device_create(devire_drv_class, NULL, MKDEV(major, 0), NULL, "devire");

/*打印一个调试设备信息*/
printk("%s:calss_craete devire init",__FUNCTION__);
return 0;
}

/*rmmod*/
static void __exit devire_drv_exit(void)
{
    unregister_chrdev(major, "devire");/*与register配对用*/
    device_destroy(devire_drv_class, MKDEV(major, 0));/*与入口device_create*/
    class_destroy(devire_drv_class);/*与入口calss_create*/
    printk("%s:calss_craete devire exit",__FUNCTION__);

}

module_init(devire_drv_init);
module_exit(devire_drv_exit);

MODULE_LICENSE("GPL");


