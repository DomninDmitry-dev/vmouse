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

#define MYDEV_NAME	"vmouse"

struct vmouse {
	int my_major, my_minor;
	struct input_dev *idev;
};

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int init_module(void)
{
	int ret, i;
	struct vmouse *pdata;

	pr_info("%s: Hello, loading module\n", MYDEV_NAME);

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		pr_err("%s: Error mem <devm_kzalloc>\n", MYDEV_NAME);
		return -ENOMEM;
	}

/*
	// Задаем статически младший и старший номер устройства
	if(my_major != 0) { // Статически из параметра модуля
		dev = MKDEV(my_major, my_minor);
		ret = register_chrdev_region(dev, COUNT_DEV, MYDEV_NAME);
	}
	else { // Динамически
		ret = alloc_chrdev_region(&dev, my_minor, COUNT_DEV, MYDEV_NAME);
		my_major = MAJOR(dev);  // не забыть зафиксировать!
	}
	if( ret < 0 ) {
	  pr_info("=== Can not register char device region\n" );
	  goto err;
	}*/

	return 0;
}
/*----------------------------------------------------------------------------*/
void cleanup_module(void)
{
	pr_info("%s: Exit module\n", MYDEV_NAME);
}
/*----------------------------------------------------------------------------*/

MODULE_LICENSE("GPL"); /* Лицензия */
MODULE_AUTHOR("Dmitry Domnin"); /* Автор */
MODULE_DESCRIPTION("Driver for /dev/vmouse"); /* Описание */
MODULE_SUPPORTED_DEVICE(MYDEV_NAME); /* Устройство */
