#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"

#define RDONLY 0x01
#define WRONLY 0x10
#define RDWR 0x11

/*Create 2 platform data*/
struct pcdev_platform_data pcdev_pdata[2] =
{
	[0] =	
	{
		.size = 512,
		.perm = RDWR,
		.serial_number = "PCDEVABC1111"
	},
	
	[1] =
	{
		.size = 1024,
		.perm = RDWR,
		.serial_number = "PCDEVABC2222"
	}
	
};

void pcdev_release(struct device * dev)
{
	pr_info("Device released.\n");
}

/*Create 2 platform devices*/
struct platform_device platform_pcdev_1 = {
	.name = "pseudo-char-device",
	.id = 0, /*ID field can be used for indexing.*/
	.dev = 
	{
		.platform_data = &pcdev_pdata[0],
		.release = pcdev_release
	}
};


struct platform_device platform_pcdev_2 = {
	.name = "pseudo-char-device",
	.id = 1,
	.dev = 
	{
		.platform_data = &pcdev_pdata[1],
		.release = pcdev_release
	}

};

static int __init pcdev_platform_init(void)
{
	/*Register platform devices*/
	platform_device_register(&platform_pcdev_1);
	platform_device_register(&platform_pcdev_2);

	pr_info("Module insterted.\n");

	return 0;
}

static void __exit pcdev_platform_exit(void)
{
	/*Unregister platform devices*/
	platform_device_unregister(&platform_pcdev_1);
	platform_device_unregister(&platform_pcdev_2);	

	pr_info("Module exit.\n");
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
