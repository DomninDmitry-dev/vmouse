#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/spinlock.h>
#include <linux/input.h>
#include <linux/input-polldev.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define MYDEV_NAME	"vmouse"

static int my_major = 0, my_minor = 0;
static struct input_dev *idev;
static dev_t dev;
static struct device pdev = { .init_name = MYDEV_NAME };
static struct class * cl;
static struct cdev my_cdev;

/*----------------------------------------------------------------------------*/
static int mychrdev_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	counter++;

	dev_info(&pdev,"Opening device %s:\n", MYDEV_NAME);
	dev_info(&pdev,"Counter: %d\n", counter);
	dev_info(&pdev,"Module refcounter: %d\n", module_refcount(THIS_MODULE));

	return 0;
}
/*----------------------------------------------------------------------------*/
static int mychrdev_release(struct inode *inode, struct file *file)
{
	dev_info(&pdev,"Releasing device - %s\n", MYDEV_NAME);
	return 0;
}
/*----------------------------------------------------------------------------*/
static ssize_t mychrdev_read(struct file * file, char __user *buf, size_t lbuf, loff_t *ppos)
{
    const char* message =
        "Usage: write the following commands to /dev/virtual_touchscreen:\n"
        "    x num  - move to (x, ...)\n"
        "    y num  - move to (..., y)\n"
        "    d 0    - touch down\n"
        "    u 0    - touch up\n"
        "    s slot - select multitouch slot (0 to 9)\n"
        "    a flag - report if the selected slot is being touched\n"
        "    e 0   - trigger input_mt_report_pointer_emulation\n"
        "    X num - report x for the given slot\n"
        "    Y num - report y for the given slot\n"
        "    S 0   - sync (should be after every block of commands)\n"
        "    M 0   - multitouch sync\n"
        "    T num - tracking ID\n"
        "    also 0123456789:; - arbitrary ABS_MT_ command (see linux/input.h)\n"
        "  each command is char and int: sscanf(\"%c%d\",...)\n"
        "  <s>x and y are from 0 to 1023</s> Probe yourself range of x and y\n"
        "  Each command is terminated with '\\n'. Short writes == dropped commands.\n"
        "  Read linux Documentation/input/multi-touch-protocol.txt to read about events\n";
    const size_t msgsize = strlen(message);
    if (*ppos >= msgsize) {
		return 0;
	}
	if (lbuf > msgsize - *ppos) {
		lbuf = msgsize - *ppos;
	}
	int nbytes = lbuf - copy_to_user(buf, *ppos + message, lbuf);

	dev_info(&pdev,"Read device %s nbytes = %d, ppos = %d:\n", MYDEV_NAME, nbytes, (int)*ppos);

	*ppos += nbytes;
	return nbytes;
}
/*----------------------------------------------------------------------------*/
static ssize_t mychrdev_write(struct file * file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
/*	int nbytes = lbuf - copy_from_user(*ppos, buf, lbuf);
	*ppos += nbytes;
	kbuf[*ppos] = 0;

	dev_info(&pdev,"Write device %s nbytes = %d, ppos = %d, data: %s\n\n", MYDEV_NAME, nbytes, (int)*ppos, kbuf);

	return nbytes;*/
}
/*----------------------------------------------------------------------------*/
static const struct file_operations mycdev_fops = {
	.owner = THIS_MODULE,
	.read = mychrdev_read,
	.write = mychrdev_write,
	.open = mychrdev_open,
	.release = mychrdev_release
};
/*----------------------------------------------------------------------------*/
int init_module(void)
{
	int ret;

	dev_info(&pdev,"Hello, loading module\n");

	// Задаем статически младший и старший номер устройства
	if(my_major != 0) { // Статически из параметра модуля
		dev = MKDEV(my_major, my_minor);
		ret = register_chrdev_region(dev, 1, MYDEV_NAME);
	}
	else { // Динамически
		ret = alloc_chrdev_region(&dev, my_minor, 1, MYDEV_NAME);
		my_major = MAJOR(dev);  // не забыть зафиксировать!
	}
	if(ret < 0) {
		dev_info(&pdev,"Can not register char device region\n");
		goto err;
	}
	dev_info(&pdev,"Device registered: MAJOR = %d, MINOR = %d\n", my_major, my_minor);

	cdev_init(&my_cdev, &mycdev_fops);
	ret = cdev_add(&my_cdev, dev, 1);

	cl = class_create(THIS_MODULE, MYDEV_NAME);
	if (!IS_ERR(cl)) {
		device_create(cl, NULL, dev, "%s", MYDEV_NAME);
	}

	return 0;

err:
	return -1;
}
/*----------------------------------------------------------------------------*/
void cleanup_module(void)
{
    if (!IS_ERR(cl)) {
	    device_destroy(cl, dev);
	    class_destroy(cl);
	    cdev_del(&my_cdev);
    }
    unregister_chrdev_region(dev, 1);

	dev_info(&pdev, "Exit module\n");
}
/*----------------------------------------------------------------------------*/

MODULE_LICENSE("GPL"); /* Лицензия */
MODULE_AUTHOR("Dmitry Domnin"); /* Автор */
MODULE_DESCRIPTION("Driver for /dev/vmouse"); /* Описание */
MODULE_SUPPORTED_DEVICE(MYDEV_NAME); /* Устройство */
