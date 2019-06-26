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

struct vmouse_device {
	int my_major, my_minor;
    signed char data[4];     /* use a 4-byte protocol */
    struct input_dev *idev;   /* input device, to push out input  data */
    int x, y;                /* keep track of the position of this device */
    dev_t dev;
    struct device pdev;
    struct class * cl;
    struct cdev my_cdev;
};

static struct vmouse_device vmouse = { .pdev.init_name = MYDEV_NAME };

/*----------------------------------------------------------------------------*/
static int mychrdev_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	counter++;

	dev_info(&vmouse.pdev,"Opening device %s:\n", MYDEV_NAME);
	dev_info(&vmouse.pdev,"Counter: %d\n", counter);
	dev_info(&vmouse.pdev,"Module refcounter: %d\n", module_refcount(THIS_MODULE));

	return 0;
}
/*----------------------------------------------------------------------------*/
static int mychrdev_release(struct inode *inode, struct file *file)
{
	dev_info(&vmouse.pdev,"Releasing device - %s\n", MYDEV_NAME);
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
    int nbytes;

    //dev_info(&pdev, "ppos = %d\n", *ppos);
    //dev_info(&pdev, "lbuf = %d\n", lbuf);
    //dev_info(&pdev, "msgsize = %d\n", msgsize);


    if(*ppos >= msgsize) {
		return 0;
	}
	if(lbuf > msgsize - *ppos) {
		lbuf = msgsize - *ppos;
	}
	nbytes = lbuf - copy_to_user(buf, *ppos + message, lbuf);

	dev_info(&vmouse.pdev,"Read device %s nbytes = %d, ppos = %d:\n", MYDEV_NAME, nbytes, (int)*ppos);

	*ppos += nbytes;

	return nbytes;
}
/*----------------------------------------------------------------------------*/
static ssize_t mychrdev_write(struct file * file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
	char mybuf[64] = {0};
	int nbytes, len;
	char command;
	int arg;

	if(lbuf > sizeof(mybuf)) lbuf = sizeof(mybuf);

	nbytes = lbuf - copy_from_user(mybuf + *ppos, buf, lbuf);
	*ppos += nbytes;

	dev_info(&vmouse.pdev,"Write device %s nbytes = %d, ppos = %d, data: %s\n", MYDEV_NAME, nbytes, (int)*ppos, mybuf);

	for(len=0; len<lbuf; ++len) {
		if (mybuf[len] == '\n') {
			mybuf[len] = '\0';
			if(sscanf(mybuf, "%c%d", &command, &arg) != 2) {
				dev_info(&vmouse.pdev, "sscanf failed to interpret this input\n");
			}
			dev_info(&vmouse.pdev, "command = %c, arg = %d", command, arg);
			switch(command) {
				case 'r': {
					//input_report_rel(vmouse.idev, REL_X, 100);
					//input_report_rel(vmouse.idev, REL_Y, 100);
					//right click press down
					input_report_key(vmouse.idev, BTN_LEFT, 1);
					input_sync(vmouse.idev);
					input_report_key(vmouse.idev, BTN_LEFT, 0);
					input_sync(vmouse.idev);
					break;
				}
			}
		}
	}
	//input_sync(vmouse.idev);

	return nbytes;
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

	dev_info(&vmouse.pdev,"Hello, loading module\n");

	// Задаем статически младший и старший номер устройства
	if(vmouse.my_major != 0) { // Статически из параметра модуля
		vmouse.dev = MKDEV(vmouse.my_major, vmouse.my_minor);
		ret = register_chrdev_region(vmouse.dev, 1, MYDEV_NAME);
	}
	else { // Динамически
		ret = alloc_chrdev_region(&vmouse.dev, vmouse.my_minor, 1, MYDEV_NAME);
		vmouse.my_major = MAJOR(vmouse.dev);  // не забыть зафиксировать!
	}
	if(ret < 0) {
		dev_info(&vmouse.pdev,"Can not register char device region\n");
		goto err;
	}
	dev_info(&vmouse.pdev,"Device registered: MAJOR = %d, MINOR = %d\n", vmouse.my_major, vmouse.my_minor);

	cdev_init(&vmouse.my_cdev, &mycdev_fops);
	ret = cdev_add(&vmouse.my_cdev, vmouse.dev, 1);

	vmouse.cl = class_create(THIS_MODULE, MYDEV_NAME);
	if (!IS_ERR(vmouse.cl)) {
		device_create(vmouse.cl, NULL, vmouse.dev, "%s", MYDEV_NAME);
	}

	vmouse.idev = input_allocate_device();
	if (!vmouse.idev) {
		dev_info(&vmouse.pdev,"Not enough memory\n");
		goto err;
	}

	set_bit(EV_REL, vmouse.idev->evbit);
	set_bit(REL_X, vmouse.idev->relbit);
	set_bit(REL_Y, vmouse.idev->relbit);
	set_bit(REL_WHEEL, vmouse.idev->relbit);

	set_bit(EV_KEY, vmouse.idev->evbit);
	set_bit(BTN_LEFT, vmouse.idev->keybit);
	set_bit(BTN_MIDDLE, vmouse.idev->keybit);
	set_bit(BTN_RIGHT, vmouse.idev->keybit);

	vmouse.idev->name = "Virtual Mouse";
	vmouse.idev->id.bustype = BUS_VIRTUAL;
	vmouse.idev->id.vendor  = 0x0000;
	vmouse.idev->id.product = 0x0000;
	vmouse.idev->id.version = 0x0000;

	ret = input_register_device(vmouse.idev);
	if(ret)
		goto err;

	return 0;

err:
	return -1;
}
/*----------------------------------------------------------------------------*/
void cleanup_module(void)
{
    if (!IS_ERR(vmouse.cl)) {
	    device_destroy(vmouse.cl, vmouse.dev);
	    class_destroy(vmouse.cl);
	    cdev_del(&vmouse.my_cdev);
    }
    unregister_chrdev_region(vmouse.dev, 1);
    input_unregister_device(vmouse.idev);

	dev_info(&vmouse.pdev, "Exit module\n");
}
/*----------------------------------------------------------------------------*/

MODULE_LICENSE("GPL"); /* Лицензия */
MODULE_AUTHOR("Dmitry Domnin"); /* Автор */
MODULE_DESCRIPTION("Driver for /dev/vmouse"); /* Описание */
MODULE_SUPPORTED_DEVICE(MYDEV_NAME); /* Устройство */
