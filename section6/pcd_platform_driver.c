#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include "platform.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Ahmed");
MODULE_DESCRIPTION("Platform driver");
MODULE_INFO(item1,"This is free form info about the platform driver.");

#define MAX_DEVICES 10

/*Device private data structure*/
struct pcdev_private_data
{
	struct pcdev_platform_data plat_data;
	char * buffer;
	dev_t dev_num;
	struct cdev cdev;
};

/*Driver private data structure*/
struct pcdrv_private_data
{
	int total_devices;
	dev_t device_number_base;
	struct device * device_pcd;
	struct class * class_pcd;
};

loff_t pcd_lseek(struct file * filp, loff_t offset, int whence)
{

	return 0;
}

ssize_t pcd_write(struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
{
	
	return -ENOMEM;
}

ssize_t pcd_read(struct file * filp, char __user * buff, size_t count, loff_t * f_pos)
{
	return 0;
}

int pcd_open(struct inode * inode, struct file * filp)
{
	pr_info("Open requested\n");

	return 0;

}

/*When a platform device is removed from the system or the driver is removed.*/
int pcd_release(struct inode * inode, struct file * filp)
{
		return 0;
}
struct file_operations pcd_fops =
 {
	.open = pcd_open,
	.write = pcd_write,
	.release = pcd_release,
	.read = pcd_read,
	.owner = THIS_MODULE
};

struct platform_device_id pcdev_ids[4] =
{	
	[0]
	{
		.name = "pcdev-A1x",
	},
	[1]
	{
		.name = "pcdev-B1x",
	},
	[2]
	{
		.name = "pcdev-C1x",
	},
	[3]
	{
		.name = "pcdev-D1x",
	}
};

/*Global structure which members will be filled in the init function*/
struct pcdrv_private_data  pcdrv_data;

int pcd_platform_driver_probe(struct platform_device * plat_dev)
{
	int ret;
	struct pcdev_private_data * dev_data;
	struct pcdev_platform_data * platform_data;

	pr_info("Device is detected.\n");
	/*1. Get platform data*/
	platform_data = (struct pcdev_platform_data *) dev_get_platdata(&plat_dev->dev);

	if (IS_ERR(platform_data))
	{
		pr_err("Error getting pcdev platform data.\n");
		ret = PTR_ERR(platform_data);
		return ret;
	}

	/*2. Dynamically allocate memory for device private data*/
	dev_data = devm_kzalloc(&plat_dev->dev,sizeof(*dev_data),GFP_KERNEL);
	if (!dev_data)
	{
		pr_err("Cannot allocate memory.\n");
		ret = -ENOMEM;
		goto dev_data_free;
	}

	/*Save the device private data pointer in the platform device structure*/
	dev_set_drvdata(&plat_dev->dev,dev_data);

	dev_data->plat_data.size = platform_data->size;
	dev_data->plat_data.perm = platform_data->perm;
	dev_data->plat_data.serial_number = platform_data->serial_number;

	/*Just for debugging*/
	pr_info("Device serial number: %s\n",dev_data->plat_data.serial_number);
	pr_info("Device size: %d\n",dev_data->plat_data.size);
	pr_info("Device permissions: %d",dev_data->plat_data.perm);
	
	/*3. Dynamically allocate memory for device buffer using size information 
	from the platform data*/
	dev_data->buffer = devm_kzalloc(&plat_dev->dev,dev_data->plat_data.size,GFP_KERNEL);	
	if (!dev_data->buffer)
	{
		pr_info("Can't allocate memory.\n");
		ret = -ENOMEM;
		goto out;		
	}

	/*4. Get the device number*/
	dev_data->dev_num = pcdrv_data.device_number_base + plat_dev->id;

	/*5. Do cdve_init and cdev_add*/
	cdev_init(&dev_data->cdev,&pcd_fops);
	dev_data->cdev.owner = THIS_MODULE;
	cdev_add(&dev_data->cdev,dev_data->dev_num,1);
	if (ret < 0)
	{
		pr_err("cdev added failed.\n");
		goto buffer_del;
	}

	/*6. Create device file for the detached platform device*/
	pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd,NULL,dev_data->dev_num,NULL,"pcdev-%d",plat_dev->id);
	if (IS_ERR(pcdrv_data.device_pcd))
	{
		pr_info("Device creation failed.\n");
		ret = PTR_ERR(pcdrv_data.device_pcd);
		goto cdev_del;
	}

	pcdrv_data.total_devices++;

	/*7. Error handling*/
	pr_info("Probe is successful.\n");
	return 0;
cdev_del:
	cdev_del(&dev_data->cdev);	
buffer_del:
	devm_kfree(&plat_dev->dev,dev_data->buffer);
dev_data_free:
	devm_kfree(&plat_dev->dev,dev_data);
out:
	pr_info("Device probe failed.\n");
	return ret;
}

/*Called when the device is removed from the system*/
int pcd_platform_driver_remove(struct platform_device * plat_dev)
{
	struct pcdev_private_data * dev_data = (struct pcdev_private_data *) dev_get_drvdata(&plat_dev->dev);
	/*1. Remove a device that was created with device create*/
	device_destroy(pcdrv_data.class_pcd,dev_data->dev_num);
		
	/*2. Remove cdev entry from system*/
	cdev_del(&dev_data->cdev);

	/*3. Free memory held by device*/
	kfree(dev_data->buffer);
	kfree(dev_data);
	
	pcdrv_data.total_devices--;
	pr_info("A device is removed.\n");
	return 0;
}


struct platform_driver pcd_platform_driver = 
{

	.id_table = pcdev_ids,
	.probe = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.driver = 
	{
		.name = "pseudo-char-device",
	},
};

static int __init pcd_platform_driver_init(void)
{
	int ret;
	/*1. Dynamically allocate a device number of MAX_DEVICES*/
		ret = alloc_chrdev_region(&pcdrv_data.device_number_base,0,MAX_DEVICES,"pcdevs");
		if (ret < 0)
		{
			pr_err("Alloc chrdev failed.\n");
			return ret;
		}
	/*2. Create device class under /sys/class*/
	pcdrv_data.class_pcd = class_create(THIS_MODULE,"pcd-class");
	if (IS_ERR(pcdrv_data.class_pcd))
	{
		pr_err("Class creation failed.\n");
		ret = PTR_ERR(pcdrv_data.class_pcd);
		unregister_chrdev_region(pcdrv_data.device_number_base,MAX_DEVICES);
		return ret;
	}

	/*3. Register platform driver*/
	platform_driver_register(&pcd_platform_driver);

	pr_info("PCD Platform driver loaded.\n");
	return 0;
}

void __exit pcd_platform_driver_exit(void)
{
	/*Unregister the platform driver*/
	platform_driver_unregister(&pcd_platform_driver);

	/*Class destroy*/
	class_destroy(pcdrv_data.class_pcd);
	
	/*Unregister device numbers for MAX_DEVICES*/
	unregister_chrdev_region(pcdrv_data.device_number_base,MAX_DEVICES);

	pr_info("Platform driver unloaded.\n");
}


module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_exit);
