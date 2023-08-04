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
#include <linux/kernel.h>
#include "pcd.h"

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Ahmed");
MODULE_DESCRIPTION("A test driver");

#define DEV_MEM_SIZE 512
char device_buffer[DEV_MEM_SIZE];
dev_t device_number;

struct device * device_pcd;
struct cdev pcd_cdev;
typedef enum
{
		SYSFS_INDIV_FILES,
		SYSFS_GROUP,
}sysfs_filecreate_method_t;

ssize_t pcd_read(struct file * filp, char __user * buff, size_t count, loff_t * f_pos)
{
	pr_info("Read requested for %zu bytes.\n",count);
	/*Adjust the count*/
	if ((*f_pos + count) > DEV_MEM_SIZE)
	{
		count = DEV_MEM_SIZE - *f_pos;
	}

	/*Copy to user*/
	/*Note: Global variables access should be serialized to prevent race conditions*/
	/*copy_to_user should return zero for success*/

	if(copy_to_user(buff,&device_buffer[*f_pos],count))
	{
		return -EFAULT;
	}

	/*Update the current file position*/
	*f_pos += count;

	pr_info("Number of bytes successfully read = %zu\n",count);
	pr_info("Updated file position: %lld",*f_pos);

	/*Return number of bytes which were successfully read*/
	return count;

}

ssize_t pcd_write(struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
{

	pr_info("Write requested for %zu bytes.\n",count);
	pr_info("Current file position is %lld\n",*f_pos);

	/*Adjust count*/
	if ((*f_pos + count) > DEV_MEM_SIZE)
	{
		count = DEV_MEM_SIZE - *f_pos;
	}

	if (!count)
	{
		return -ENOMEM;	
	}

	/*Copy from user*/
	if (copy_from_user(&device_buffer[*f_pos],buff,count))
	{
		return -EFAULT;
	}

	/*Update file position*/
	*f_pos += count;


	/*Return number of bytes successfully written*/
	return count;
}

loff_t pcd_lseek(struct file * filp, loff_t offset, int whence)
{
	loff_t temp;

	pr_info("lseek requested\n");
	switch(whence)
	{
		case SEEK_SET:
			if ((offset > DEV_MEM_SIZE) || (offset < 0))
			{
				return -EINVAL;
			}
			else 
			{
				filp->f_pos = offset;
			}
			break;
		case SEEK_CUR:
			temp =  filp->f_pos + offset;
			if ((temp > DEV_MEM_SIZE) || temp < 0)
			{
				return -EINVAL;
			}
			else
			{
				filp->f_pos = filp->f_pos + offset;       
			}
			break;
		case SEEK_END:
			temp = filp->f_pos + DEV_MEM_SIZE;
			if ((temp > DEV_MEM_SIZE) || temp < 0)
			{
				return -EINVAL;
			}
			filp->f_pos = DEV_MEM_SIZE + offset;
			break;
		default:
			return -EINVAL;
	}

	return filp->f_pos;
}

int pcd_open(struct inode * inode, struct file * filp)
{
	pr_info("Open requested\n");

	return 0;

}

int pcd_release(struct inode * inode, struct file * filp)
{

	return 0;
}

ssize_t max_size_show (struct device * dev, struct device_attribute * attr, char * buff)
{
	/*Get access to device's private data*/
	struct pcd_private_data * dev_data = (struct pcd_private_data *) dev_get_drvdata(dev);

	/*Store the value into the buffer*/
	return sprintf(buff,"%d\n",dev_data->max_size);
}

ssize_t serial_number_show (struct device * dev, struct device_attribute * attr, char * buff)
{
	struct pcd_private_data * dev_data = (struct pcd_private_data *) dev_get_drvdata(dev);
	/*Returns the number of bytes copied into the buffer or an error code*/
	return sprintf(buff,"%s\n",dev_data->serial_number);
}

ssize_t max_size_store (struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	long result;
	int ret;
	struct pcd_private_data * dev_data = (struct pcd_private_data*) dev_get_drvdata(dev);	

	/*pr_info for debugging purposes*/
	pr_info("In max_size store\n");

	ret = kstrtol(buf,10,&result);
	if (ret)
	{
		pr_info("Failed to store max_size\n");
		return ret;
	}
	else
	{
		pr_info("Stored new value for max_size successfully\n");
		/*Maxs size changed via store method*/
		dev_data->max_size = result;
		
		return count;
	}
}

ssize_t serial_number_store (struct device * dev, struct device_attribute * attr, const char * buf, size_t count)
{
	struct pcd_private_data * dev_data	= (struct pcd_private_data *) dev_get_drvdata(dev);

	pr_info("In serial_number store\n");
	dev_data->serial_number = buf;

	return 0;
}

/*Create 2 variables of struct device_attribute*/
static DEVICE_ATTR(max_size,S_IRUGO|S_IWUSR,max_size_show,max_size_store);
static DEVICE_ATTR(serial_number,S_IRUGO|S_IWUSR,serial_number_show,serial_number_store);

struct attribute * pcd_attr_list[3] = {&dev_attr_max_size.attr,&dev_attr_serial_number.attr,NULL};

const struct attribute_group pcd_attr_group = {
	.attrs = pcd_attr_list,

};


int pcd_sysfs_create_files(struct device * pcd_dev, sysfs_filecreate_method_t method)
{
	int ret;
	switch (method)
	{
		case SYSFS_INDIV_FILES:
			ret = sysfs_create_file(&pcd_dev->kobj,&dev_attr_max_size.attr);
			if (ret < 0)
			{
				pr_err("Error creating file.");
				return ret;
			}
			else
			{
				pr_info("max_size SysFS file created");
			}
			ret = sysfs_create_file(&pcd_dev->kobj,&dev_attr_serial_number.attr);
			if (ret < 0)
			{
				pr_err("Error creating file.");
				return ret;
			}
			else
			{
				pr_info("serial_bumber SysFS file created.");

			}
			return ret;
			break;
		case SYSFS_GROUP:
			ret =sysfs_create_group(&pcd_dev->kobj,&pcd_attr_group);
			if (ret == 0)
			{
				pr_info("SYSFS files created using create_group.\n");
			}
			break;
		default:
			ret =sysfs_create_group(&pcd_dev->kobj,&pcd_attr_group);
			break;

	}
	return ret;
}

struct file_operations pcd_fops =
{
	.open = pcd_open,
	.write = pcd_write,
	.release = pcd_release,
	.read = pcd_read,
	.owner = THIS_MODULE

};

struct class *  class_pcd;
struct pcd_private_data prdata1 =  {.max_size=512,.serial_number="XYZ121"};
struct pcd_private_data * readDevPrvData;

static int __init pcd_init(void)
{
	int ret;
	/*Dynamically allocate a device number*/
	ret = alloc_chrdev_region(&device_number,0,7,"pcd_device");
	if (ret < 0)
	{
		goto out;
	}

	pr_info("%s : Device number <major> <minor> = %d:%d",__func__,MAJOR(device_number),MINOR(device_number));

	/*Initialize cdev structure*/    
	cdev_init(&pcd_cdev,&pcd_fops);

	/*Registering device driver with VFS*/
	pcd_cdev.owner = THIS_MODULE;
	ret = cdev_add(&pcd_cdev,device_number,1);
	if (ret < 0)
	{
		goto unreg_chrdev;
	}

	printk("Hello from module.\r\n");

	/*Create device class under /sys/class*/
	class_pcd = class_create(THIS_MODULE,"pcd_class");
	if (IS_ERR(class_pcd))
	{
		pr_err("Class creation failed.\n");
		/*Converting pointer to error code*/
		ret = PTR_ERR(class_pcd);
		goto cdev_del;
	}

	/*Populate SYSFS with device information*/
	device_pcd = device_create(class_pcd,NULL,device_number,NULL,"pcd");
	if (IS_ERR(device_pcd))
	{
		pr_err("Device creation failed.\n");
		ret = PTR_ERR(device_pcd);
		goto class_del;

	}
	else
	{

		printk("Module init was successful.");

		/*Set private data*/
		dev_set_drvdata(device_pcd,&prdata1);

		ret = pcd_sysfs_create_files(device_pcd,SYSFS_GROUP);
		if (ret != 0)
		{
			pr_info("Failed to create device attributes");
		}

		/*Read device private data*/
		readDevPrvData = (struct pcd_private_data*) dev_get_drvdata(device_pcd);
		pr_info("Max size read from private data is %d\n",readDevPrvData->max_size);
		pr_info("Serial number read from private data is %s\n",readDevPrvData->serial_number);

		return 0;
	}
class_del:
	class_destroy(class_pcd);

cdev_del:
	cdev_del(&pcd_cdev);

unreg_chrdev:
	unregister_chrdev_region(device_number,1);

out:
	return 0;
}

static void __exit pcd_driver_cleanup(void)
{
	printk("Exiting module PCD.\r\n");

	device_destroy(class_pcd,device_number);
	class_destroy(class_pcd);

	unregister_chrdev_region(device_number,1);
}


module_init(pcd_init);
module_exit(pcd_driver_cleanup);
