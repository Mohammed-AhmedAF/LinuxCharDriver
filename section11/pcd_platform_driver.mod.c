#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x92997ed8, "_printk" },
	{ 0x48931e20, "device_destroy" },
	{ 0x5708ebf2, "cdev_del" },
	{ 0x37a0cba, "kfree" },
	{ 0x5f540977, "kmalloc_caches" },
	{ 0xfa55b3ee, "kmem_cache_alloc_trace" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xfd205a2d, "cdev_init" },
	{ 0x3fd2f8f8, "cdev_add" },
	{ 0xf2d7865d, "device_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xaee657ee, "__class_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xe9925143, "__platform_driver_register" },
	{ 0x66b467be, "platform_driver_unregister" },
	{ 0x645620c0, "class_destroy" },
	{ 0x541a6db8, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "793C05CD3C5D767F465E56F");
