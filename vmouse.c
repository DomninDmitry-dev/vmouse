#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
int init_module(void)
{
	int ret, i;

	pr_info("Hello, loading module\n");

	/*// Задаем статически младший и старший номер устройства
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
	pr_info("Exit module\n");
}
/*----------------------------------------------------------------------------*/

MODULE_LICENSE("GPL"); /* Лицензия */
MODULE_AUTHOR("Dmitry Domnin"); /* Автор */
MODULE_DESCRIPTION("Driver for /dev/mydev"); /* Описание */
MODULE_SUPPORTED_DEVICE(MYDEV_NAME); /* Устройство */
