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
#define DEVICE_NAME  "devire"/*�豸��*/
#define DEVIRE_MAJOR 0 /*���豸�ţ�0��ʾ�Զ�����*/

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
    .owner = THIS_MODULE,/*����һ���꣬�������ģ��ʱ�Զ�������__this_module����*/
    .open  = devire_drv_open,
    .read  = devire_drv_read,
    .write = devire_drv_write,
};

int major;
/*ִ��insmod����ʱ�ͻ�����������*/
static int __init devire_drv_init(void)
{
    /*ע���ַ��豸
    **����Ϊ���豸�š��豸���֡�file_operations�ṹ
    **�������豸Ϊdevire_MAJOR���豸�ļ�ʱ���ͻ����devire_drv_fops�е���س�Ա����*/
    major = register_chrdev(DEVIRE_MAJOR, DEVICE_NAME, &devire_drv_fops);
    if(major < 0)/*ע��ʧ��*/
    {
        printk(DEVICE_NAME"can't register major number\n");
        return major;
    }
    

/*�����������ڴ����豸�ڵ㣬
�����д�����У�������linuxϵͳ��������ͨ��mknod�����豸�ڵ�
*/
/*������*/
devire_drv_class = class_create(THIS_MODULE, "devire");
/*�����洴���豸�ڵ�*/
device_create(devire_drv_class, NULL, MKDEV(major, 0), NULL, "devire");

/*��ӡһ�������豸��Ϣ*/
printk("%s:calss_craete devire init",__FUNCTION__);
return 0;
}

/*rmmod*/
static void __exit devire_drv_exit(void)
{
    unregister_chrdev(major, "devire");/*��register�����*/
    device_destroy(devire_drv_class, MKDEV(major, 0));/*�����device_create*/
    class_destroy(devire_drv_class);/*�����calss_create*/
    printk("%s:calss_craete devire exit",__FUNCTION__);

}

module_init(devire_drv_init);
module_exit(devire_drv_exit);

MODULE_LICENSE("GPL");


