/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 ***************************************************************************
 *
 */
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>

#include "ralink_gpio.h"


#define RALINK_GPIO_DEVNAME	"ralink-gpio"

static dev_t		ralink_gpio_major;
static struct class	*ralink_gpio_class;
static struct cdev	ralink_gpio_cdev;
static spinlock_t	ioctllock;

MODULE_DESCRIPTION("Ralink SoC GPIO Driver");
MODULE_AUTHOR("Winfred Lu <winfred_lu@ralinktech.com.tw>");
MODULE_LICENSE("GPL");


long ralink_gpio_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	unsigned long tmp;

	switch(req) {
	case RALINK_GPIO_GET_DIR:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO_SET_DIR:
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO_GET_DATA:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO_SET_DATA:
		*(volatile u32 *)(RALINK_REG_PIODATA) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO_GET_POL:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOPOL));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO_SET_POL:
		*(volatile u32 *)(RALINK_REG_PIOPOL) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO_SET_DIR_IN:
		spin_lock(&ioctllock);
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
		tmp&= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIODIR) = tmp;
		spin_unlock(&ioctllock);
		break;
	case RALINK_GPIO_SET_DIR_OUT:
		spin_lock(&ioctllock);
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
		tmp|= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIODIR) = tmp;
		spin_unlock(&ioctllock);
		break;
	case RALINK_GPIO_SET:
		*(volatile u32 *)(RALINK_REG_PIOSET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO_CLR:
		*(volatile u32 *)(RALINK_REG_PIORESET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO_TOG:
		*(volatile u32 *)(RALINK_REG_PIOTOGGLE) = cpu_to_le32(arg);
		break;
#if defined (RALINK_GPIO_HAS_2722) || defined (RALINK_GPIO_HAS_4524) || defined (RALINK_GPIO_HAS_5124) || defined (RALINK_GPIO_HAS_7224) || defined (RALINK_GPIO_HAS_9524)
	case RALINK_GPIO24_GET_DIR:
#ifdef RALINK_GPIO_HAS_2722
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722DIR));
#else
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO24DIR));
#endif // RALINK_GPIO_HAS_2722
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO24_SET_DIR:
#ifdef RALINK_GPIO_HAS_2722
		*(volatile u32 *)(RALINK_REG_PIO2722DIR) = cpu_to_le32(arg);
#else
		*(volatile u32 *)(RALINK_REG_PIO24DIR) = cpu_to_le32(arg);
#endif // RALINK_GPIO_HAS_2722
		break;
	case RALINK_GPIO24_GET_DATA:
#ifdef RALINK_GPIO_HAS_2722
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722DATA));
#else
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO24DATA));
#endif // RALINK_GPIO_HAS_2722
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO24_SET_DATA:
#ifdef RALINK_GPIO_HAS_2722
		*(volatile u32 *)(RALINK_REG_PIO2722DATA) = cpu_to_le32(arg);
#else
		*(volatile u32 *)(RALINK_REG_PIO24DATA) = cpu_to_le32(arg);
#endif // RALINK_GPIO_HAS_2722
		break;
	case RALINK_GPIO24_GET_POL:
#ifdef RALINK_GPIO_HAS_2722
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722POL));
#else
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO24POL));
#endif // RALINK_GPIO_HAS_2722
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO24_SET_POL:
#ifdef RALINK_GPIO_HAS_2722
		*(volatile u32 *)(RALINK_REG_PIO2722POL) = cpu_to_le32(arg);
#else
		*(volatile u32 *)(RALINK_REG_PIO24POL) = cpu_to_le32(arg);
#endif // RALINK_GPIO_HAS_2722
		break;
	case RALINK_GPIO24_SET_DIR_IN:
		spin_lock(&ioctllock);
#ifdef RALINK_GPIO_HAS_2722
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722DIR));
		tmp&= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO2722DIR) = tmp;
#else
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO24DIR));
		tmp&= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO24DIR) = tmp;
#endif // RALINK_GPIO_HAS_2722
		spin_unlock(&ioctllock);
		break;
	case RALINK_GPIO24_SET_DIR_OUT:
		spin_lock(&ioctllock);
#ifdef RALINK_GPIO_HAS_2722
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722DIR));
		tmp|= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO2722DIR) = tmp;
#else
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO24DIR));
		tmp|= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO24DIR) = tmp;
#endif // RALINK_GPIO_HAS_2722
		spin_unlock(&ioctllock);
		break;
	case RALINK_GPIO24_SET:
#ifdef RALINK_GPIO_HAS_2722
		*(volatile u32 *)(RALINK_REG_PIO2722SET) = cpu_to_le32(arg);
#else
		*(volatile u32 *)(RALINK_REG_PIO24SET) = cpu_to_le32(arg);
#endif // RALINK_GPIO_HAS_2722
		break;
	case RALINK_GPIO24_CLR:
#ifdef RALINK_GPIO_HAS_2722
		*(volatile u32 *)(RALINK_REG_PIO2722RESET) = cpu_to_le32(arg);
#else
		*(volatile u32 *)(RALINK_REG_PIO24RESET) = cpu_to_le32(arg);
#endif // RALINK_GPIO_HAS_2722
		break;
	case RALINK_GPIO24_TOG:
#ifdef RALINK_GPIO_HAS_2722
		*(volatile u32 *)(RALINK_REG_PIO2722TOGGLE) = cpu_to_le32(arg);
#else
		*(volatile u32 *)(RALINK_REG_PIO24TOGGLE) = cpu_to_le32(arg);
#endif // RALINK_GPIO_HAS_2722
		break;
#endif // defined
#if defined (RALINK_GPIO_HAS_4524) || defined (RALINK_GPIO_HAS_5124) || defined (RALINK_GPIO_HAS_7224) || defined (RALINK_GPIO_HAS_9524)
	case RALINK_GPIO40_GET_DIR:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DIR));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO40_SET_DIR:
		*(volatile u32 *)(RALINK_REG_PIO7140DIR) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO40_GET_DATA:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DATA));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO40_SET_DATA:
		*(volatile u32 *)(RALINK_REG_PIO7140DATA) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO40_GET_POL:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140POL));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO40_SET_POL:
		*(volatile u32 *)(RALINK_REG_PIO7140POL) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO40_SET_DIR_IN:
		spin_lock(&ioctllock);
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DIR));
		tmp&= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO7140DIR) = tmp;
		spin_unlock(&ioctllock);
		break;
	case RALINK_GPIO40_SET_DIR_OUT:
		spin_lock(&ioctllock);
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DIR));
		tmp|= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO7140DIR) = tmp;
		spin_unlock(&ioctllock);
		break;
	case RALINK_GPIO40_SET:
		*(volatile u32 *)(RALINK_REG_PIO7140SET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO40_CLR:
		*(volatile u32 *)(RALINK_REG_PIO7140RESET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO40_TOG:
		*(volatile u32 *)(RALINK_REG_PIO7140TOGGLE) = cpu_to_le32(arg);
		break;
#endif // defined
#if defined (RALINK_GPIO_HAS_7224) || defined (RALINK_GPIO_HAS_9524)
	case RALINK_GPIO72_GET_DIR:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72DIR));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO72_SET_DIR:
		*(volatile u32 *)(RALINK_REG_PIO72DIR) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO72_GET_DATA:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72DATA));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO72_SET_DATA:
		*(volatile u32 *)(RALINK_REG_PIO72DATA) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO72_GET_POL:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72POL));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO72_SET_POL:
		*(volatile u32 *)(RALINK_REG_PIO72POL) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO72_SET_DIR_IN:
		spin_lock(&ioctllock);
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72DIR));
		tmp&= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO72DIR) = tmp;
		spin_unlock(&ioctllock);
		break;
	case RALINK_GPIO72_SET_DIR_OUT:
		spin_lock(&ioctllock);
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72DIR));
		tmp|= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO72DIR) = tmp;
		spin_unlock(&ioctllock);
		break;
	case RALINK_GPIO72_SET:
		*(volatile u32 *)(RALINK_REG_PIO72SET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO72_CLR:
		*(volatile u32 *)(RALINK_REG_PIO72RESET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO72_TOG:
		*(volatile u32 *)(RALINK_REG_PIO72TOGGLE) = cpu_to_le32(arg);
		break;
#endif // defined
#if defined (RALINK_GPIO_HAS_9532)
	case RALINK_GPIO6332_SET_DIR:
		*(volatile u32 *)(RALINK_REG_PIO6332DIR) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO6332_SET_DIR_IN:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332DIR));
		tmp &= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO6332DIR) = tmp;
		break;
	case RALINK_GPIO6332_SET_DIR_OUT:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332DIR));
		tmp |= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO6332DIR) = tmp;
		break;
	case RALINK_GPIO6332_READ:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332DATA));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO6332_WRITE:
		*(volatile u32 *)(RALINK_REG_PIO6332DATA) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO6332_SET:
		*(volatile u32 *)(RALINK_REG_PIO6332SET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO6332_CLEAR:
		*(volatile u32 *)(RALINK_REG_PIO6332RESET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO9564_SET_DIR:
		*(volatile u32 *)(RALINK_REG_PIO9564DIR) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO9564_SET_DIR_IN:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564DIR));
		tmp &= ~cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO9564DIR) = tmp;
		break;
	case RALINK_GPIO9564_SET_DIR_OUT:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564DIR));
		tmp |= cpu_to_le32(arg);
		*(volatile u32 *)(RALINK_REG_PIO9564DIR) = tmp;
		break;
	case RALINK_GPIO9564_READ:
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564DATA));
		put_user(tmp, (int __user *)arg);
		break;
	case RALINK_GPIO9564_WRITE:
		*(volatile u32 *)(RALINK_REG_PIO9564DATA) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO9564_SET:
		*(volatile u32 *)(RALINK_REG_PIO9564SET) = cpu_to_le32(arg);
		break;
	case RALINK_GPIO9564_CLEAR:
		*(volatile u32 *)(RALINK_REG_PIO9564RESET) = cpu_to_le32(arg);
		break;
#endif

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

int ralink_gpio_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);

	return 0;
}

int ralink_gpio_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);

	return 0;
}

struct file_operations ralink_gpio_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= ralink_gpio_ioctl,
	.open			= ralink_gpio_open,
	.release		= ralink_gpio_release,
};


int __init ralink_gpio_init(void)
{
	int ret = 0;
	u32 gpiomode;

	gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));

	spin_lock_init(&ioctllock);

	ret = alloc_chrdev_region(&ralink_gpio_major, 0, 1, RALINK_GPIO_DEVNAME);
	if (ret < 0) {
		printk("Failed to alloc_chrdev_region, ret=%d", ret);
		return ret;
	}

	ralink_gpio_class = class_create(THIS_MODULE, RALINK_GPIO_DEVNAME);
	if (IS_ERR(ralink_gpio_class)) {
		printk("Failed to class_create, ret=%d", (int)ralink_gpio_class);
		return -1;
	}

	device_create(ralink_gpio_class, NULL, ralink_gpio_major, NULL, RALINK_GPIO_DEVNAME);

	ralink_gpio_cdev.owner = THIS_MODULE;
	cdev_init(&ralink_gpio_cdev, &ralink_gpio_fops);
	cdev_add(&ralink_gpio_cdev, ralink_gpio_major, 1);


	printk("Ralink gpio driver initialized, RALINK_REG_GPIOMODE=%X\n", gpiomode);
	return 0;
}

void __exit ralink_gpio_exit(void)
{
	cdev_del(&ralink_gpio_cdev);
	device_destroy(ralink_gpio_class, ralink_gpio_major);
	class_destroy(ralink_gpio_class);
	unregister_chrdev_region(ralink_gpio_major, 1);

	printk("Ralink gpio driver exited\n");
}

module_init(ralink_gpio_init);
module_exit(ralink_gpio_exit);

