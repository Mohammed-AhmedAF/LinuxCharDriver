#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"

#define RDONLY 0x01
#define WRONLY 0x10
#define RDWR 0x11

void pcdev_release(struct device * dev);

/*Create 2 platform data*/
struct pcdev_platform_data pcdev_pdata[4] =
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
	},
	[2] =
	{
		.size = 512,
		.perm = RDWR,
		.serial_number = "PCDEVABC3333"
	},
	[3] =
	{
		.size = 512,
		.perm = RDWR,
		.serial_number = "PCDEVABC3333"
	}
	
};

/*Create 2 platform devices*/
struct platform_device platform_pcdev_1 = {
	.name = "pcdev-A1x",
	.id = 0, /*ID field can be used for indexing.*/
	.dev = 
	{
		.platform_data = &pcdev_pdata[0],
		.release = pcdev_release
	}
};


struct platform_device platform_pcdev_2 = {
	.name = "pcdev-B1x",
	.id = 1,
	.dev = 
	{
		.platform_data = &pcdev_pdata[1],
		.release = pcdev_release
	}

};

struct platform_device platform_pcdev_3 = {
	.name = "pcdev-C1x",
	.id = 3,
	.dev = 
	{
		.platform_data = &pcdev_pdata[2],
		.release = pcdev_release
	}

};

struct platform_device platform_pcdev_4 = {
	.name = "pcdev-D1x",
	.id = 4,
	.dev = 
	{
		.platform_data = &pcdev_pdata[4],
		.release = pcdev_release
	}

};

struct platform_device * platform_devs_arr[4] =
{
	&platform_pcdev_1,
	&platform_pcdev_2,
	&platform_pcdev_3,
	&platform_pcdev_4
};

void pcdev_release(struct device * dev)
{
	pr_info("Device released.\n");
}

static int __init pcdev_platform_init(void)
{
	/*Register platform devices*/
	//platform_device_register(&platform_pcdev_1);
	//platform_device_register(&platform_pcdev_2);
	/*Add all devices in one function*/
	platform_add_devices(platform_devs_arr,ARRAY_SIZE(platform_devs_arr));

	pr_info("Module insterted.\n");

	return 0;
}

static void __exit pcdev_platform_exit(void)
{
	/*Unregister platform devices*/
	platform_device_unregister(&platform_pcdev_1);
	platform_device_unregister(&platform_pcdev_2);	
	platform_device_unregister(&platform_pcdev_3);
	platform_device_unregister(&platform_pcdev_4);
	pr_info("Module exit.\n");
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
