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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Ahmed");
MODULE_DESCRIPTION("Platform driver");

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

int pcd_release(struct inode * inode, struct file * filp)
{

	return 0;
}

int pcd_platform_driver_probe(struct platform_device * pdev)
{
	pr_info("A device is detected.\n");

	return 0;
}

/*Called when the device is removed from the system*/
int pcd_platform_driver_remove(struct platform_device * pdev)
{
	pr_info("A device is removed.\n");
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

struct platform_driver pcd_platform_driver = 
{
	.probe = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.driver = 
	{
		.name = "pseudo-char-device",
	},
};

static int __init pcd_platform_driver_init(void)
{
	platform_driver_register(&pcd_platform_driver);

	pr_info("Platform driver loaded.\n");
	return 0;
}

void __exit pcd_platform_driver_exit(void)
{
	platform_driver_unregister(&pcd_platform_driver);

	pr_info("Platform driver unloaded.\n");
}


module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_exit);
