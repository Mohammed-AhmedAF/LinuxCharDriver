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
#include <linux/spinlock.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Ahmed");
MODULE_DESCRIPTION("A test driver");


#define DEV_MEM_SIZE 512
char device_buffer[DEV_MEM_SIZE];
dev_t device_number;

struct cdev pcd_cdev;
/**/
static DEFINE_MUTEX(pcd_mutex);

ssize_t pcd_read(struct file * filp, char __user * buff, size_t count, loff_t * f_pos)
{
	mutex_lock(&pcd_mutex);
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

	mutex_unlock(&pcd_mutex);

    /*Return number of bytes which were successfully read*/
    return count;

}

ssize_t pcd_write(struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
{

	/*Lock mutex*/
	mutx_lock(&pcd_mutex);
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

	/*Release the mutex*/
	mutex_unlock(&pcd_mutex);

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

struct file_operations pcd_fops =
 {
	.open = pcd_open,
	.write = pcd_write,
	.release = pcd_release,
	.read = pcd_read,
	.owner = THIS_MODULE

};

struct class *  class_pcd;
struct device * device_pcd;

static int __init hellodriver_init(void)
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

    printk("Module init was successful.");

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


module_init(hellodriver_init);
module_exit(pcd_driver_cleanup);
