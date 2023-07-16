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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Ahmed");
MODULE_DESCRIPTION("Pseudo character driver that handles n devices");

#define NO_OF_DEVICES 4

#define MEM_SIZE_MAX_PCDEV1 1024
#define MEM_SIZE_MAX_PCDEV2 512
#define MEM_SIZE_MAX_PCDEV3 1024
#define MEM_SIZE_MAX_PCDEV4 512


/*pseudo device's memory*/
char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];


/*Device private data*/
struct pcdev_private_data
{
	char * buffer;
 	unsigned size;
	const char * serial_number;
	int perm;
	struct cdev cdev;
};

/*Driver private data structure*/
/*class and device members will be assigned by kernel API*/
struct pcdrv_private_data
{
	int total_devices;
	struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
	dev_t device_number;
	struct class *  class_pcd;
	struct device * device_pcd;

};

struct pcdrv_private_data pcdrv_data = 
{
		.pcdev_data = 
	{
		[0] = 
		{
			.buffer = device_buffer_pcdev1,
			.size = MEM_SIZE_MAX_PCDEV1,
			.serial_number = "PCDEV1XYZ123",
			.perm = 0x1 /*Read-only*/
			
		},
		[1] =
		{
			.buffer = device_buffer_pcdev2,
			.size = MEM_SIZE_MAX_PCDEV2,
			.serial_number = "PCDEV2XYZ123",
			.perm = 0x11 /*Read-write*/
			
		},
		[2] =
		{
			.buffer = device_buffer_pcdev3,
			.size = MEM_SIZE_MAX_PCDEV3,
			.serial_number = "PCDEV3XYZ123",
			.perm = 0x01 /*Ready-only*/

		},
		[3] =
		{
			.buffer = device_buffer_pcdev4,
			.size = MEM_SIZE_MAX_PCDEV4,
			.serial_number = "PCDEV3XYZ123",
			.perm = 0x01
		
		}
	},
	.total_devices = NO_OF_DEVICES,

};


ssize_t pcd_read(struct file * filp, char __user * buff, size_t count, loff_t * f_pos)
{
    #if 0
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
	#endif
	return 0;	
}

ssize_t pcd_write(struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
{
	#if 0
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
	#endif
	return 0;
}

loff_t pcd_lseek(struct file * filp, loff_t offset, int whence)
{
	#if 0
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
	#endif
	return 0;
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

struct file_operations pcd_fops =
 {
	.open = pcd_open,
	.write = pcd_write,
	.release = pcd_release,
	.read = pcd_read,
	.owner = THIS_MODULE

};

static int __init hellodriver_init(void)
{
	int ret;
	int dev_index;
    /*Dynamically allocate a device number*/
    ret = alloc_chrdev_region(&pcdrv_data.device_number,0,NO_OF_DEVICES,"pcd_devices");
	if (ret < 0)
	{
		pr_err("Alloc chrdev failed.\n");
		goto out;
	}

	/*Create device class under /sys/class*/
			pcdrv_data.class_pcd = class_create(THIS_MODULE,"pcd_class");
			if (IS_ERR(pcdrv_data.class_pcd))
			{
				pr_err("Class creation failed.\n");
				/*Converting pointer to error code*/
				ret = PTR_ERR(pcdrv_data.class_pcd);
				goto unreg_chrdev;
			}



	for (dev_index = 0; dev_index < NO_OF_DEVICES; dev_index++)
	{
    	pr_info("%s : Device number <major> <minor> = %d:%d",__func__,MAJOR(pcdrv_data.device_number+dev_index),MINOR(pcdrv_data.device_number+dev_index));
	
			/*Initialize cdev structure*/    
			cdev_init(&pcdrv_data.pcdev_data[dev_index].cdev,&pcd_fops);

			/*Registering device driver with VFS*/
			pcdrv_data.pcdev_data[dev_index].cdev.owner = THIS_MODULE;
			ret = cdev_add(&pcdrv_data.pcdev_data[dev_index].cdev,pcdrv_data.device_number,1);
			if (ret < 0)
			{ 
				pr_err("cdev add failed.\n");
				goto cdev_del;
			}

			printk("Hello from module.\n");

			/*Populate SYSFS with device information*/
			pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd,NULL,pcdrv_data.device_number,NULL,"pcdev-%d",dev_index+1);
			if (IS_ERR(pcdrv_data.device_pcd))
			{
				pr_err("Device creation failed.\n");
				ret = PTR_ERR(pcdrv_data.device_pcd);
				goto class_del;
			
			}	
	}
	    printk("Module init was successful.\n");

cdev_del:	
class_del:
	for (;dev_index>=0;dev_index--)
	{
		device_destroy(pcdrv_data.class_pcd,pcdrv_data.device_number+dev_index);
		cdev_del(&pcdrv_data.pcdev_data[dev_index].cdev);
	}
	class_destroy(pcdrv_data.class_pcd);

unreg_chrdev:
	unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEVICES);

out: 
	pr_info("Module insertion failed.\n");
	return 0;
}

static void __exit pcd_driver_cleanup(void)
{
	int dev_index;
    printk("Exiting module PCD.\r\n");
	
	for (dev_index = 0; dev_index < NO_OF_DEVICES; dev_index++)
	{
    	device_destroy(pcdrv_data.class_pcd,pcdrv_data.device_number+dev_index);
    	cdev_del(&pcdrv_data.pcdev_data[dev_index].cdev);
	}	
	class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEVICES);
	
}


module_init(hellodriver_init);
module_exit(pcd_driver_cleanup);
